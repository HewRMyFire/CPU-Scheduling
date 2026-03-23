#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"

static void enqueue_mlfq(MLFQScheduler *config, int q_level, Process *p) {
    MLFQQueue *q = &config->queues[q_level];
    q->queue[q->rear] = p;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

static Process* dequeue_mlfq(MLFQScheduler *config, int q_level) {
    MLFQQueue *q = &config->queues[q_level];
    Process *p = q->queue[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return p;
}

static void dispatch_mlfq(SchedulerState *state) {
    if (state->current_running != NULL) return;
    
    MLFQScheduler *config = (MLFQScheduler*)state->algo_data;

    for (int i = 0; i < config->num_queues; i++) {
        if (config->queues[i].size > 0) {
            Process *p = dequeue_mlfq(config, i);
            state->current_running = p;
            if (p->start_time == -1) p->start_time = state->current_time;
            p->state = STATE_RUNNING;
            p->quantum_used = state->current_time;

            int q = config->queues[i].time_quantum;
            if (q != -1 && p->remaining_time > q) {
                push_event(&state->event_queue, state->current_time + q, EVENT_QUANTUM_EXPIRE, p);
            } else {
                push_event(&state->event_queue, state->current_time + p->remaining_time, EVENT_COMPLETION, p);
            }
            break;
        }
    }
}

static void mlfq_arrival(SchedulerState *state, Process *p) {
    MLFQScheduler *config = (MLFQScheduler*)state->algo_data;
    p->priority = 0;
    p->time_in_queue = 0;
    enqueue_mlfq(config, 0, p);
    dispatch_mlfq(state);
}

static void mlfq_priority_boost(SchedulerState *state) {
    MLFQScheduler *config = (MLFQScheduler*)state->algo_data;
    
    if (state->current_running) {
        cancel_process_events(&state->event_queue, state->current_running);
        int elapsed = state->current_time - state->current_running->quantum_used;
        state->current_running->remaining_time -= elapsed;
        state->current_running->priority = 0;
        state->current_running->time_in_queue = 0;
        enqueue_mlfq(config, 0, state->current_running);
        state->current_running = NULL;
    }
    for (int i = 1; i < config->num_queues; i++) {
        while (config->queues[i].size > 0) {
            Process *p = dequeue_mlfq(config, i);
            p->priority = 0; 
            p->time_in_queue = 0;
            enqueue_mlfq(config, 0, p);
        }
    }
    push_event(&state->event_queue, state->current_time + config->boost_period, EVENT_PRIORITY_BOOST, NULL);
    dispatch_mlfq(state);
}

static void mlfq_quantum_expire(SchedulerState *state, Process *p) {
    MLFQScheduler *config = (MLFQScheduler*)state->algo_data;
    
    int elapsed = state->current_time - p->quantum_used;
    p->remaining_time -= elapsed;
    p->time_in_queue += elapsed;
    state->current_running = NULL;

    int a = config->queues[p->priority].allotment;
    if (a != -1 && p->time_in_queue >= a) {
        p->priority = (p->priority + 1 < config->num_queues) ? p->priority + 1 : p->priority;
        p->time_in_queue = 0;
    }
    enqueue_mlfq(config, p->priority, p);
    dispatch_mlfq(state);
}

static void mlfq_completion(SchedulerState *state, Process *p) {
    p->finish_time = state->current_time;
    p->state = STATE_FINISHED;
    p->remaining_time = 0;
    state->current_running = NULL;
    dispatch_mlfq(state);
}

int schedule_mlfq(SchedulerState *state, MLFQScheduler *config) {
    state->current_time = 0; state->event_queue = NULL; state->current_running = NULL;
    state->algo_data = config;
    init_gantt_chart(&state->chart);

    for (int i = 0; i < config->num_queues; i++) {
        config->queues[i].queue = (Process**)malloc(state->num_processes * sizeof(Process*));
        config->queues[i].front = 0; config->queues[i].rear = 0; config->queues[i].size = 0; config->queues[i].capacity = state->num_processes;
    }

    for (int i = 0; i < state->num_processes; i++) {
        push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);
    }
    push_event(&state->event_queue, config->boost_period, EVENT_PRIORITY_BOOST, NULL);

    SchedulerOps ops = {
        .handle_arrival = mlfq_arrival,
        .handle_completion = mlfq_completion,
        .handle_quantum_expire = mlfq_quantum_expire,
        .handle_priority_boost = mlfq_priority_boost
    };
    
    simulate_scheduler(state, &ops);

    print_gantt_chart(&state->chart);
    free_gantt_chart(&state->chart);

    for (int i = 0; i < config->num_queues; i++) {
        free(config->queues[i].queue);
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

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

static void dispatch_mlfq(SchedulerState *state, MLFQScheduler *config) {
    if (state->current_running != NULL) return;
    
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

int schedule_mlfq(SchedulerState *state, MLFQScheduler *config) {
    state->current_time = 0; state->event_queue = NULL; state->current_running = NULL;
    init_gantt_chart(&state->chart);

    for (int i = 0; i < config->num_queues; i++) {
        config->queues[i].queue = (Process**)malloc(state->num_processes * sizeof(Process*));
        config->queues[i].front = 0; config->queues[i].rear = 0; config->queues[i].size = 0; config->queues[i].capacity = state->num_processes;
    }

    for (int i = 0; i < state->num_processes; i++) push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);
    push_event(&state->event_queue, config->boost_period, EVENT_PRIORITY_BOOST, NULL);

    while (state->event_queue != NULL) {
        Event *evt = pop_event(&state->event_queue);

        int all_done = 1;
        for (int i=0; i<state->num_processes; i++) if (state->processes[i].state != STATE_FINISHED) all_done = 0;
        if (all_done && evt->type == EVENT_PRIORITY_BOOST) { free(evt); continue; }

        if (state->current_time < evt->time) {
            add_gantt_segment(&state->chart, state->current_running ? state->current_running->pid : "IDLE", state->current_time, evt->time);
            state->current_time = evt->time;
        }

        if (evt->type == EVENT_ARRIVAL) {
            evt->process->priority = 0;
            evt->process->time_in_queue = 0;
            enqueue_mlfq(config, 0, evt->process);
            dispatch_mlfq(state, config);

        } else if (evt->type == EVENT_PRIORITY_BOOST) {
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
                    p->priority = 0; p->time_in_queue = 0;
                    enqueue_mlfq(config, 0, p);
                }
            }
            push_event(&state->event_queue, state->current_time + config->boost_period, EVENT_PRIORITY_BOOST, NULL);
            dispatch_mlfq(state, config);

        } else if (evt->type == EVENT_QUANTUM_EXPIRE) {
            int elapsed = state->current_time - evt->process->quantum_used;
            evt->process->remaining_time -= elapsed;
            evt->process->time_in_queue += elapsed;
            state->current_running = NULL;

            int a = config->queues[evt->process->priority].allotment;
            if (a != -1 && evt->process->time_in_queue >= a) {
                evt->process->priority = (evt->process->priority + 1 < config->num_queues) ? evt->process->priority + 1 : evt->process->priority;
                evt->process->time_in_queue = 0;
            }
            enqueue_mlfq(config, evt->process->priority, evt->process);
            dispatch_mlfq(state, config);

        } else if (evt->type == EVENT_COMPLETION) {
            evt->process->finish_time = state->current_time;
            evt->process->state = STATE_FINISHED;
            evt->process->remaining_time = 0;
            state->current_running = NULL;
            dispatch_mlfq(state, config);
        }
        free(evt);
    }
    print_gantt_chart(&state->chart);
    free_gantt_chart(&state->chart);
    return 0;
}
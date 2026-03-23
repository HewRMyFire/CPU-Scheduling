#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"

static void dispatch_rr(SchedulerState *state) {
    int time_quantum = *(int*)state->algo_data;

    if (state->current_running == NULL && state->rq_size > 0) {
        Process *p = state->ready_queue[state->rq_front];
        state->rq_front = (state->rq_front + 1) % state->num_processes;
        state->rq_size--;

        state->current_running = p;
        if (p->start_time == -1) p->start_time = state->current_time;
        p->state = STATE_RUNNING;
        p->quantum_used = state->current_time;
        
        if (p->remaining_time > time_quantum) {
            push_event(&state->event_queue, state->current_time + time_quantum, EVENT_QUANTUM_EXPIRE, p);
        } else {
            push_event(&state->event_queue, state->current_time + p->remaining_time, EVENT_COMPLETION, p);
        }
    }
}

static void rr_arrival(SchedulerState *state, Process *p) {
    state->ready_queue[state->rq_rear] = p;
    state->rq_rear = (state->rq_rear + 1) % state->num_processes;
    state->rq_size++;
    dispatch_rr(state);
}

static void rr_quantum_expire(SchedulerState *state, Process *p) {
    int elapsed = state->current_time - p->quantum_used;
    p->remaining_time -= elapsed;
    state->current_running = NULL;
    
    state->ready_queue[state->rq_rear] = p;
    state->rq_rear = (state->rq_rear + 1) % state->num_processes;
    state->rq_size++;
    dispatch_rr(state);
}

static void rr_completion(SchedulerState *state, Process *p) {
    p->finish_time = state->current_time;
    p->state = STATE_FINISHED;
    p->remaining_time = 0;
    state->current_running = NULL;
    dispatch_rr(state);
}

int schedule_rr(SchedulerState *state, int time_quantum) {
    state->current_time = 0; state->event_queue = NULL; state->current_running = NULL;
    state->ready_queue = (Process **)malloc(state->num_processes * sizeof(Process *));
    state->rq_front = 0; state->rq_rear = 0; state->rq_size = 0;
    state->algo_data = &time_quantum;
    init_gantt_chart(&state->chart);

    for (int i = 0; i < state->num_processes; i++) {
        push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);
    }

    SchedulerOps ops = {
        .handle_arrival = rr_arrival,
        .handle_completion = rr_completion,
        .handle_quantum_expire = rr_quantum_expire,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    print_gantt_chart(&state->chart);
    free(state->ready_queue); free_gantt_chart(&state->chart);
    return 0;
}
#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"
#include "utils.h"

static void dispatch_rr(SchedulerState *state) {
    int time_quantum = *(int*)state->algo_data;

    if (state->current_running == NULL && state->rq_size > 0) {
        Process *p;
        dequeue_ready_queue(state, &p);

        set_process_running(p, state);
        init_process_start_time(p, state);
        p->quantum_used = state->current_time;
        
        if (p->remaining_time > time_quantum) {
            push_event(&state->event_queue, state->current_time + time_quantum, EVENT_QUANTUM_EXPIRE, p);
        } else {
            push_event(&state->event_queue, state->current_time + p->remaining_time, EVENT_COMPLETION, p);
        }
    }
}

static void rr_arrival(SchedulerState *state, Process *p) {
    enqueue_ready_queue_fifo(state, p);
    dispatch_rr(state);
}

static void rr_quantum_expire(SchedulerState *state, Process *p) {
    int elapsed = state->current_time - p->quantum_used;
    p->remaining_time -= elapsed;
    state->current_running = NULL;
    
    enqueue_ready_queue_fifo(state, p);
    dispatch_rr(state);
}

static void rr_completion(SchedulerState *state, Process *p) {
    process_completion_handler(state, p);
    dispatch_rr(state);
}

int schedule_rr(SchedulerState *state, int time_quantum) {
    state->current_time = 0;
    state->event_queue = NULL;
    state->current_running = NULL;
    state->ready_queue = (Process **)malloc(state->num_processes * sizeof(Process *));
    state->rq_front = 0;
    state->rq_rear = 0;
    state->rq_size = 0;
    state->algo_data = &time_quantum;
    init_gantt_chart(&state->chart);

    init_arrival_events(state);

    SchedulerOps ops = {
        .handle_arrival = rr_arrival,
        .handle_completion = rr_completion,
        .handle_quantum_expire = rr_quantum_expire,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    finalize_scheduler(state);
    return 0;
}
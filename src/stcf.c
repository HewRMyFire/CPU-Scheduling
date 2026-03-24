#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"
#include "utils.h"

static void enqueue_stcf(SchedulerState *state, Process *p) {
    int i;
    for (i = 0; i < state->rq_size; i++) {
        int idx = (state->rq_front + i) % state->num_processes;
        if (state->ready_queue[idx]->remaining_time > p->remaining_time) break;
    }
    for (int j = state->rq_size; j > i; j--) {
        state->ready_queue[(state->rq_front + j) % state->num_processes] = 
            state->ready_queue[(state->rq_front + j - 1) % state->num_processes];
    }
    state->ready_queue[(state->rq_front + i) % state->num_processes] = p;
    state->rq_rear = (state->rq_front + state->rq_size + 1) % state->num_processes;
    state->rq_size++;
}

static void dispatch_stcf(SchedulerState *state) {
    if (state->current_running == NULL && state->rq_size > 0) {
        Process *p;
        dequeue_ready_queue(state, &p);

        set_process_running(p, state);
        init_process_start_time(p, state);
        p->quantum_used = state->current_time;
        
        push_event(&state->event_queue, state->current_time + p->remaining_time, EVENT_COMPLETION, p);
    }
}

static void stcf_arrival(SchedulerState *state, Process *p) {
    if (state->current_running) {
        int elapsed = state->current_time - state->current_running->quantum_used;
        int current_rem = state->current_running->remaining_time - elapsed;

        if (p->remaining_time < current_rem) {
            cancel_process_events(&state->event_queue, state->current_running);
            state->current_running->remaining_time = current_rem;
            enqueue_stcf(state, state->current_running);
            state->current_running = NULL;
        }
    }
    enqueue_stcf(state, p);
    dispatch_stcf(state);
}

static void stcf_completion(SchedulerState *state, Process *p) {
    process_completion_handler(state, p);
    dispatch_stcf(state);
}

int schedule_stcf(SchedulerState *state) {
    init_ready_queue_scheduler(state);
    init_arrival_events(state);

    SchedulerOps ops = {
        .handle_arrival = stcf_arrival,
        .handle_completion = stcf_completion,
        .handle_quantum_expire = NULL,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    finalize_scheduler(state);
    return 0;
}
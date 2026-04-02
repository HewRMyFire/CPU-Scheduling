#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"
#include "utils.h"

// Comparison function for STCF: sort by remaining time
static int compare_remaining_time(const Process* a, const Process* b) {
    if (a->remaining_time < b->remaining_time) return -1;
    if (a->remaining_time > b->remaining_time) return 1;
    return 0;
}

static void dispatch_stcf(SchedulerState *state) {
    if (state->current_running == NULL && queue_size(state) > 0) {
        Process *p = queue_dequeue(state);

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
            queue_enqueue_sorted(state, state->current_running, compare_remaining_time);
            state->current_running = NULL;
        }
    }
    queue_enqueue_sorted(state, p, compare_remaining_time);
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
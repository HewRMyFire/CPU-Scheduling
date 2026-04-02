#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"
#include "utils.h"

// Comparison function for SJF: sort by burst time
static int compare_burst_time(const Process* a, const Process* b) {
    if (a->burst_time < b->burst_time) return -1;
    if (a->burst_time > b->burst_time) return 1;
    return 0;
}

static void dispatch_sjf(SchedulerState *state) {
    if (state->current_running == NULL && queue_size(state) > 0) {
        Process *p = queue_dequeue(state);
        
        set_process_running(p, state);
        init_process_start_time(p, state);
        
        push_event(&state->event_queue, state->current_time + p->burst_time, EVENT_COMPLETION, p);
    }
}

static void sjf_arrival(SchedulerState *state, Process *p) {
    queue_enqueue_sorted(state, p, compare_burst_time);
    dispatch_sjf(state);
}

static void sjf_completion(SchedulerState *state, Process *p) {
    process_completion_handler(state, p);
    dispatch_sjf(state);
}

int schedule_sjf(SchedulerState *state) {
    init_ready_queue_scheduler(state);
    init_arrival_events(state);

    SchedulerOps ops = {
        .handle_arrival = sjf_arrival,
        .handle_completion = sjf_completion,
        .handle_quantum_expire = NULL,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    finalize_scheduler(state);
    return 0;
}
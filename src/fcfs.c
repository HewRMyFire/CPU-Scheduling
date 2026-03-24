#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"
#include "utils.h"

static void dispatch_fcfs(SchedulerState *state) {
    if (state->current_running == NULL && state->rq_size > 0) {
        Process *p;
        dequeue_ready_queue(state, &p);

        set_process_running(p, state);
        init_process_start_time(p, state);
        
        push_event(&state->event_queue, state->current_time + p->burst_time, EVENT_COMPLETION, p);
    }
}

static void fcfs_arrival(SchedulerState *state, Process *p) {
    enqueue_ready_queue_fifo(state, p);
    dispatch_fcfs(state);
}

static void fcfs_completion(SchedulerState *state, Process *p) {
    process_completion_handler(state, p);
    dispatch_fcfs(state);
}

int schedule_fcfs(SchedulerState *state) {
    init_ready_queue_scheduler(state);
    init_arrival_events(state);

    SchedulerOps ops = {
        .handle_arrival = fcfs_arrival,
        .handle_completion = fcfs_completion,
        .handle_quantum_expire = NULL,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    finalize_scheduler(state);
    return 0;
}
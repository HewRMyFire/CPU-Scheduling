#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"

static void dispatch_fcfs(SchedulerState *state) {
    if (state->current_running == NULL && state->rq_size > 0) {
        Process *p = state->ready_queue[state->rq_front];
        state->rq_front = (state->rq_front + 1) % state->num_processes;
        state->rq_size--;

        state->current_running = p;
        if (p->start_time == -1) p->start_time = state->current_time;
        p->state = STATE_RUNNING;
        
        push_event(&state->event_queue, state->current_time + p->burst_time, EVENT_COMPLETION, p);
    }
}

static void fcfs_arrival(SchedulerState *state, Process *p) {
    state->ready_queue[state->rq_rear] = p;
    state->rq_rear = (state->rq_rear + 1) % state->num_processes;
    state->rq_size++;
    dispatch_fcfs(state);
}

static void fcfs_completion(SchedulerState *state, Process *p) {
    p->finish_time = state->current_time;
    p->state = STATE_FINISHED;
    p->remaining_time = 0;
    state->current_running = NULL;
    dispatch_fcfs(state);
}

int schedule_fcfs(SchedulerState *state) {
    state->current_time = 0;
    state->event_queue = NULL;
    state->current_running = NULL;
    state->ready_queue = (Process **)malloc(state->num_processes * sizeof(Process *));
    state->rq_front = 0; state->rq_rear = 0; state->rq_size = 0;
    init_gantt_chart(&state->chart);

    for (int i = 0; i < state->num_processes; i++) {
        push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);
    }

    SchedulerOps ops = {
        .handle_arrival = fcfs_arrival,
        .handle_completion = fcfs_completion,
        .handle_quantum_expire = NULL,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    print_gantt_chart(&state->chart);
    free(state->ready_queue);
    free_gantt_chart(&state->chart);
    return 0;
}
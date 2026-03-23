#include <stdlib.h>
#include "scheduler.h"
#include "engine.h"

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
        Process *p = state->ready_queue[state->rq_front];
        state->rq_front = (state->rq_front + 1) % state->num_processes;
        state->rq_size--;

        state->current_running = p;
        if (p->start_time == -1) p->start_time = state->current_time;
        p->state = STATE_RUNNING;
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
    p->finish_time = state->current_time;
    p->state = STATE_FINISHED;
    p->remaining_time = 0;
    state->current_running = NULL;
    dispatch_stcf(state);
}

int schedule_stcf(SchedulerState *state) {
    state->current_time = 0; state->event_queue = NULL; state->current_running = NULL;
    state->ready_queue = (Process **)malloc(state->num_processes * sizeof(Process *));
    state->rq_front = 0; state->rq_rear = 0; state->rq_size = 0;
    init_gantt_chart(&state->chart);

    for (int i = 0; i < state->num_processes; i++) {
        push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);
    }

    SchedulerOps ops = {
        .handle_arrival = stcf_arrival,
        .handle_completion = stcf_completion,
        .handle_quantum_expire = NULL,
        .handle_priority_boost = NULL
    };
    
    simulate_scheduler(state, &ops);

    print_gantt_chart(&state->chart);
    free(state->ready_queue); free_gantt_chart(&state->chart);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "utils.h"

void init_des_state(DES_State *state, Process *procs, int num) {
    state->processes = procs;
    state->num_processes = num;
    state->current_time = 0;
    init_gantt_chart(&state->chart);
    state->event_queue = NULL;
    state->current_running = NULL;

    state->ready_queue = (Process **)malloc(num * sizeof(Process *));
    state->rq_front = 0; 
    state->rq_rear = 0; 
    state->rq_size = 0;
}

void free_des_state(DES_State *state) {
    if (state->ready_queue) free(state->ready_queue);

    while (state->event_queue != NULL) {
        Event *e = pop_event(&state->event_queue);
        free(e);
    }
}

static void dispatch_next_fcfs(DES_State *state) {
    if (state->current_running == NULL && state->rq_size > 0) {
        Process *p = state->ready_queue[state->rq_front];
        state->rq_front = (state->rq_front + 1) % state->num_processes;
        state->rq_size--;

        state->current_running = p;
        if (p->start_time == -1) p->start_time = state->current_time;
        p->state = STATE_RUNNING;

        push_event(&state->event_queue, state->current_time + p->burst_time, 
                   EVENT_COMPLETION, p);
    }
}

static void handle_arrival_fcfs(DES_State *state, Process *p) {
    state->ready_queue[state->rq_rear] = p;
    state->rq_rear = (state->rq_rear + 1) % state->num_processes;
    state->rq_size++;

    dispatch_next_fcfs(state);
}

static void handle_completion_fcfs(DES_State *state, Process *p) {
    p->finish_time = state->current_time;
    p->state = STATE_FINISHED;
    p->remaining_time = 0;

    state->current_running = NULL;

    dispatch_next_fcfs(state);
}

void simulate_scheduler_des(DES_State *state, SchedulingAlgorithm algorithm) {
    for (int i = 0; i < state->num_processes; i++) {
        push_event(&state->event_queue, state->processes[i].arrival_time, 
                   EVENT_ARRIVAL, &state->processes[i]);
    }

    while (state->event_queue != NULL) {
        Event *current = pop_event(&state->event_queue);

        if (state->current_time < current->time) {
            if (state->current_running == NULL) {
                add_gantt_segment(&state->chart, "IDLE", state->current_time, current->time);
            } else {
                add_gantt_segment(&state->chart, state->current_running->pid, state->current_time, current->time);
            }
            state->current_time = current->time;
        }

        switch (current->type) {
            case EVENT_ARRIVAL:
                if (algorithm == ALGO_FCFS) handle_arrival_fcfs(state, current->process);
                break;
                
            case EVENT_COMPLETION:
                if (algorithm == ALGO_FCFS) handle_completion_fcfs(state, current->process);
                break;
                
            case EVENT_QUANTUM_EXPIRE:
                break;
                
            case EVENT_PRIORITY_BOOST:
                break;
        }
        
        free(current);
    }
}
#include <stdlib.h>
#include "scheduler.h"

static void dispatch_rr(SchedulerState *state, int time_quantum) {
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

int schedule_rr(SchedulerState *state, int time_quantum) {
    state->current_time = 0; state->event_queue = NULL; state->current_running = NULL;
    state->ready_queue = (Process **)malloc(state->num_processes * sizeof(Process *));
    state->rq_front = 0; state->rq_rear = 0; state->rq_size = 0;
    init_gantt_chart(&state->chart);

    for (int i = 0; i < state->num_processes; i++) push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);

    while (state->event_queue != NULL) {
        Event *evt = pop_event(&state->event_queue);
        if (state->current_time < evt->time) {
            add_gantt_segment(&state->chart, state->current_running ? state->current_running->pid : "IDLE", state->current_time, evt->time);
            state->current_time = evt->time;
        }

        if (evt->type == EVENT_ARRIVAL) {
            state->ready_queue[state->rq_rear] = evt->process;
            state->rq_rear = (state->rq_rear + 1) % state->num_processes;
            state->rq_size++;
            dispatch_rr(state, time_quantum);
            
        } else if (evt->type == EVENT_QUANTUM_EXPIRE) {
            int elapsed = state->current_time - evt->process->quantum_used;
            evt->process->remaining_time -= elapsed;
            state->current_running = NULL;
            
            state->ready_queue[state->rq_rear] = evt->process;
            state->rq_rear = (state->rq_rear + 1) % state->num_processes;
            state->rq_size++;
            dispatch_rr(state, time_quantum);
            
        } else if (evt->type == EVENT_COMPLETION) {
            evt->process->finish_time = state->current_time;
            evt->process->state = STATE_FINISHED;
            evt->process->remaining_time = 0;
            state->current_running = NULL;
            dispatch_rr(state, time_quantum);
        }
        free(evt);
    }
    print_gantt_chart(&state->chart);
    free(state->ready_queue); free_gantt_chart(&state->chart);
    return 0;
}
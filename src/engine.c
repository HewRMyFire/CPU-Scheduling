#include <stdlib.h>
#include "engine.h"
#include "utils.h"

void simulate_scheduler(SchedulerState *state, SchedulerOps *ops) {
    while (state->event_queue != NULL) {
        Event *evt = pop_event(&state->event_queue);

        int all_done = 1;
        for (int i = 0; i < state->num_processes; i++) {
            if (state->processes[i].state != STATE_FINISHED) {
                all_done = 0; 
                break;
            }
        }
        if (all_done && evt->type == EVENT_PRIORITY_BOOST) { 
            free(evt); 
            continue; 
        }

        if (state->current_time < evt->time) {
            add_gantt_segment(&state->chart, state->current_running ? state->current_running->pid : "IDLE", state->current_time, evt->time);
            state->current_time = evt->time;
        }

        switch (evt->type) {
            case EVENT_ARRIVAL:
                if (ops->handle_arrival) ops->handle_arrival(state, evt->process);
                break;
            case EVENT_COMPLETION:
                if (ops->handle_completion) ops->handle_completion(state, evt->process);
                break;
            case EVENT_QUANTUM_EXPIRE:
                if (ops->handle_quantum_expire) ops->handle_quantum_expire(state, evt->process);
                break;
            case EVENT_PRIORITY_BOOST:
                if (ops->handle_priority_boost) ops->handle_priority_boost(state);
                break;
        }
        
        free(evt);
    }
}
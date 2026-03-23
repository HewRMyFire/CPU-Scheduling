#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "utils.h"

int schedule_sjf(SchedulerState *state) {
    if (!state || !state->processes || state->num_processes <= 0) return -1;

    init_gantt_chart(&state->chart);
    state->current_time = 0;
    int completed = 0;

    while (completed < state->num_processes) {
        int shortest = -1;
        for (int i = 0; i < state->num_processes; i++) {
            if (state->processes[i].arrival_time <= state->current_time && state->processes[i].state != STATE_FINISHED) {
                if (shortest == -1 || state->processes[i].burst_time < state->processes[shortest].burst_time) {
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            int next_arrival = -1;
            for (int i = 0; i < state->num_processes; i++) {
                if (state->processes[i].state != STATE_FINISHED) {
                    if (next_arrival == -1 || state->processes[i].arrival_time < next_arrival) {
                        next_arrival = state->processes[i].arrival_time;
                    }
                }
            }
            add_gantt_segment(&state->chart, "IDLE", state->current_time, next_arrival);
            state->current_time = next_arrival;
            continue;
        }

        Process *p = &state->processes[shortest];
        p->start_time = state->current_time;
        p->state = STATE_RUNNING;
        
        add_gantt_segment(&state->chart, p->pid, state->current_time, state->current_time + p->burst_time);

        state->current_time += p->burst_time;
        p->remaining_time = 0;
        p->finish_time = state->current_time;
        p->state = STATE_FINISHED;
        completed++;
    }

    print_gantt_chart(&state->chart);
    free_gantt_chart(&state->chart);
    return 0;
}
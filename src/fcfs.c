#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "utils.h"

int schedule_fcfs(SchedulerState *state) {
    if (!state || !state->processes || state->num_processes <= 0) return -1;

    qsort(state->processes, state->num_processes, sizeof(Process), compare_arrival_time);
    init_gantt_chart(&state->chart);
    state->current_time = 0;

    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];

        if (state->current_time < p->arrival_time) {
            add_gantt_segment(&state->chart, "IDLE", state->current_time, p->arrival_time);
            state->current_time = p->arrival_time;
        }

        p->start_time = state->current_time;
        p->state = STATE_RUNNING;

        add_gantt_segment(&state->chart, p->pid, state->current_time, state->current_time + p->burst_time);

        state->current_time += p->burst_time;
        p->remaining_time = 0;
        p->finish_time = state->current_time;
        p->state = STATE_FINISHED;
    }

    print_gantt_chart(&state->chart);
    free_gantt_chart(&state->chart);
    return 0;
}
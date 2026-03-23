#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "utils.h"

#define MAX_LOGS 1000

int schedule_stcf(SchedulerState *state) {
    if (!state || !state->processes || state->num_processes <= 0) return -1;

    qsort(state->processes, state->num_processes, sizeof(Process), compare_arrival_time);
    init_gantt_chart(&state->chart);
    state->current_time = 0;
    
    int completed = 0;
    int prev_shortest = -1;
    char preemption_logs[MAX_LOGS][128];
    int log_count = 0;

    while (completed < state->num_processes) {
        int shortest = -1;

        for (int i = 0; i < state->num_processes; i++) {
            if (state->processes[i].arrival_time <= state->current_time && state->processes[i].state != STATE_FINISHED) {
                if (shortest == -1 || state->processes[i].remaining_time < state->processes[shortest].remaining_time) {
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            add_gantt_segment(&state->chart, "IDLE", state->current_time, state->current_time + 1);
            state->current_time++;
            prev_shortest = -1;
            continue;
        }

        Process* p = &state->processes[shortest];

        if (prev_shortest != -1 && prev_shortest != shortest) {
            Process* prev_p = &state->processes[prev_shortest];
            if (prev_p->state != STATE_FINISHED && prev_p->remaining_time > 0) {
                if (log_count < MAX_LOGS) sprintf(preemption_logs[log_count++], "Process %s was preempted at t=%d (remaining: %d)", prev_p->pid, state->current_time, prev_p->remaining_time);
            }
        }

        if (p->start_time != -1 && prev_shortest != shortest && state->current_time > p->start_time) {
            if (log_count < MAX_LOGS) sprintf(preemption_logs[log_count++], "Process %s resumed at t=%d", p->pid, state->current_time);
        }

        if (p->start_time == -1) p->start_time = state->current_time;
        p->state = STATE_RUNNING;

        add_gantt_segment(&state->chart, p->pid, state->current_time, state->current_time + 1);
        
        p->remaining_time--;
        state->current_time++;

        if (p->remaining_time == 0) {
            p->finish_time = state->current_time;
            p->state = STATE_FINISHED;
            completed++;
        }
        prev_shortest = shortest;
    }

    print_gantt_chart(&state->chart);
    if (log_count > 0) {
        for (int i = 0; i < log_count; i++) printf("%s\n", preemption_logs[i]);
        printf("\n");
    }
    free_gantt_chart(&state->chart);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "utils.h"

int schedule_rr(SchedulerState *state, int time_quantum) {
    if (!state || !state->processes || state->num_processes <= 0) return -1;

    qsort(state->processes, state->num_processes, sizeof(Process), compare_arrival_time);
    printf("Using time quantum q=%d\n", time_quantum);

    init_gantt_chart(&state->chart);
    state->current_time = 0;
    
    int completed = 0;
    int context_switches = 0;
    char prev_pid[16] = "";

    int* in_queue = (int*)safe_malloc(state->num_processes * sizeof(int));
    for (int i = 0; i < state->num_processes; i++) in_queue[i] = 0;

    Process** queue = (Process**)safe_malloc(state->num_processes * sizeof(Process*));
    int front = 0, rear = 0, q_size = 0;

    if (state->processes[0].arrival_time > 0) {
        add_gantt_segment(&state->chart, "IDLE", 0, state->processes[0].arrival_time);
        state->current_time = state->processes[0].arrival_time;
        strcpy(prev_pid, "IDLE");
    }

    int idx = 0;
    for (; idx < state->num_processes && state->processes[idx].arrival_time <= state->current_time; idx++) {
        queue[rear] = &state->processes[idx];
        rear = (rear + 1) % state->num_processes;
        q_size++;
        in_queue[idx] = 1;
    }

    while (completed < state->num_processes) {
        if (q_size == 0) {
            if (idx < state->num_processes) {
                if (strcmp(prev_pid, "IDLE") != 0) {
                    strcpy(prev_pid, "IDLE");
                    context_switches++;
                }
                add_gantt_segment(&state->chart, "IDLE", state->current_time, state->processes[idx].arrival_time);
                state->current_time = state->processes[idx].arrival_time;

                for (; idx < state->num_processes && state->processes[idx].arrival_time <= state->current_time; idx++) {
                    queue[rear] = &state->processes[idx];
                    rear = (rear + 1) % state->num_processes;
                    q_size++;
                    in_queue[idx] = 1;
                }
            }
            continue;
        }

        Process* p = queue[front];
        front = (front + 1) % state->num_processes;
        q_size--;

        if (strcmp(prev_pid, p->pid) != 0) {
            if (prev_pid[0] != '\0') context_switches++;
            strcpy(prev_pid, p->pid);
        }

        if (p->start_time == -1) p->start_time = state->current_time;
        p->state = STATE_RUNNING;

        int exec_time = util_min(p->remaining_time, time_quantum);
        int start = state->current_time;

        state->current_time += exec_time;
        p->remaining_time -= exec_time;
        p->quantum_used += exec_time;

        add_gantt_segment(&state->chart, p->pid, start, state->current_time);

        for (; idx < state->num_processes && state->processes[idx].arrival_time <= state->current_time; idx++) {
            if (!in_queue[idx]) {
                queue[rear] = &state->processes[idx];
                rear = (rear + 1) % state->num_processes;
                q_size++;
                in_queue[idx] = 1;
            }
        }

        if (p->remaining_time > 0) {
            queue[rear] = p;
            rear = (rear + 1) % state->num_processes;
            q_size++;
        } else {
            p->finish_time = state->current_time;
            p->state = STATE_FINISHED;
            completed++;
        }
    }
    context_switches++;

    print_gantt_chart(&state->chart);
    printf("Total context switches: %d\n\n", context_switches);

    free(in_queue);
    free(queue);
    free_gantt_chart(&state->chart);
    return 0;
}
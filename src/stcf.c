#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"
#include "utils.h"

#define MAX_LOGS 1000

void simulate_stcf(Process* processes, int num_processes) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    int current_time = 0;
    int completed = 0;
    int prev_shortest = -1;

    GanttChart chart;
    init_gantt_chart(&chart);

    char preemption_logs[MAX_LOGS][128];
    int log_count = 0;

    while (completed < num_processes) {
        int shortest = -1;

        for (int i = 0; i < num_processes; i++) {
            if (processes[i].arrival_time <= current_time && processes[i].state != STATE_FINISHED) {
                if (shortest == -1 || processes[i].remaining_time < processes[shortest].remaining_time) {
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            add_gantt_segment(&chart, "IDLE", current_time, current_time + 1);
            current_time++;
            prev_shortest = -1;
            continue;
        }

        Process* p = &processes[shortest];

        if (prev_shortest != -1 && prev_shortest != shortest) {
            Process* prev_p = &processes[prev_shortest];
            if (prev_p->state != STATE_FINISHED && prev_p->remaining_time > 0) {
                if (log_count < MAX_LOGS) {
                    sprintf(preemption_logs[log_count++], "Process %s was preempted at t=%d (remaining: %d)",
                            prev_p->pid, current_time, prev_p->remaining_time);
                }
            }
        }

        if (p->start_time != -1 && prev_shortest != shortest && current_time > p->start_time) {
            if (log_count < MAX_LOGS) {
                sprintf(preemption_logs[log_count++], "Process %s resumed at t=%d", p->pid, current_time);
            }
        }

        if (p->start_time == -1) {
            p->start_time = current_time;
        }
        
        p->state = STATE_RUNNING;

        add_gantt_segment(&chart, p->pid, current_time, current_time + 1);
        
        p->remaining_time--;
        current_time++;

        if (p->remaining_time == 0) {
            p->completion_time = current_time;
            p->state = STATE_FINISHED;
            completed++;
        }
        
        prev_shortest = shortest;
    }

    print_gantt_chart(&chart);
    
    if (log_count > 0) {
        for (int i = 0; i < log_count; i++) {
            printf("%s\n", preemption_logs[i]);
        }
        printf("\n");
    }

    free_gantt_chart(&chart);
}
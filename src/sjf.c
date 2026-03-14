#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"

static int compare_arrival_time(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    return p1->arrival_time - p2->arrival_time;
}

void simulate_sjf(Process* processes, int num_processes) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    for (int i = 0; i < num_processes; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].state = STATE_READY;
    }

    int current_time = 0;
    int completed = 0;

    GanttChart chart;
    init_gantt_chart(&chart);

    while (completed < num_processes) {
        int shortest = -1;

        for (int i = 0; i < num_processes; i++) {
            if (processes[i].arrival_time <= current_time &&
                processes[i].state != STATE_FINISHED) {

                if (shortest == -1 ||
                    processes[i].burst_time < processes[shortest].burst_time) {
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            add_gantt_segment(&chart, "IDLE", current_time, current_time + 1);
            current_time++;
            continue;
        }

        Process* p = &processes[shortest];

        p->start_time = current_time;
        p->state = STATE_RUNNING;

        int start = current_time;

        current_time += p->burst_time;

        p->remaining_time = 0;
        p->completion_time = current_time;
        p->state = STATE_FINISHED;

        add_gantt_segment(&chart, p->pid, start, current_time);

        completed++;
    }

    printf("\nSJF Gantt Chart:\n");
    print_gantt_chart(&chart);
    free_gantt_chart(&chart);
}
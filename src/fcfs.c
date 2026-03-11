#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"

static int compare_arrival_time(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }
    return 0;
}

void simulate_fcfs(Process* processes, int num_processes) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    int current_time = 0;
    GanttChart chart;
    init_gantt_chart(&chart);

    for (int i = 0; i < num_processes; i++) {
        Process* p = &processes[i];

        if (current_time < p->arrival_time) {
            add_gantt_segment(&chart, "IDLE", current_time, p->arrival_time);
            current_time = p->arrival_time;
        }

        p->start_time = current_time;
        p->state = STATE_RUNNING;

        int start = current_time;
        current_time += p->burst_time;

        p->remaining_time = 0;
        p->completion_time = current_time;
        p->state = STATE_FINISHED;

        add_gantt_segment(&chart, p->pid, start, current_time);
    }

    printf("\nFCFS Gantt Chart:\n");
    print_gantt_chart(&chart);
    free_gantt_chart(&chart);
}
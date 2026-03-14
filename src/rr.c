#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"
#include "utils.h"

void simulate_rr(Process* processes, int num_processes, int time_quantum) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    int current_time = 0;
    int completed = 0;

    int* in_queue = (int*)safe_malloc(num_processes * sizeof(int));
    for (int i = 0; i < num_processes; i++) in_queue[i] = 0;

    Process** queue = (Process**)safe_malloc(num_processes * sizeof(Process*));
    int front = 0, rear = 0, q_size = 0;

    GanttChart chart;
    init_gantt_chart(&chart);

    if (processes[0].arrival_time > 0) {
        add_gantt_segment(&chart, "IDLE", 0, processes[0].arrival_time);
        current_time = processes[0].arrival_time;
    }

    int idx = 0;
    for (; idx < num_processes && processes[idx].arrival_time <= current_time; idx++) {
        queue[rear] = &processes[idx];
        rear = (rear + 1) % num_processes;
        q_size++;
        in_queue[idx] = 1;
    }

    while (completed < num_processes) {
        if (q_size == 0) {
            if (idx < num_processes) {
                add_gantt_segment(&chart, "IDLE", current_time, processes[idx].arrival_time);
                current_time = processes[idx].arrival_time;

                for (; idx < num_processes && processes[idx].arrival_time <= current_time; idx++) {
                    queue[rear] = &processes[idx];
                    rear = (rear + 1) % num_processes;
                    q_size++;
                    in_queue[idx] = 1;
                }
            }
            continue;
        }

        Process* p = queue[front];
        front = (front + 1) % num_processes;
        q_size--;

        if (p->start_time == -1) {
            p->start_time = current_time;
            p->state = STATE_RUNNING;
        }

        int exec_time = util_min(p->remaining_time, time_quantum);
        int start = current_time;

        current_time += exec_time;
        p->remaining_time -= exec_time;
        p->quantum_used++;

        add_gantt_segment(&chart, p->pid, start, current_time);

        for (; idx < num_processes && processes[idx].arrival_time <= current_time; idx++) {
            if (!in_queue[idx]) {
                queue[rear] = &processes[idx];
                rear = (rear + 1) % num_processes;
                q_size++;
                in_queue[idx] = 1;
            }
        }

        if (p->remaining_time > 0) {
            queue[rear] = p;
            rear = (rear + 1) % num_processes;
            q_size++;
        } else {
            p->completion_time = current_time;
            p->state = STATE_FINISHED;
            completed++;
        }
    }

    printf("\nRR Gantt Chart:\n");
    print_gantt_chart(&chart);

    free(in_queue);
    free(queue);
    free_gantt_chart(&chart);
}
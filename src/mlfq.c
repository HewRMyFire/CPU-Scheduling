#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"

typedef struct {
    Process** procs;
    int front;
    int rear;
    int size;
    int capacity;
} MLFQ_Queue;

static void init_queue(MLFQ_Queue* q, int cap) {
    q->procs = (Process**)malloc(cap * sizeof(Process*));
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = cap;
}

static void enqueue(MLFQ_Queue* q, Process* p) {
    q->procs[q->rear] = p;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

static Process* dequeue(MLFQ_Queue* q) {
    Process* p = q->procs[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return p;
}

static void free_queue(MLFQ_Queue* q) {
    free(q->procs);
}

static int compare_arrival_time(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }
    return 0;
}

void simulate_mlfq(Process* processes, int num_processes, MLFQ_Config* config) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    int current_time = 0;
    int completed = 0;
    int last_boost_time = 0;
    int idx = 0;

    MLFQ_Queue* queues = (MLFQ_Queue*)malloc(config->num_queues * sizeof(MLFQ_Queue));
    for (int i = 0; i < config->num_queues; i++) {
        init_queue(&queues[i], num_processes);
    }

    GanttChart chart;
    init_gantt_chart(&chart);

    if (processes[0].arrival_time > 0) {
        add_gantt_segment(&chart, "IDLE", 0, processes[0].arrival_time);
        current_time = processes[0].arrival_time;
    }

    while (completed < num_processes) {
        while (idx < num_processes && processes[idx].arrival_time <= current_time) {
            processes[idx].queue_level = 0;
            enqueue(&queues[0], &processes[idx]);
            idx++;
        }

        int selected_q = -1;
        Process* p = NULL;
        for (int i = 0; i < config->num_queues; i++) {
            if (queues[i].size > 0) {
                p = dequeue(&queues[i]);
                selected_q = i;
                break;
            }
        }

        if (p == NULL) {
            int next_event = (idx < num_processes) ? processes[idx].arrival_time : -1;
            int next_boost = last_boost_time + config->boost_interval;

            if (next_event == -1) break;

            if (next_boost < next_event && completed < num_processes) {
                add_gantt_segment(&chart, "IDLE", current_time, next_boost);
                current_time = next_boost;
                last_boost_time = current_time;
            } else {
                add_gantt_segment(&chart, "IDLE", current_time, next_event);
                current_time = next_event;
            }
            continue;
        }

        if (p->start_time == -1) p->start_time = current_time;
        p->state = STATE_RUNNING;

        int time_allotment = config->time_quantums[selected_q];
        int run_time = p->remaining_time;

        if (time_allotment < run_time) {
            run_time = time_allotment;
        }

        int time_to_next_arrival = (idx < num_processes) ? (processes[idx].arrival_time - current_time) : 999999;
        if (selected_q > 0 && time_to_next_arrival < run_time) {
            run_time = time_to_next_arrival;
        }

        int time_to_boost = (last_boost_time + config->boost_interval) - current_time;
        if (time_to_boost > 0 && time_to_boost < run_time) {
            run_time = time_to_boost;
        }

        int start = current_time;
        current_time += run_time;
        p->remaining_time -= run_time;
        p->quantum_used += run_time;

        add_gantt_segment(&chart, p->pid, start, current_time);

        if (current_time - last_boost_time >= config->boost_interval) {
            last_boost_time = current_time;

            while (idx < num_processes && processes[idx].arrival_time <= current_time) {
                processes[idx].queue_level = 0;
                enqueue(&queues[0], &processes[idx]);
                idx++;
            }

            if (p->remaining_time > 0) {
                p->queue_level = 0;
                enqueue(&queues[0], p);
                p = NULL;
            }

            for (int i = 1; i < config->num_queues; i++) {
                while (queues[i].size > 0) {
                    Process* lower_p = dequeue(&queues[i]);
                    lower_p->queue_level = 0;
                    enqueue(&queues[0], lower_p);
                }
            }
        } else {
            while (idx < num_processes && processes[idx].arrival_time <= current_time) {
                processes[idx].queue_level = 0;
                enqueue(&queues[0], &processes[idx]);
                idx++;
            }
        }

        if (p != NULL) {
            if (p->remaining_time == 0) {
                p->completion_time = current_time;
                p->state = STATE_FINISHED;
                completed++;
            } else if (run_time == time_allotment) {
                if (selected_q < config->num_queues - 1) {
                    p->queue_level = selected_q + 1;
                }
                enqueue(&queues[p->queue_level], p);
            } else {
                enqueue(&queues[selected_q], p);
            }
        }
    }

    printf("\nMLFQ Gantt Chart:\n");
    print_gantt_chart(&chart);

    for (int i = 0; i < config->num_queues; i++) {
        free_queue(&queues[i]);
    }
    free(queues);
    free_gantt_chart(&chart);
}
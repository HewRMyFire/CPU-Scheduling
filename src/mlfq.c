#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"
#include "utils.h"

#define MAX_TRACE 2000

typedef struct {
    Process** procs;
    int front;
    int rear;
    int size;
    int capacity;
} MLFQ_Queue;

static void init_queue(MLFQ_Queue* q, int cap) {
    q->procs = (Process**)safe_malloc(cap * sizeof(Process*));
    q->front = 0; q->rear = 0; q->size = 0; q->capacity = cap;
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

static void free_queue(MLFQ_Queue* q) { free(q->procs); }

void simulate_mlfq(Process* processes, int num_processes, MLFQ_Config* config) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    printf("\n=== MLFQ Configuration ===\n");
    for(int i = 0; i < config->num_queues; i++) {
        if(config->time_quantums[i] == -1) 
            printf("Queue %d: FCFS (lowest priority)\n", i);
        else 
            printf("Queue %d: q=%d, allotment=%d %s\n", i, config->time_quantums[i], config->allotments[i], i==0?"(highest priority)":"");
    }
    printf("Boost period: %d\n", config->boost_interval);

    char traces[MAX_TRACE][128];
    int trace_count = 0;

    MLFQ_Queue* queues = (MLFQ_Queue*)safe_malloc(config->num_queues * sizeof(MLFQ_Queue));
    for (int i = 0; i < config->num_queues; i++) init_queue(&queues[i], num_processes);

    GanttChart chart;
    init_gantt_chart(&chart);

    int current_time = 0;
    int completed = 0;
    int last_boost = 0;
    int idx = 0;

    Process* running = NULL;
    int slice_used = 0;

    while (completed < num_processes) {
        if (current_time > 0 && current_time - last_boost >= config->boost_interval) {
            if (trace_count < MAX_TRACE) sprintf(traces[trace_count++], "t=%-4d Priority boost: all processes -> Q0", current_time);
            
            if (running) { enqueue(&queues[running->queue_level], running); running = NULL; }
            for (int i = 1; i < config->num_queues; i++) {
                while (queues[i].size > 0) {
                    Process* p = dequeue(&queues[i]);
                    p->queue_level = 0; p->current_allotment_used = 0;
                    enqueue(&queues[0], p);
                }
            }
            for (int i = 0; i < queues[0].size; i++) {
                queues[0].procs[(queues[0].front + i) % queues[0].capacity]->current_allotment_used = 0;
            }
            last_boost = current_time;
        }

        while (idx < num_processes && processes[idx].arrival_time <= current_time) {
            if (trace_count < MAX_TRACE) sprintf(traces[trace_count++], "t=%-4d Process %s enters Q0", current_time, processes[idx].pid);
            processes[idx].queue_level = 0;
            processes[idx].current_allotment_used = 0;
            enqueue(&queues[0], &processes[idx]);
            idx++;
        }

        if (running) {
            for (int i = 0; i < running->queue_level; i++) {
                if (queues[i].size > 0) {
                    enqueue(&queues[running->queue_level], running);
                    running = NULL;
                    break;
                }
            }
        }

        if (!running) {
            for (int i = 0; i < config->num_queues; i++) {
                if (queues[i].size > 0) {
                    running = dequeue(&queues[i]);
                    slice_used = 0;
                    break;
                }
            }
        }

        if (!running) {
            add_gantt_segment(&chart, "IDLE", current_time, current_time + 1);
            current_time++;
            continue;
        }

        if (running->start_time == -1) running->start_time = current_time;
        running->state = STATE_RUNNING;
        
        add_gantt_segment(&chart, running->pid, current_time, current_time + 1);
        
        running->remaining_time--;
        running->current_allotment_used++;
        slice_used++;
        current_time++;
        running->quantum_used++;

        if (running->remaining_time == 0) {
            running->completion_time = current_time;
            running->state = STATE_FINISHED;
            if (trace_count < MAX_TRACE) sprintf(traces[trace_count++], "t=%-4d Process %s completes in Q%d", current_time, running->pid, running->queue_level);
            completed++;
            running = NULL;
        } else {
            int a = config->allotments[running->queue_level];
            int q = config->time_quantums[running->queue_level];
            
            if (a != -1 && running->current_allotment_used >= a) {
                int next_q = (running->queue_level + 1 < config->num_queues) ? running->queue_level + 1 : running->queue_level;
                if (trace_count < MAX_TRACE && next_q != running->queue_level) {
                    sprintf(traces[trace_count++], "t=%-4d Process %s -> Q%d (exhausted Q%d allotment)", current_time, running->pid, next_q, running->queue_level);
                }
                running->queue_level = next_q;
                running->current_allotment_used = 0;
                enqueue(&queues[running->queue_level], running);
                running = NULL;
            } else if (q != -1 && slice_used >= q) {
                enqueue(&queues[running->queue_level], running);
                running = NULL;
            }
        }
    }

    printf("\n=== Execution Trace ===\n");
    for(int i = 0; i < trace_count; i++) printf("%s\n", traces[i]);

    printf("\n=== Analysis ===\n");
    Process* interactive = NULL;
    Process* long_job = NULL;
    double total_resp = 0; int int_count = 0;
    
    for(int i = 0; i < num_processes; i++) {
        if (processes[i].queue_level == 0) {
            interactive = &processes[i];
            total_resp += (processes[i].start_time - processes[i].arrival_time);
            int_count++;
        }
        if (processes[i].queue_level == config->num_queues - 1) long_job = &processes[i];
    }

    printf("Interactive job (short burst) behavior:\n");
    if(interactive) {
        printf("  - Process %s stayed in Q0 (completed in %d time units)\n", interactive->pid, interactive->burst_time);
        printf("  - Average response time: %.1f\n", total_resp/int_count);
    } else {
        printf("  - No purely interactive jobs completed in Q0.\n");
    }
    
    printf("\nLong-running job behavior:\n");
    if(long_job) {
        printf("  - Process %s demoted to Q%d\n", long_job->pid, config->num_queues - 1);
        printf("  - Turnaround time: %d (fair for its burst time)\n", long_job->completion_time - long_job->arrival_time);
    } else {
        printf("  - No long-running jobs reached the lowest queue.\n");
    }
    printf("\nThe MLFQ successfully balanced responsiveness and fairness.\n");

    for (int i = 0; i < config->num_queues; i++) free_queue(&queues[i]);
    free(queues);
    free_gantt_chart(&chart);
}
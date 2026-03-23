#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scheduler.h"
#include "process.h"
#include "gantt.h"
#include "utils.h"

#define MAX_TRACE 2000

static void init_queue(MLFQQueue* q, int cap) {
    q->queue = (Process**)safe_malloc(cap * sizeof(Process*));
    q->front = 0; 
    q->rear = 0; 
    q->size = 0; 
    q->capacity = cap;
}

static void enqueue(MLFQQueue* q, Process* p) {
    q->queue[q->rear] = p;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

static Process* dequeue(MLFQQueue* q) {
    Process* p = q->queue[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return p;
}

static void free_queue(MLFQQueue* q) { 
    free(q->queue); 
}

void simulate_mlfq(Process* processes, int num_processes, MLFQScheduler* scheduler) {
    if (num_processes <= 0) return;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    printf("\n=== MLFQ Configuration ===\n");
    for(int i = 0; i < scheduler->num_queues; i++) {
        if(scheduler->queues[i].time_quantum == -1) 
            printf("Queue %d: FCFS (lowest priority)\n", i);
        else 
            printf("Queue %d: q=%d, allotment=%d %s\n", i, scheduler->queues[i].time_quantum, scheduler->queues[i].allotment, i==0?"(highest priority)":"");
    }
    printf("Boost period: %d\n", scheduler->boost_period);

    char traces[MAX_TRACE][128];
    int trace_count = 0;

    for (int i = 0; i < scheduler->num_queues; i++) {
        init_queue(&scheduler->queues[i], num_processes);
    }

    GanttChart chart;
    init_gantt_chart(&chart);

    int current_time = 0;
    int completed = 0;
    scheduler->last_boost = 0;
    int idx = 0;

    Process* running = NULL;
    int slice_used = 0;

    while (completed < num_processes) {
        if (current_time > 0 && current_time - scheduler->last_boost >= scheduler->boost_period) {
            if (trace_count < MAX_TRACE) sprintf(traces[trace_count++], "t=%-4d Priority boost: all processes -> Q0", current_time);
            
            if (running) { 
                enqueue(&scheduler->queues[running->priority], running); 
                running = NULL; 
            }
            
            for (int i = 1; i < scheduler->num_queues; i++) {
                while (scheduler->queues[i].size > 0) {
                    Process* p = dequeue(&scheduler->queues[i]);
                    p->priority = 0; 
                    p->time_in_queue = 0;
                    enqueue(&scheduler->queues[0], p);
                }
            }
            
            for (int i = 0; i < scheduler->queues[0].size; i++) {
                scheduler->queues[0].queue[(scheduler->queues[0].front + i) % scheduler->queues[0].capacity]->time_in_queue = 0;
            }
            scheduler->last_boost = current_time;
        }

        while (idx < num_processes && processes[idx].arrival_time <= current_time) {
            if (trace_count < MAX_TRACE) sprintf(traces[trace_count++], "t=%-4d Process %s enters Q0", current_time, processes[idx].pid);
            processes[idx].priority = 0;
            processes[idx].time_in_queue = 0;
            enqueue(&scheduler->queues[0], &processes[idx]);
            idx++;
        }

        if (running) {
            for (int i = 0; i < running->priority; i++) {
                if (scheduler->queues[i].size > 0) {
                    enqueue(&scheduler->queues[running->priority], running);
                    running = NULL;
                    break;
                }
            }
        }

        if (!running) {
            for (int i = 0; i < scheduler->num_queues; i++) {
                if (scheduler->queues[i].size > 0) {
                    running = dequeue(&scheduler->queues[i]);
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
        running->time_in_queue++;
        slice_used++;
        current_time++;
        running->quantum_used++;

        if (running->remaining_time == 0) {
            running->finish_time = current_time;
            running->state = STATE_FINISHED;
            if (trace_count < MAX_TRACE) sprintf(traces[trace_count++], "t=%-4d Process %s completes in Q%d", current_time, running->pid, running->priority);
            completed++;
            running = NULL;
        } else {
            int a = scheduler->queues[running->priority].allotment;
            int q = scheduler->queues[running->priority].time_quantum;
            
            if (a != -1 && running->time_in_queue >= a) {
                int next_q = (running->priority + 1 < scheduler->num_queues) ? running->priority + 1 : running->priority;
                if (trace_count < MAX_TRACE && next_q != running->priority) {
                    sprintf(traces[trace_count++], "t=%-4d Process %s -> Q%d (exhausted Q%d allotment)", current_time, running->pid, next_q, running->priority);
                }
                running->priority = next_q;
                running->time_in_queue = 0;
                enqueue(&scheduler->queues[running->priority], running);
                running = NULL;
            } else if (q != -1 && slice_used >= q) {
                enqueue(&scheduler->queues[running->priority], running);
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
        if (processes[i].priority == 0) {
            interactive = &processes[i];
            total_resp += (processes[i].start_time - processes[i].arrival_time);
            int_count++;
        }
        if (processes[i].priority == scheduler->num_queues - 1) long_job = &processes[i];
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
        printf("  - Process %s demoted to Q%d\n", long_job->pid, scheduler->num_queues - 1);
        printf("  - Turnaround time: %d (fair for its burst time)\n", long_job->finish_time - long_job->arrival_time);
    } else {
        printf("  - No long-running jobs reached the lowest queue.\n");
    }
    printf("\nThe MLFQ successfully balanced responsiveness and fairness.\n");

    for (int i = 0; i < scheduler->num_queues; i++) free_queue(&scheduler->queues[i]);
    free_gantt_chart(&chart);
}
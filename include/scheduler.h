#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "process.h"

typedef struct {
    int level;
    int time_quantum;
    int allotment;
    Process **queue;
    int size;

    int front;              
    int rear;               
    int capacity;           
} MLFQQueue;

typedef struct {
    MLFQQueue *queues;
    int num_queues;
    int boost_period;
    int last_boost;
} MLFQScheduler;

void simulate_fcfs(Process* processes, int num_processes);
void simulate_sjf(Process* processes, int num_processes);
void simulate_stcf(Process* processes, int num_processes);
void simulate_rr(Process* processes, int num_processes, int time_quantum);

void simulate_mlfq(Process* processes, int num_processes, MLFQScheduler* scheduler);

void calculate_and_print_metrics(Process* processes, int num_processes);
void print_gantt_chart();

#endif
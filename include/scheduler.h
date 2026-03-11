#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "process.h"
#include "event.h"

typedef struct {
    int num_queues;
    int* time_quantums;
    int boost_interval;
} MLFQ_Config;

void simulate_fcfs(Process* processes, int num_processes);

void simulate_sjf(Process* processes, int num_processes);

void simulate_stcf(Process* processes, int num_processes);

void simulate_rr(Process* processes, int num_processes, int time_quantum);

void simulate_mlfq(Process* processes, int num_processes, MLFQ_Config* config);

void calculate_and_print_metrics(Process* processes, int num_processes);

void print_gantt_chart();

#endif
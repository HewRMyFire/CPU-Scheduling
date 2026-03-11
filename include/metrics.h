#ifndef METRICS_H
#define METRICS_H
#include "process.h"

typedef struct {
    const char* algorithm_name;
    int num_processes;

    double avg_turnaround_time;
    double avg_waiting_time;
    double avg_response_time;

    int total_turnaround_time;
    int total_waiting_time;
    int total_response_time;

    int total_execution_time;
    double cpu_utilization;
    double throughput;

} SchedulingMetrics;

void calculate_metrics(SchedulingMetrics* metrics, Process* processes, int num_processes, const char* algo_name);

void print_metrics(const SchedulingMetrics* metrics);

void print_comparative_analysis(const SchedulingMetrics* metrics_array, int num_algorithms);

#endif
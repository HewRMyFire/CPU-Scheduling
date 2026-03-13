#include <stdio.h>
#include <string.h>
#include "metrics.h"
#include "process.h"

void calculate_metrics(SchedulingMetrics* metrics, Process* processes, int num_processes, const char* algo_name) {
    if (num_processes == 0) return;

    metrics->algorithm_name = algo_name;
    metrics->num_processes = num_processes;

    metrics->total_turnaround_time = 0;
    metrics->total_waiting_time = 0;
    metrics->total_response_time = 0;
    metrics->total_execution_time = 0;

    int max_completion_time = 0;
    int min_arrival_time = processes[0].arrival_time;

    for (int i = 0; i < num_processes; i++) {
        Process* p = &processes[i];

        int turnaround_time = p->completion_time - p->arrival_time;
        int waiting_time = turnaround_time - p->burst_time;
        int response_time = p->start_time - p->arrival_time;

        metrics->total_turnaround_time += turnaround_time;
        metrics->total_waiting_time += waiting_time;
        metrics->total_response_time += response_time;
        metrics->total_execution_time += p->burst_time;

        if (p->completion_time > max_completion_time) {
            max_completion_time = p->completion_time;
        }
        if (p->arrival_time < min_arrival_time) {
            min_arrival_time = p->arrival_time;
        }
    }

    metrics->avg_turnaround_time = (double)metrics->total_turnaround_time / num_processes;
    metrics->avg_waiting_time = (double)metrics->total_waiting_time / num_processes;
    metrics->avg_response_time = (double)metrics->total_response_time / num_processes;

    int total_schedule_time = max_completion_time - min_arrival_time;
    if (total_schedule_time > 0) {
        metrics->cpu_utilization = ((double)metrics->total_execution_time / total_schedule_time) * 100.0;
        metrics->throughput = (double)num_processes / total_schedule_time;
    } else {
        metrics->cpu_utilization = 0.0;
        metrics->throughput = 0.0;
    }
}

void print_metrics(const SchedulingMetrics* metrics) {
    printf("--- Metrics for %s ---\n", metrics->algorithm_name);
    printf("Average Turnaround Time : %.2f\n", metrics->avg_turnaround_time);
    printf("Average Waiting Time    : %.2f\n", metrics->avg_waiting_time);
    printf("Average Response Time   : %.2f\n", metrics->avg_response_time);
    printf("CPU Utilization         : %.2f%%\n", metrics->cpu_utilization);
    printf("Throughput              : %.4f processes/unit time\n", metrics->throughput);
    printf("----------------------------\n");
}

void print_comparative_analysis(const SchedulingMetrics* metrics_array, int num_algorithms) {
    printf("\n=== Comparative Analysis ===\n");
    printf("%-10s | %-12s | %-12s | %-12s | %-12s | %-10s\n",
           "Algorithm", "Avg Turn", "Avg Wait", "Avg Resp", "CPU Util(%)", "Throughput");
    printf("--------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_algorithms; i++) {
        const SchedulingMetrics* m = &metrics_array[i];
        printf("%-10s | %-12.2f | %-12.2f | %-12.2f | %-12.2f | %-10.4f\n",
               m->algorithm_name,
               m->avg_turnaround_time,
               m->avg_waiting_time,
               m->avg_response_time,
               m->cpu_utilization,
               m->throughput);
    }
    printf("============================\n");
}
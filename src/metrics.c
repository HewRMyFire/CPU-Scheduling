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

        p->turnaround_time = p->finish_time - p->arrival_time;

        p->waiting_time = p->turnaround_time - p->burst_time;
        if (p->waiting_time < 0) p->waiting_time = 0;

        p->response_time = p->start_time - p->arrival_time;

        metrics->total_turnaround_time += p->turnaround_time;
        metrics->total_waiting_time += p->waiting_time;
        metrics->total_response_time += p->response_time;
        metrics->total_execution_time += p->burst_time;

        if (p->finish_time > max_completion_time) max_completion_time = p->finish_time;
        if (p->arrival_time < min_arrival_time) min_arrival_time = p->arrival_time;
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

void print_metrics(const SchedulingMetrics* metrics, Process* processes) {
    printf("=== Metrics for %s ===\n", metrics->algorithm_name);

    int convoy_detected = 0;
    char convoy_pid[16] = "";
    int convoy_wait_time = 0;

    for (int i = 0; i < metrics->num_processes; i++) {
        Process* p = &processes[i];

        printf("Process %s:\n", p->pid);
        printf("  Arrival Time:           %d\n", p->arrival_time);
        printf("  Burst Time:             %d\n", p->burst_time);
        printf("  Finish Time:            %d\n", p->finish_time);

        printf("  Turnaround Time:  %d - %d = %d\n", p->finish_time, p->arrival_time, p->turnaround_time);
        printf("  Waiting Time:         %d - %d = %d\n", p->turnaround_time, p->burst_time, p->waiting_time);
        printf("  Response Time:     %d - %d = %d\n\n", p->start_time, p->arrival_time, p->response_time);

        if (p->waiting_time > p->burst_time && p->waiting_time > convoy_wait_time && p->burst_time > 0) {
            convoy_detected = 1;
            strcpy(convoy_pid, p->pid);
            convoy_wait_time = p->waiting_time;
        }
    }

    printf("--- Average Metrics ---\n");
    printf("Average Turnaround Time: %.2f\n", metrics->avg_turnaround_time);
    printf("Average Waiting Time:    %.2f\n", metrics->avg_waiting_time);
    printf("Average Response Time:   %.2f\n\n", metrics->avg_response_time);

    if (convoy_detected && strcmp(metrics->algorithm_name, "FCFS") == 0) {
        printf("Convoy effect detected: Process %s waited %d time units\n\n", convoy_pid, convoy_wait_time);
    }
}

void print_comparative_analysis(const SchedulingMetrics* metrics_array, int num_algorithms, const char* input_file) {
    if (input_file && strlen(input_file) > 0) {
        printf("\n=== Algorithm Comparison for %s ===\n\n", input_file);
    } else {
        printf("\n=== Algorithm Comparison ===\n\n");
    }
    
    printf("%-10s | %-6s | %-6s | %-6s | %-16s\n",
           "Algorithm", "Avg TT", "Avg WT", "Avg RT", "Context Switches");
    printf("-----------|--------|--------|--------|------------------\n");

    for (int i = 0; i < num_algorithms; i++) {
        const SchedulingMetrics* m = &metrics_array[i];
        
        printf("%-10s | %6.1f | %6.1f | %6.1f | %16d\n",
               m->algorithm_name,
               m->avg_turnaround_time,
               m->avg_waiting_time,
               m->avg_response_time,
               m->context_switches);
    }
    printf("\n");
}
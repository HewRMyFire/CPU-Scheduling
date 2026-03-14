#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "process.h"
#include "scheduler.h"
#include "metrics.h"
#include "gantt.h"
#include "utils.h"
#define MAX_PROCESSES 100

char* input_filename = NULL;
int time_quantum = 2;
int run_fcfs = 0, run_sjf = 0, run_stcf = 0, run_rr = 0, run_mlfq = 0, run_all = 0;

void print_usage(const char* prog_name) {
    printf("Usage: %s -f <input_file> [options]\n", prog_name);
    printf("Options:\n");
    printf("  -f <file>   Specify the input file containing process workloads (Required)\n");
    printf("  -q <int>    Time quantum for Round Robin (Default: 2)\n");
    printf("  -a          Run all scheduling algorithms\n");
    printf("  -1          Run FCFS\n");
    printf("  -2          Run SJF\n");
    printf("  -3          Run STCF\n");
    printf("  -4          Run RR\n");
    printf("  -5          Run MLFQ\n");
    printf("  -h          Print this help message\n");
}

void parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "f:q:a12345h")) != -1) {
        switch (opt) {
            case 'f': input_filename = optarg; break;
            case 'q': time_quantum = atoi(optarg); break;
            case 'a': run_all = 1; break;
            case '1': run_fcfs = 1; break;
            case '2': run_sjf = 1; break;
            case '3': run_stcf = 1; break;
            case '4': run_rr = 1; break;
            case '5': run_mlfq = 1; break;
            case 'h': print_usage(argv[0]); exit(EXIT_SUCCESS);
            default: print_usage(argv[0]); exit(EXIT_FAILURE);
        }
    }

    if (input_filename == NULL) {
        fprintf(stderr, "Error: Input file is required.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!run_fcfs && !run_sjf && !run_stcf && !run_rr && !run_mlfq && !run_all) {
        run_all = 1;
    }
}

int load_processes(const char* filename, Process* processes) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char pid[16];
    int arrival, burst;

    while (fscanf(file, "%15s %d %d", pid, &arrival, &burst) == 3) {
        if (count >= MAX_PROCESSES) {
            fprintf(stderr, "Warning: Maximum number of processes reached (%d).\n", MAX_PROCESSES);
            break;
        }
        init_process(&processes[count], pid, arrival, burst);
        count++;
    }

    fclose(file);
    return count;
}

int main(int argc, char* argv[]) {
    parse_arguments(argc, argv);

    Process original_processes[MAX_PROCESSES];
    Process current_processes[MAX_PROCESSES];
    int num_processes = load_processes(input_filename, original_processes);

    if (num_processes == 0) {
        fprintf(stderr, "Error: No valid processes found in %s\n", input_filename);
        return EXIT_FAILURE;
    }

    printf("Loaded %d processes from %s.\n", num_processes, input_filename);

    SchedulingMetrics metrics_array[5];
    int metrics_count = 0;

    if (run_fcfs || run_all) {
        printf("\n Running First-Come, First-Served (FCFS) \n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_fcfs(current_processes, num_processes);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "FCFS");
        print_metrics(&metrics_array[metrics_count]);
        metrics_count++;
    }

    if (run_sjf || run_all) {
        printf("\n Running Shortest Job First (SJF) \n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_sjf(current_processes, num_processes);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "SJF");
        print_metrics(&metrics_array[metrics_count]);
        metrics_count++;
    }

    if (run_stcf || run_all) {
        printf("\n Running Shortest Time-to-Completion First (STCF) \n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_stcf(current_processes, num_processes);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "STCF");
        print_metrics(&metrics_array[metrics_count]);
        metrics_count++;
    }

    if (run_rr || run_all) {
        printf("\n Running Round Robin (RR) [Quantum = %d] \n", time_quantum);
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_rr(current_processes, num_processes, time_quantum);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "RR");
        print_metrics(&metrics_array[metrics_count]);
        metrics_count++;
    }

    if (run_mlfq || run_all) {
        printf("\n Running Multi-Level Feedback Queue (MLFQ) \n");
        copy_process_array(current_processes, original_processes, num_processes);

        int quantums[] = {2, 4, 8};
        MLFQ_Config mlfq_config = { .num_queues = 3, .time_quantums = quantums, .boost_interval = 20 };

        simulate_mlfq(current_processes, num_processes, &mlfq_config);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "MLFQ");
        print_metrics(&metrics_array[metrics_count]);
        metrics_count++;
    }


    if (metrics_count > 1) {
        print_comparative_analysis(metrics_array, metrics_count);
    }

    return EXIT_SUCCESS;
}
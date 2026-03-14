#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "process.h"
#include "scheduler.h"
#include "metrics.h"
#include "gantt.h"
#include "utils.h"

#define MAX_PROCESSES 100

int run_fcfs = 0, run_sjf = 0, run_stcf = 0, run_rr = 0, run_mlfq = 0, run_all = 0;
int time_quantum = 2;

void print_usage(const char* prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  --algorithm=<name>   Specify algorithm (FCFS, SJF, STCF, RR, MLFQ, ALL)\n");
    printf("  --processes=<str>    Inline processes format 'PID:Arrival:Burst,...'\n");
    printf("  -f <file>            Specify the input file containing process workloads\n");
    printf("  -q <int>             Time quantum for Round Robin (Default: 2)\n");
}

int load_processes_from_file(const char* filename, Process* processes) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char pid[16];
        int arrival, burst;
        if (sscanf(line, "%15s %d %d", pid, &arrival, &burst) == 3) {
            if (count >= MAX_PROCESSES) break;
            init_process(&processes[count], pid, arrival, burst);
            count++;
        }
    }

    fclose(file);
    return count;
}

int load_processes_from_string(char* str, Process* processes) {
    int count = 0;
    char* token = strtok(str, ",");
    
    while (token != NULL) {
        char pid[16];
        int arrival, burst;
        if (sscanf(token, "%15[^:]:%d:%d", pid, &arrival, &burst) == 3) {
            if (count >= MAX_PROCESSES) break;
            init_process(&processes[count], pid, arrival, burst);
            count++;
        }
        token = strtok(NULL, ",");
    }
    return count;
}

int main(int argc, char* argv[]) {
    Process original_processes[MAX_PROCESSES];
    Process current_processes[MAX_PROCESSES];
    int num_processes = 0;
    
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:q:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                if (strcmp(optarg, "FCFS") == 0) run_fcfs = 1;
                else if (strcmp(optarg, "SJF") == 0) run_sjf = 1;
                else if (strcmp(optarg, "STCF") == 0) run_stcf = 1;
                else if (strcmp(optarg, "RR") == 0) run_rr = 1;
                else if (strcmp(optarg, "MLFQ") == 0) run_mlfq = 1;
                else if (strcmp(optarg, "ALL") == 0) run_all = 1;
                break;
            case 'p':
                num_processes = load_processes_from_string(optarg, original_processes);
                break;
            case 'f': 
                num_processes = load_processes_from_file(optarg, original_processes); 
                break;
            case 'q': time_quantum = atoi(optarg); break;
            case 'h': print_usage(argv[0]); exit(EXIT_SUCCESS);
            default: print_usage(argv[0]); exit(EXIT_FAILURE);
        }
    }

    if (num_processes == 0) {
        fprintf(stderr, "Error: No valid processes loaded.\n");
        return EXIT_FAILURE;
    }

    if (!run_fcfs && !run_sjf && !run_stcf && !run_rr && !run_mlfq) run_all = 1;

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
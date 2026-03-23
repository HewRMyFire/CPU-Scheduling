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
int time_quantum = 30; 

void print_usage(const char* prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  --algorithm=<name>   Specify algorithm (FCFS, SJF, STCF, RR, MLFQ, ALL)\n");
    printf("  --processes=<str>    Inline processes format 'PID:Arrival:Burst,...'\n");
    printf("  --input=<file>       Specify input file containing process workloads\n");
    printf("  --mlfq-config=<file> Specify MLFQ configuration file\n");
    printf("  --quantum=<int>      Time quantum for Round Robin (Default: 30)\n");
    printf("  -f <file>            Specify input file containing process workloads\n");
    printf("  -q <int>             Time quantum for Round Robin\n");
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

int load_mlfq_config(const char* filename, MLFQ_Config* config) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    int q_count = 0;
    char line[256];
    
    while(fgets(line, sizeof(line), file)) {
        if(line[0] == 'Q' && line[1] >= '0' && line[1] <= '9') q_count++;
    }
    
    config->num_queues = q_count;
    config->time_quantums = (int*)malloc(q_count * sizeof(int));
    config->allotments = (int*)malloc(q_count * sizeof(int));
    
    rewind(file);
    while(fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (strncmp(line, "BOOST_PERIOD", 12) == 0) {
            sscanf(line, "BOOST_PERIOD %d", &config->boost_interval);
        } else if (line[0] == 'Q') {
            int qid, tq, allmt;
            if (sscanf(line, "Q%d %d %d", &qid, &tq, &allmt) == 3) {
                if (qid < q_count) {
                    config->time_quantums[qid] = tq;
                    config->allotments[qid] = allmt;
                }
            }
        }
    }
    fclose(file);
    return 1;
}

int main(int argc, char* argv[]) {
    Process original_processes[MAX_PROCESSES];
    Process current_processes[MAX_PROCESSES];
    int num_processes = 0;
    char mlfq_config_file[256] = "";
    
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'}, 
        {"quantum", required_argument, 0, 'q'}, 
        {"mlfq-config", required_argument, 0, 'm'}, 
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:q:hm:i:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                if (strcmp(optarg, "FCFS") == 0) run_fcfs = 1;
                else if (strcmp(optarg, "SJF") == 0) run_sjf = 1;
                else if (strcmp(optarg, "STCF") == 0) run_stcf = 1;
                else if (strcmp(optarg, "RR") == 0) run_rr = 1;
                else if (strcmp(optarg, "MLFQ") == 0) run_mlfq = 1;
                else if (strcmp(optarg, "ALL") == 0) run_all = 1;
                break;
            case 'p': num_processes = load_processes_from_string(optarg, original_processes); break;
            case 'i': 
            case 'f': num_processes = load_processes_from_file(optarg, original_processes); break;
            case 'q': time_quantum = atoi(optarg); break;
            case 'm': strncpy(mlfq_config_file, optarg, 255); break;
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
        printf("\nRunning First-Come, First-Served (FCFS) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_fcfs(current_processes, num_processes);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "FCFS");
        print_metrics(&metrics_array[metrics_count], current_processes);
        metrics_count++;
    }

    if (run_sjf || run_all) {
        printf("\nRunning Shortest Job First (SJF) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_sjf(current_processes, num_processes);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "SJF");
        print_metrics(&metrics_array[metrics_count], current_processes); 
        metrics_count++;
    }

    if (run_stcf || run_all) {
        printf("\nRunning Shortest Time-to-Completion First (STCF) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_stcf(current_processes, num_processes);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "STCF");
        print_metrics(&metrics_array[metrics_count], current_processes); 
        metrics_count++;
    }

    if (run_rr || run_all) {
        printf("\nRunning Round Robin (RR) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        simulate_rr(current_processes, num_processes, time_quantum);
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "RR");
        print_metrics(&metrics_array[metrics_count], current_processes); 
        metrics_count++;
    }

    if (run_mlfq || run_all) {
        printf("\nRunning Multi-Level Feedback Queue (MLFQ) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        
        MLFQ_Config mlfq_config;
        int def_q[3] = {10, 30, -1};
        int def_a[3] = {50, 150, -1};
        mlfq_config.num_queues = 3;
        mlfq_config.time_quantums = def_q;
        mlfq_config.allotments = def_a;
        mlfq_config.boost_interval = 200;
        int allocated_config = 0;

        if (strlen(mlfq_config_file) > 0) {
            if (load_mlfq_config(mlfq_config_file, &mlfq_config)) {
                allocated_config = 1;
            } else {
                fprintf(stderr, "Warning: Failed to load MLFQ config, using defaults.\n");
            }
        }
        
        simulate_mlfq(current_processes, num_processes, &mlfq_config);
        
        calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "MLFQ");
        print_metrics(&metrics_array[metrics_count], current_processes);
        metrics_count++;

        if (allocated_config) {
            free(mlfq_config.time_quantums);
            free(mlfq_config.allotments);
        }
    }

    if (metrics_count > 1) {
        print_comparative_analysis(metrics_array, metrics_count);
    }

    return EXIT_SUCCESS;
}
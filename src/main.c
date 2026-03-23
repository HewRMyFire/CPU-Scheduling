#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include "process.h"
#include "scheduler.h"
#include "metrics.h"
#include "gantt.h"
#include "utils.h"
#include "parser.h"

#define MAX_PROCESSES 100

#ifdef _WIN32
#define DEV_NULL "NUL"
#else
#define DEV_NULL "/dev/null"
#endif

int run_fcfs = 0, run_sjf = 0, run_stcf = 0, run_rr = 0, run_mlfq = 0, run_all = 0;
int time_quantum = 30; 
int compare_mode = 0;
int saved_stdout;

void suppress_output() {
    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    int fd = open(DEV_NULL, O_WRONLY);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

void restore_output() {
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

const char* get_basename(const char* path) {
    const char* base = strrchr(path, '/');
#ifdef _WIN32
    const char* base_win = strrchr(path, '\\');
    if (base_win > base) base = base_win;
#endif
    return base ? base + 1 : path;
}

void print_usage(const char* prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  --algorithm=<name>   Specify algorithm (FCFS, SJF, STCF, RR, MLFQ, ALL)\n");
    printf("  --processes=<str>    Inline processes format 'PID:Arrival:Burst,...'\n");
    printf("  --input=<file>       Specify input file containing process workloads\n");
    printf("  --mlfq-config=<file> Specify MLFQ configuration file\n");
    printf("  --compare            Run all algorithms silently and display a comparison table\n");
    printf("  --quantum=<int>      Time quantum for Round Robin (Default: 30)\n");
    printf("  -f <file>            Specify input file containing process workloads\n");
    printf("  -q <int>             Time quantum for Round Robin\n");
    printf("  -c                   Alias for --compare\n");
}

int main(int argc, char* argv[]) {
    Process original_processes[MAX_PROCESSES];
    Process current_processes[MAX_PROCESSES];
    int num_processes = 0;
    
    char input_filename[256] = "inline_workload";
    char mlfq_config_file[256] = "";
    static char rr_name[32]; 

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'}, 
        {"quantum", required_argument, 0, 'q'}, 
        {"mlfq-config", required_argument, 0, 'm'}, 
        {"compare", no_argument, 0, 'c'}, 
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:q:hm:i:c", long_options, &option_index)) != -1) {
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
                num_processes = load_processes_from_string(optarg, original_processes, MAX_PROCESSES); 
                break;
            case 'c': compare_mode = 1; run_all = 1; break;
            case 'i': 
            case 'f': 
                strncpy(input_filename, get_basename(optarg), 255);
                num_processes = load_processes_from_file(optarg, original_processes, MAX_PROCESSES); 
                break;
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
        if (!compare_mode) printf("\nRunning First-Come, First-Served (FCFS) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        
        SchedulerState state = { .processes = current_processes, .num_processes = num_processes, .current_time = 0 };
        
        if (compare_mode) suppress_output();
        if (schedule_fcfs(&state) == 0) {
            if (compare_mode) restore_output();
            calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "FCFS");
            metrics_array[metrics_count].context_switches = get_last_context_switches(num_processes);
            if (!compare_mode) print_metrics(&metrics_array[metrics_count], current_processes);
            metrics_count++;
        } else if (compare_mode) restore_output();
    }

    if (run_sjf || run_all) {
        if (!compare_mode) printf("\nRunning Shortest Job First (SJF) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        
        SchedulerState state = { .processes = current_processes, .num_processes = num_processes, .current_time = 0 };
        
        if (compare_mode) suppress_output();
        if (schedule_sjf(&state) == 0) {
            if (compare_mode) restore_output();
            calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "SJF");
            metrics_array[metrics_count].context_switches = get_last_context_switches(num_processes);
            if (!compare_mode) print_metrics(&metrics_array[metrics_count], current_processes); 
            metrics_count++;
        } else if (compare_mode) restore_output();
    }

    if (run_stcf || run_all) {
        if (!compare_mode) printf("\nRunning Shortest Time-to-Completion First (STCF) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        
        SchedulerState state = { .processes = current_processes, .num_processes = num_processes, .current_time = 0 };
        
        if (compare_mode) suppress_output();
        if (schedule_stcf(&state) == 0) {
            if (compare_mode) restore_output();
            calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "STCF");
            metrics_array[metrics_count].context_switches = get_last_context_switches(num_processes);
            if (!compare_mode) print_metrics(&metrics_array[metrics_count], current_processes); 
            metrics_count++;
        } else if (compare_mode) restore_output();
    }

    if (run_rr || run_all) {
        if (!compare_mode) printf("\nRunning Round Robin (RR) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        
        sprintf(rr_name, "RR (q=%d)", time_quantum);
        SchedulerState state = { .processes = current_processes, .num_processes = num_processes, .current_time = 0 };
        
        if (compare_mode) suppress_output();
        if (schedule_rr(&state, time_quantum) == 0) {
            if (compare_mode) restore_output();
            calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, rr_name);
            metrics_array[metrics_count].context_switches = get_last_context_switches(num_processes);
            if (!compare_mode) print_metrics(&metrics_array[metrics_count], current_processes); 
            metrics_count++;
        } else if (compare_mode) restore_output();
    }

    if (run_mlfq || run_all) {
        if (!compare_mode) printf("\nRunning Multi-Level Feedback Queue (MLFQ) Scheduler...\n");
        copy_process_array(current_processes, original_processes, num_processes);
        
        MLFQ_Config parsed_config = { .num_queues = 3, .time_quantums = (int[]){10, 30, -1}, .allotments = (int[]){50, 150, -1}, .boost_interval = 200 };
        int allocated = (strlen(mlfq_config_file) > 0 && load_mlfq_config(mlfq_config_file, &parsed_config));
        
        MLFQScheduler mlfq_sched;
        mlfq_sched.num_queues = parsed_config.num_queues;
        mlfq_sched.boost_period = parsed_config.boost_interval;
        mlfq_sched.queues = (MLFQQueue*)malloc(mlfq_sched.num_queues * sizeof(MLFQQueue));
        
        for(int i = 0; i < mlfq_sched.num_queues; i++) {
            mlfq_sched.queues[i].level = i;
            mlfq_sched.queues[i].time_quantum = parsed_config.time_quantums[i];
            mlfq_sched.queues[i].allotment = parsed_config.allotments[i];
        }

        SchedulerState state = { .processes = current_processes, .num_processes = num_processes, .current_time = 0 };
        
        if (compare_mode) suppress_output();
        if (schedule_mlfq(&state, &mlfq_sched) == 0) {
            if (compare_mode) restore_output();
            calculate_metrics(&metrics_array[metrics_count], current_processes, num_processes, "MLFQ");
            metrics_array[metrics_count].context_switches = get_last_context_switches(num_processes);
            if (!compare_mode) print_metrics(&metrics_array[metrics_count], current_processes);
            metrics_count++;
        } else if (compare_mode) restore_output();

        free(mlfq_sched.queues);
        if (allocated) { free(parsed_config.time_quantums); free(parsed_config.allotments); }
    }

    if (metrics_count > 1) print_comparative_analysis(metrics_array, metrics_count, input_filename);

    return EXIT_SUCCESS;
}
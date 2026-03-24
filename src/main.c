#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include "process.h"
#include "scheduler.h"
#include "scheduler_registry.h"
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

static int run_scheduler(const char* scheduler_name, Process* current_processes, 
                        Process* original_processes, int num_processes,
                        int time_quantum, void* scheduler_config,
                        SchedulingMetrics* metrics, int compare_mode) {
    int result = 0;
    const SchedulerEntry* entry = NULL;
    const SchedulerEntry* available = get_available_schedulers(NULL);
    
    for (int i = 0; available[i].name != NULL; i++) {
        if (strcmp(available[i].name, scheduler_name) == 0) {
            entry = &available[i];
            break;
        }
    }
    
    if (entry == NULL) {
        fprintf(stderr, "Error: Unknown scheduler '%s'\n", scheduler_name);
        return -1;
    }
    
    if (!compare_mode) printf("\nRunning %s...\n", entry->full_name);
    copy_process_array(current_processes, original_processes, num_processes);
    
    SchedulerState state = {
        .processes = current_processes,
        .num_processes = num_processes,
        .current_time = 0
    };
    
    char algo_name_buffer[64];
    if (strcmp(scheduler_name, "RR") == 0) {
        snprintf(algo_name_buffer, sizeof(algo_name_buffer), "RR (q=%d)", time_quantum);
    } else {
        strncpy(algo_name_buffer, scheduler_name, sizeof(algo_name_buffer) - 1);
    }
    
    if (compare_mode) suppress_output();
    result = entry->run(&state, time_quantum, scheduler_config);
    
    if (result == 0) {
        if (compare_mode) restore_output();
        calculate_metrics(metrics, current_processes, num_processes, algo_name_buffer);
        metrics->context_switches = get_last_context_switches(num_processes);
        if (!compare_mode) print_metrics(metrics, current_processes);
        return 0;
    } else if (compare_mode) {
        restore_output();
    }
    
    return -1;
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

    int opt;
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'}, 
        {"quantum", required_argument, 0, 'q'}, 
        {"mlfq-config", required_argument, 0, 'm'}, 
        {"compare", no_argument, 0, 'c'}, 
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:q:hm:i:c", long_options, NULL)) != -1) {
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
            case 'f': {
                const char* base = strrchr(optarg, '/');
#ifdef _WIN32
                const char* base_win = strrchr(optarg, '\\');
                if (base_win > base) base = base_win;
#endif
                strncpy(input_filename, base ? base + 1 : optarg, 255);
                num_processes = load_processes_from_file(optarg, original_processes, MAX_PROCESSES); 
                break;
            }
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
    
    struct {
        const char* name;
        int* flag;
    } scheduler_flags[] = {
        {"FCFS", &run_fcfs},
        {"SJF", &run_sjf},
        {"STCF", &run_stcf},
        {"RR", &run_rr},
        {"MLFQ", &run_mlfq},
        {NULL, NULL}
    };
    
    for (int i = 0; scheduler_flags[i].name != NULL; i++) {
        if (*scheduler_flags[i].flag || run_all) {
            void* scheduler_config = NULL;
            
            if (strcmp(scheduler_flags[i].name, "MLFQ") == 0) {
                MLFQ_Config parsed_config;
                
                parsed_config.num_queues = 3;
                parsed_config.time_quantums = (int*)malloc(3 * sizeof(int));
                parsed_config.time_quantums[0] = 10;
                parsed_config.time_quantums[1] = 30;
                parsed_config.time_quantums[2] = -1;
                
                parsed_config.allotments = (int*)malloc(3 * sizeof(int));
                parsed_config.allotments[0] = 50;
                parsed_config.allotments[1] = 150;
                parsed_config.allotments[2] = -1;
                
                parsed_config.boost_interval = 200;
                
                int allocated = (strlen(mlfq_config_file) > 0 && 
                                load_mlfq_config(mlfq_config_file, &parsed_config));
                
                MLFQScheduler mlfq_sched;
                mlfq_sched.num_queues = parsed_config.num_queues;
                mlfq_sched.boost_period = parsed_config.boost_interval;
                mlfq_sched.queues = (MLFQQueue*)malloc(mlfq_sched.num_queues * sizeof(MLFQQueue));
                
                for(int j = 0; j < mlfq_sched.num_queues; j++) {
                    mlfq_sched.queues[j].time_quantum = parsed_config.time_quantums[j];
                    mlfq_sched.queues[j].allotment = parsed_config.allotments[j];
                }
                
                scheduler_config = &mlfq_sched;
                
                if (run_scheduler(scheduler_flags[i].name, current_processes, original_processes,
                                num_processes, time_quantum, scheduler_config,
                                &metrics_array[metrics_count], compare_mode) == 0) {
                    metrics_count++;
                }
                
                free(mlfq_sched.queues);
                if (allocated) {
                    free(parsed_config.time_quantums);
                    free(parsed_config.allotments);
                }
            } else {
                if (run_scheduler(scheduler_flags[i].name, current_processes, original_processes,
                                num_processes, time_quantum, NULL,
                                &metrics_array[metrics_count], compare_mode) == 0) {
                    metrics_count++;
                }
            }
        }
    }

    if (metrics_count > 1) print_comparative_analysis(metrics_array, metrics_count, input_filename);

    return EXIT_SUCCESS;
}
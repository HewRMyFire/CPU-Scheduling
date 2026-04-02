#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "scheduler_runner.h"
#include "scheduler_registry.h"
#include "metrics.h"
#include "gantt.h"
#include "utils.h"
#include "parser.h"

#ifdef _WIN32
#define DEV_NULL "NUL"
#else
#define DEV_NULL "/dev/null"
#endif

static int saved_stdout;

static void suppress_output() {
    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    int fd = open(DEV_NULL, O_WRONLY);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

static void restore_output() {
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}

int run_single_scheduler(const char* scheduler_name, Process* current_processes,
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
        metrics->context_switches = get_last_context_switches(&state.chart, num_processes);
        if (!compare_mode) print_metrics(metrics, current_processes);
        return 0;
    } else if (compare_mode) {
        restore_output();
    }
    
    return -1;
}

int run_all_schedulers(Process* original_processes, int num_processes,
                       int time_quantum, const char* mlfq_config_file,
                       int run_all, int run_fcfs, int run_sjf,
                       int run_stcf, int run_rr, int run_mlfq,
                       SchedulingMetrics* metrics_array,
                       int compare_mode) {
    int metrics_count = 0;
    
    struct {
        const char* name;
        int flag;
    } scheduler_flags[] = {
        {"FCFS", run_fcfs},
        {"SJF", run_sjf},
        {"STCF", run_stcf},
        {"RR", run_rr},
        {"MLFQ", run_mlfq},
        {NULL, 0}
    };
    
    Process* current_processes = (Process*)malloc(num_processes * sizeof(Process));
    if (current_processes == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return -1;
    }
    
    for (int i = 0; scheduler_flags[i].name != NULL; i++) {
        if (scheduler_flags[i].flag || run_all) {
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
                
                if (run_single_scheduler(scheduler_flags[i].name, current_processes, original_processes,
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
                if (run_single_scheduler(scheduler_flags[i].name, current_processes, original_processes,
                                        num_processes, time_quantum, NULL,
                                        &metrics_array[metrics_count], compare_mode) == 0) {
                    metrics_count++;
                }
            }
        }
    }
    
    free(current_processes);
    return metrics_count;
}

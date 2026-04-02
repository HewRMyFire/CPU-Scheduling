#ifndef SCHEDULER_RUNNER_H
#define SCHEDULER_RUNNER_H

#include "scheduler.h"
#include "metrics.h"
#include "process.h"

/**
 * Run a single scheduler and collect its metrics
 * 
 * @param scheduler_name Name of scheduler (FCFS, SJF, STCF, RR, MLFQ)
 * @param current_processes Working copy of processes to schedule
 * @param original_processes Original process list to copy from
 * @param num_processes Number of processes
 * @param time_quantum Time quantum for RR (ignored for other schedulers)
 * @param scheduler_config Optional scheduler-specific config (for MLFQ)
 * @param metrics Output metrics structure
 * @param compare_mode If true, suppress output during execution
 * @return 0 on success, -1 on failure
 */
int run_single_scheduler(const char* scheduler_name, Process* current_processes,
                         Process* original_processes, int num_processes,
                         int time_quantum, void* scheduler_config,
                         SchedulingMetrics* metrics, int compare_mode);

/**
 * Setup and run all requested schedulers
 * 
 * @param original_processes Original process list
 * @param num_processes Number of processes
 * @param time_quantum Time quantum for RR
 * @param mlfq_config_file Path to MLFQ config file (can be empty string)
 * @param run_all If true, run all algorithms
 * @param run_fcfs, run_sjf, run_stcf, run_rr, run_mlfq Individual algorithm flags
 * @param metrics_array Output array of metrics
 * @param compare_mode If true, only print comparative analysis at end
 * @return Number of schedulers successfully executed
 */
int run_all_schedulers(Process* original_processes, int num_processes,
                       int time_quantum, const char* mlfq_config_file,
                       int run_all, int run_fcfs, int run_sjf, 
                       int run_stcf, int run_rr, int run_mlfq,
                       SchedulingMetrics* metrics_array,
                       int compare_mode);

#endif

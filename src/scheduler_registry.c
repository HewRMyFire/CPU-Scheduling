#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler_registry.h"
#include "scheduler.h"

static int run_fcfs_wrapper(SchedulerState *state, int param1, void* param2) {
    (void)param1; (void)param2;
    return schedule_fcfs(state);
}

static int run_sjf_wrapper(SchedulerState *state, int param1, void* param2) {
    (void)param1; (void)param2;
    return schedule_sjf(state);
}

static int run_stcf_wrapper(SchedulerState *state, int param1, void* param2) {
    (void)param1; (void)param2;
    return schedule_stcf(state);
}

static int run_rr_wrapper(SchedulerState *state, int param1, void* param2) {
    (void)param2;
    return schedule_rr(state, param1);
}

static int run_mlfq_wrapper(SchedulerState *state, int param1, void* param2) {
    (void)param1;
    return schedule_mlfq(state, (MLFQScheduler*)param2);
}

static const SchedulerEntry schedulers[] = {
    {"FCFS", "First-Come, First-Served (FCFS) Scheduler", run_fcfs_wrapper},
    {"SJF", "Shortest Job First (SJF) Scheduler", run_sjf_wrapper},
    {"STCF", "Shortest Time-to-Completion First (STCF) Scheduler", run_stcf_wrapper},
    {"RR", "Round Robin (RR) Scheduler", run_rr_wrapper},
    {"MLFQ", "Multi-Level Feedback Queue (MLFQ) Scheduler", run_mlfq_wrapper},
    {NULL, NULL, NULL}
};

int execute_scheduler(const char* name, SchedulerState *state, int param1, void* param2,
                      const char** algo_name_out) {
    if (algo_name_out) *algo_name_out = name;
    
    for (int i = 0; schedulers[i].name != NULL; i++) {
        if (strcmp(schedulers[i].name, name) == 0) {
            return schedulers[i].run(state, param1, param2);
        }
    }
    return -1;
}

const SchedulerEntry* get_available_schedulers(int* count) {
    int i = 0;
    while (schedulers[i].name != NULL) i++;
    if (count != NULL) {
        *count = i;
    }
    return schedulers;
}

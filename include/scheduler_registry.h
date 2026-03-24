#ifndef SCHEDULER_REGISTRY_H
#define SCHEDULER_REGISTRY_H

#include "scheduler.h"

typedef struct {
    const char* name;
    const char* full_name;
    int (*run)(SchedulerState *state, int param1, void* param2);
} SchedulerEntry;

int execute_scheduler(const char* name, SchedulerState *state, int param1, void* param2, 
                      const char** algo_name_out);

const SchedulerEntry* get_available_schedulers(int* count);

#endif

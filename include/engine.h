#ifndef ENGINE_H
#define ENGINE_H
#include "scheduler.h"

typedef struct {
    void (*handle_arrival)(SchedulerState *state, Process *process);
    void (*handle_completion)(SchedulerState *state, Process *process);
    void (*handle_quantum_expire)(SchedulerState *state, Process *process);
    void (*handle_priority_boost)(SchedulerState *state);
} SchedulerOps;

void simulate_scheduler(SchedulerState *state, SchedulerOps *ops);

#endif
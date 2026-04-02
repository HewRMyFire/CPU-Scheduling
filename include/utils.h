#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "process.h"
#include "scheduler.h"

int compare_arrival_time(const void* a, const void* b);

void copy_process_array(Process* dest, const Process* src, int count);

void* safe_malloc(size_t size);

void init_ready_queue_scheduler(SchedulerState *state);
void init_arrival_events(SchedulerState *state);
void process_completion_handler(SchedulerState *state, Process *p);

// Queue abstraction layer
void queue_enqueue_fifo(SchedulerState *state, Process *p);
Process* queue_dequeue(SchedulerState *state);
Process* queue_peek(SchedulerState *state);
void queue_enqueue_sorted(SchedulerState *state, Process *p, int (*compare)(const Process*, const Process*));
int queue_size(SchedulerState *state);
void queue_clear(SchedulerState *state);

void finalize_scheduler(SchedulerState *state);

#endif
#ifndef ENGINE_H
#define ENGINE_H
#include "scheduler.h"
#include "events.h"
#include "gantt.h"

typedef enum {
    ALGO_FCFS,
    ALGO_SJF,
    ALGO_STCF,
    ALGO_RR,
    ALGO_MLFQ
} SchedulingAlgorithm;

typedef struct {
    Process *processes;
    int num_processes;
    int current_time;
    GanttChart chart;

    Event *event_queue;
    Process *current_running;

    Process **ready_queue;
    int rq_front;
    int rq_rear;
    int rq_size;
} DES_State;

void init_des_state(DES_State *state, Process *procs, int num);
void free_des_state(DES_State *state);

void simulate_scheduler_des(DES_State *state, SchedulingAlgorithm algorithm);

#endif
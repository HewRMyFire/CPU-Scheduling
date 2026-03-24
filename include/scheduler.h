#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "process.h"
#include "gantt.h"
#include "events.h" 

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
    
    void *algo_data;
} SchedulerState;

typedef struct {
    int time_quantum;       
    int allotment;          
    Process **queue;        
    int size;               
    int front;              
    int rear;               
    int capacity;           
} MLFQQueue;

typedef struct {
    MLFQQueue *queues;      
    int num_queues;         
    int boost_period;       
    int last_boost;         
} MLFQScheduler;

typedef struct {
    int num_queues;
    int* time_quantums;
    int* allotments;
    int boost_interval;
} MLFQ_Config;

int schedule_fcfs(SchedulerState *state);
int schedule_sjf(SchedulerState *state);
int schedule_stcf(SchedulerState *state);
int schedule_rr(SchedulerState *state, int time_quantum);
int schedule_mlfq(SchedulerState *state, MLFQScheduler* scheduler);

static inline void init_process_start_time(Process *p, SchedulerState *state) {
    if (p->start_time == -1) {
        p->start_time = state->current_time;
    }
}

static inline void dequeue_ready_queue(SchedulerState *state, Process **p) {
    *p = state->ready_queue[state->rq_front];
    state->rq_front = (state->rq_front + 1) % state->num_processes;
    state->rq_size--;
}

static inline void set_process_running(Process *p, SchedulerState *state) {
    state->current_running = p;
    p->state = STATE_RUNNING;
}

#endif
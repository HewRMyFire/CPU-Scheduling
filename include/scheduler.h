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
} SchedulerState;

typedef struct {
    int level;              
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

#endif
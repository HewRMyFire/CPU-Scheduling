#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    STATE_READY,
    STATE_RUNNING,
    STATE_FINISHED
} ProcessState;

typedef struct {
    char pid[16];           
    int arrival_time;       
    int burst_time;         
    int remaining_time;     
    int start_time;         
    int finish_time;        
    
    int turnaround_time;    
    int waiting_time;       
    int response_time;      
    
    int priority;           
    int time_in_queue;      
    
    ProcessState state;     
    int quantum_used;       
} Process;

void init_process(Process* p, const char* pid, int arrival, int burst);

#endif
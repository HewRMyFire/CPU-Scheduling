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
    int completion_time;

    ProcessState state;

    int queue_level;
    int quantum_used;

} Process;

void init_process(Process* p, const char* pid, int arrival, int burst);

#endif
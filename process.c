#include "process.h"
#include <string.h>

void init_process(Process* p, const char* pid, int arrival, int burst) {
    strncpy(p->pid, pid, sizeof(p->pid) - 1);
    p->pid[sizeof(p->pid) - 1] = '\0';

    p->arrival_time = arrival;
    p->burst_time = burst;

    p->remaining_time = burst;
    p->start_time = -1;
    p->completion_time = -1;

    p->state = STATE_READY;

    p->queue_level = 0;
    p->quantum_used = 0;
}
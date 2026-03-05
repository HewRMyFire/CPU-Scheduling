#include "event.h"
#include "process.h"
#include <stdio.h>
#include <stdlib.h>

int current_time = 0;
Process* current_running_process = NULL;

void handle_arrival(Process* p) {
    printf("[%d] Process %s arrived.\n", current_time, p->pid);
}

void handle_completion(Process* p) {
    p->state = STATE_FINISHED;
    p->completion_time = current_time;
    printf("[%d] Process %s completed.\n", current_time, p->pid);
    current_running_process = NULL;

}

void handle_time_slice_expired(Process* p) {
    printf("[%d] Time slice expired for %s.\n", current_time, p->pid);
    p->state = STATE_READY;
    current_running_process = NULL;

}

void run_simulation(EventQueue* eq) {
    printf("--- Starting Discrete-Event Simulation ---\n");

    while (!is_empty(eq)) {
        Event* ev = dequeue_event(eq);

        if (current_time < ev->time) {
            current_time = ev->time;
        }

        switch (ev->type) {
            case EVENT_PROCESS_ARRIVAL:
                handle_arrival(ev->process);
                break;

            case EVENT_PROCESS_COMPLETION:
                handle_completion(ev->process);
                break;

            case EVENT_TIME_SLICE_EXPIRED:
                handle_time_slice_expired(ev->process);
                break;
        }

        free(ev);
    }

    printf("--- Simulation Finished at time %d ---\n", current_time);
}

int main() {
    EventQueue eq;
    init_event_queue(&eq);

    Process p1, p2;
    init_process(&p1, "P1", 0, 10);
    init_process(&p2, "P2", 2, 5);

    enqueue_event(&eq, p1.arrival_time, EVENT_PROCESS_ARRIVAL, &p1);
    enqueue_event(&eq, p2.arrival_time, EVENT_PROCESS_ARRIVAL, &p2);

    run_simulation(&eq);

    return 0;
}
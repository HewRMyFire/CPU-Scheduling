#include <stdio.h>
#include <stdlib.h>
#include "process.h"
#include "scheduler.h"
#include "events.h"
#include "gantt.h"

int compare_arrival_time(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }
    return p1->pid[0] - p2->pid[0];
}

void copy_process_array(Process* dest, const Process* src, int count) {
    for (int i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void init_ready_queue_scheduler(SchedulerState *state) {
    state->current_time = 0;
    state->event_queue = NULL;
    state->current_running = NULL;
    state->ready_queue = (Process **)malloc(state->num_processes * sizeof(Process *));
    state->rq_front = 0;
    state->rq_rear = 0;
    state->rq_size = 0;
    init_gantt_chart(&state->chart);
}

void init_arrival_events(SchedulerState *state) {
    for (int i = 0; i < state->num_processes; i++) {
        push_event(&state->event_queue, state->processes[i].arrival_time, EVENT_ARRIVAL, &state->processes[i]);
    }
}

void process_completion_handler(SchedulerState *state, Process *p) {
    p->finish_time = state->current_time;
    p->state = STATE_FINISHED;
    p->remaining_time = 0;
    state->current_running = NULL;
}

// Queue abstraction layer implementation

void queue_enqueue_fifo(SchedulerState *state, Process *p) {
    if (state->rq_size >= state->num_processes) return;  // Queue full
    state->ready_queue[state->rq_rear] = p;
    state->rq_rear = (state->rq_rear + 1) % state->num_processes;
    state->rq_size++;
}

Process* queue_dequeue(SchedulerState *state) {
    if (state->rq_size <= 0) return NULL;
    Process* p = state->ready_queue[state->rq_front];
    state->rq_front = (state->rq_front + 1) % state->num_processes;
    state->rq_size--;
    return p;
}

Process* queue_peek(SchedulerState *state) {
    if (state->rq_size <= 0) return NULL;
    return state->ready_queue[state->rq_front];
}

int queue_size(SchedulerState *state) {
    return state->rq_size;
}

void queue_clear(SchedulerState *state) {
    state->rq_front = 0;
    state->rq_rear = 0;
    state->rq_size = 0;
}

// Sorted insertion for priority queues (SJF, STCF)
void queue_enqueue_sorted(SchedulerState *state, Process *p, int (*compare)(const Process*, const Process*)) {
    if (state->rq_size >= state->num_processes) return;  // Queue full
    
    // Temporarily extract all current queue elements
    Process **temp = (Process **)malloc(state->rq_size * sizeof(Process *));
    for (int i = 0; i < state->rq_size; i++) {
        temp[i] = state->ready_queue[(state->rq_front + i) % state->num_processes];
    }
    
    // Find insertion position by comparing with temp array
    int insert_pos = state->rq_size;
    for (int i = 0; i < state->rq_size; i++) {
        if (compare(temp[i], p) > 0) {
            insert_pos = i;
            break;
        }
    }
    
    // Rebuild queue linearly: copy back with new element inserted
    state->rq_front = 0;
    int new_idx = 0;
    
    // Copy elements before insertion point
    for (int i = 0; i < insert_pos; i++) {
        state->ready_queue[new_idx++] = temp[i];
    }
    
    // Insert new process
    state->ready_queue[new_idx++] = p;
    
    // Copy elements after insertion point
    for (int i = insert_pos; i < state->rq_size; i++) {
        state->ready_queue[new_idx++] = temp[i];
    }
    
    state->rq_rear = new_idx;
    state->rq_size = new_idx;
    free(temp);
}

// Backward compatibility wrapper
void enqueue_ready_queue_fifo(SchedulerState *state, Process *p) {
    queue_enqueue_fifo(state, p);
}

void finalize_scheduler(SchedulerState *state) {
    print_gantt_chart(&state->chart);
    free(state->ready_queue);
    free_gantt_chart(&state->chart);
}
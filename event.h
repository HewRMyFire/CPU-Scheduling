#ifndef EVENT_H
#define EVENT_h

#include "process.h"

typedef enum {
    EVENT_PROCESS_ARRIVAL,
    EVENT_TIME_SLICE_EXPIRED,
    EVENT_PROCESS_COMPLETION
} EventType;

typedef struct Event {
    int time;
    EventType type;
    Process* process;
    struct Event* next;
} Event;

typedef struct {
    Event* head;
} EventQueue;

void init_event_queue(EventQueue* eq);
void enqueue_event(EventQueue* eq, int time, EventType type, Process* p);
Event* dequeue_event(EventQueue* eq);
int is_empty(EventQueue* eq);

#endif
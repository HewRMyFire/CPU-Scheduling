#ifndef EVENTS_H
#define EVENTS_H
#include "process.h"

typedef enum {
    EVENT_ARRIVAL = 0,      
    EVENT_PRIORITY_BOOST,
    EVENT_QUANTUM_EXPIRE,
    EVENT_COMPLETION
} EventType;

typedef struct Event {
    int time;
    EventType type;
    Process *process;
    struct Event *next;
} Event;

void push_event(Event **queue, int time, EventType type, Process *process);
Event *pop_event(Event **queue);
void cancel_process_events(Event **queue, Process *process);

#endif
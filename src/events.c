#include <stdlib.h>
#include "events.h"

void push_event(Event **queue, int time, EventType type, Process *process) {
    Event *new_event = (Event *)malloc(sizeof(Event));
    new_event->time = time;
    new_event->type = type;
    new_event->process = process;
    new_event->next = NULL;

    if (*queue == NULL || (*queue)->time > time || 
       ((*queue)->time == time && (*queue)->type > type)) {
        new_event->next = *queue;
        *queue = new_event;
        return;
    }

    Event *current = *queue;
    while (current->next != NULL) {
        if (current->next->time > time) break;
        if (current->next->time == time && current->next->type > type) break;
        current = current->next;
    }

    new_event->next = current->next;
    current->next = new_event;
}

Event *pop_event(Event **queue) {
    if (*queue == NULL) return NULL;
    Event *top = *queue;
    *queue = (*queue)->next;
    return top;
}

void cancel_process_events(Event **queue, Process *process) {
    Event *current = *queue;
    Event *prev = NULL;

    while (current != NULL) {
        if (current->process == process) {
            if (prev == NULL) {
                *queue = current->next;
                free(current);
                current = *queue;
            } else {
                prev->next = current->next;
                free(current);
                current = prev->next;
            }
        } else {
            prev = current;
            current = current->next;
        }
    }
}
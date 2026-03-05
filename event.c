#include "event.h"
#include <stdlib.h>
#include <stdio.h>

void init_event_queue(EventQueue* eq) {
    eq->head = NULL;
}

void enqueue_event (EventQueue*eq, int time, EventType type, Process* p) {
    Event* new_event = (Event*)malloc(sizeof(Event));
    if (!new_event) {
        perror("Failed to allocate memory for event");
        exit(EXIT_FAILURE);
    }

    new_event->time = time;
    new_event->type = type;
    new_event->process = p;
    new_event->next = NULL;

    if (eq->head == NULL || time < eq->head->time) {
        new_event->next = eq->head;
        eq->head = new_event;
        return;
    }

    Event* current = eq-> head;
    while (current->next != NULL && current->next->time <= time) {
        current = current->next;
    }

    new_event->next = current->next;
    current->next = new_event;
}

Event* dequeue_event(EventQueue* eq) {
    if (eq->head == NULL) {
        return NULL;
    }

    Event* top_event = eq->head;
    eq->head = eq->head->next;
    return top_event;
}

int is_empty(EventQueue* eq) {
    return eq->head == NULL;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gantt.h"
#include "utils.h"

void init_gantt_chart(GanttChart* chart) {
    if (chart != NULL) {
        chart->head = NULL;
        chart->tail = NULL;
    }
}

void add_gantt_segment(GanttChart* chart, const char* pid, int start_time, int end_time) {
    if (chart == NULL) return;

    if (chart->tail != NULL && strcmp(chart->tail->pid, pid) == 0) {
        chart->tail->end_time = end_time;
        return;
    }

    GanttSegment* new_seg = (GanttSegment*)safe_malloc(sizeof(GanttSegment));

    strncpy(new_seg->pid, pid, sizeof(new_seg->pid) - 1);
    new_seg->pid[sizeof(new_seg->pid) - 1] = '\0';
    new_seg->start_time = start_time;
    new_seg->end_time = end_time;
    new_seg->next = NULL;

    if (chart->tail == NULL) {
        chart->head = new_seg;
        chart->tail = new_seg;
    } else {
        chart->tail->next = new_seg;
        chart->tail = new_seg;
    }
}

void print_gantt_chart(const GanttChart* chart) {
    if (chart == NULL || chart->head == NULL) {
        printf("Empty Gantt Chart\n");
        return;
    }

    GanttSegment* current = chart->head;

    while (current != NULL) {
        printf("---------");
        current = current->next;
    }
    printf("-\n");

    current = chart->head;
    while (current != NULL) {
        printf("| %-5s ", current->pid);
        current = current->next;
    }
    printf("|\n");

    current = chart->head;
    while (current != NULL) {
        printf("---------");
        current = current->next;
    }
    printf("-\n");

    current = chart->head;
    printf("%-9d", current->start_time);
    while (current != NULL) {
        printf("%-9d", current->end_time);
        current = current->next;
    }
    printf("\n");
}

void free_gantt_chart(GanttChart* chart) {
    if (chart == NULL) return;

    GanttSegment* current = chart->head;
    while (current != NULL) {
        GanttSegment* next = current->next;
        free(current);
        current = next;
    }

    chart->head = NULL;
    chart->tail = NULL;
}
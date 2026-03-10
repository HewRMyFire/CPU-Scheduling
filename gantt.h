#ifndef GANTT_H
#define GANTT_H

#include "process.h"

typedef struct GanttSegment {
    char pid[16];
    int start_time;
    int end_time;
    struct GanttSegment* next;
} GanttSegment;

typedef struct {
    GanttSegment* head;
    GanttSegment* tail;
} GanttChart;

void init_gantt_chart(GanttChart* chart);

void add_gantt_segment(GanttChart* chart, const char* pid, int start_time, int end_time);

void print_gantt_chart(const GanttChart* chart);

void free_gantt_chart(GanttChart* chart);

#endif
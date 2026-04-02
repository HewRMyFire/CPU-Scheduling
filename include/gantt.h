#ifndef GANTT_H
#define GANTT_H

typedef struct GanttSegment {
    char pid[16];
    int start_time;
    int end_time;
    struct GanttSegment* next;
} GanttSegment;

typedef struct {
    GanttSegment* head;
    GanttSegment* tail;
    int dispatch_count;
} GanttChart;

void init_gantt_chart(GanttChart* chart);
void add_gantt_segment(GanttChart* chart, const char* pid, int start_time, int end_time);
void print_gantt_chart(const GanttChart* chart);
void free_gantt_chart(GanttChart* chart);

int get_last_context_switches(const GanttChart* chart, int num_processes);

#endif
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

    int total_time = chart->tail->end_time;

    int scale = (total_time > 60) ? (total_time / 60) + 1 : 1;

    printf("=== Gantt Chart ===\n");
    if (scale > 1) {
        printf("Scale: Each character represents ~%d time units\n", scale);
    }

    GanttSegment* current = chart->head;
    while (current != NULL) {
        int duration = current->end_time - current->start_time;
        int chars = duration / scale;
        if (chars < 1) chars = 1;

        printf("[");
        for(int i = 0; i < chars; i++) {
            if (strcmp(current->pid, "IDLE") == 0) {
                printf("-");
            } else {
                printf("%c", current->pid[0]);
            }
        }
        printf("]");
        current = current->next;
    }
    printf("\n");

    current = chart->head;
    char buf[32];
    sprintf(buf, "Time: %d", current->start_time);
    printf("%s", buf);
    
    int time_cursor = strlen(buf);
    int block_cursor = 0;

    while (current != NULL) {
        int duration = current->end_time - current->start_time;
        int chars = duration / scale;
        if (chars < 1) chars = 1;
        
        block_cursor += (chars + 2);
        
        char time_str[32];
        sprintf(time_str, "%d", current->end_time);
        int time_len = strlen(time_str);

        int target_pos = block_cursor - (time_len / 2);
        
        if (target_pos <= time_cursor) {
            printf(" ");
            time_cursor += 1;
        } else {
            while (time_cursor < target_pos) {
                printf(" ");
                time_cursor++;
            }
        }
        printf("%s", time_str);
        time_cursor += time_len;
        
        current = current->next;
    }
    printf("\n\n");
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
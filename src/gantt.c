#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gantt.h"
#include "utils.h"

void init_gantt_chart(GanttChart* chart) {
    if (chart != NULL) {
        chart->head = NULL;
        chart->tail = NULL;
        chart->dispatch_count = 0;
    }
}

void add_gantt_segment(GanttChart* chart, const char* pid, int start_time, int end_time) {
    if (chart == NULL) return;

    if (chart->tail != NULL && strcmp(chart->tail->pid, pid) == 0) {
        chart->tail->end_time = end_time;
        return;
    }

    if (strcmp(pid, "IDLE") != 0) {
        chart->dispatch_count++;
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
    int scale = (total_time > 50) ? (total_time / 50) + 1 : 1;
    int max_width = 55;
    int chart_width = 0;
    
    // Calculate chart width based on segments
    GanttSegment* temp = chart->head;
    while (temp != NULL) {
        int duration = temp->end_time - temp->start_time;
        int chars = (duration + scale - 1) / scale;  // Ceiling division
        if (chars < 1) chars = 1;
        chart_width += chars + 2;  // +2 for brackets
        temp = temp->next;
    }
    
    // Constrain to max width
    if (chart_width > max_width) {
        chart_width = max_width;
    }

    // Header
    printf("\n┌─ GANTT CHART ─────────────────────────────────────────────┐\n");
    printf("  Total Time: %d units\n", total_time);
    
    if (scale > 1) {
        printf("  Scale: 1 char ≈ %d time unit%s\n", scale, (scale > 1 ? "s" : ""));
    }

    // Collect process list
    printf("  Processes: ");
    GanttSegment* current = chart->head;
    int legend_pos = 13;
    while (current != NULL) {
        if (strcmp(current->pid, "IDLE") != 0) {
            int len = printf("%s ", current->pid);
            legend_pos += len;
            if (legend_pos > 65) {
                printf("\n              ");
                legend_pos = 14;
            }
        }
        current = current->next;
    }
    printf("\n");
    printf("├────────────────────────────────────────────────────────────┤\n");

    // Gantt bar
    printf("  ");
    current = chart->head;
    while (current != NULL) {
        int duration = current->end_time - current->start_time;
        int chars = (duration + scale - 1) / scale;
        if (chars < 1) chars = 1;

        printf("[");
        for(int i = 0; i < chars; i++) {
            if (strcmp(current->pid, "IDLE") == 0) {
                printf(" ");
            } else {
                printf("%c", current->pid[0]);
            }
        }
        printf("]");
        current = current->next;
    }
    printf("\n");

    // Tick marks row
    printf("  ");
    current = chart->head;
    while (current != NULL) {
        int duration = current->end_time - current->start_time;
        int chars = (duration + scale - 1) / scale;
        if (chars < 1) chars = 1;
        printf("+");
        for (int i = 0; i < chars + 1; i++) {
            printf("-");
        }
        current = current->next;
    }
    printf("\n");

    // Print time values under boundaries
    printf("  ");
    current = chart->head;
    int col_pos = 2;
    
    // Start time
    printf("%d", current->start_time);
    int start_len = (current->start_time == 0 ? 1 : 
                     (current->start_time > 99 ? 3 : 2));
    col_pos += start_len;

    // End times for each segment
    while (current != NULL) {
        int duration = current->end_time - current->start_time;
        int chars = (duration + scale - 1) / scale;
        if (chars < 1) chars = 1;
        
        // Position for next time value (at end of segment)
        int next_col = col_pos + chars + 2;  // +2 for bracket
        char time_str[16];
        int time_len = sprintf(time_str, "%d", current->end_time);
        
        // Move to position for time value
        int spaces_needed = next_col - col_pos - start_len;
        if (spaces_needed > 0) {
            printf("%*s", spaces_needed, "");
        }
        printf("%s", time_str);
        
        col_pos = next_col;
        start_len = time_len;
        current = current->next;
    }
    printf("\n");
    printf("└────────────────────────────────────────────────────────────┘\n\n");
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

int get_last_context_switches(const GanttChart* chart, int num_processes) {
    if (chart == NULL) return 0;
    int switches = chart->dispatch_count - num_processes;
    return (switches < 0) ? 0 : switches;
}
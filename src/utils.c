#include <stdio.h>
#include <stdlib.h>
#include "process.h"

int compare_arrival_time(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }
    return p1->pid[0] - p2->pid[0];
}

int util_max(int a, int b) {
    return (a > b) ? a : b;
}

int util_min(int a, int b) {
    return (a < b) ? a : b;
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
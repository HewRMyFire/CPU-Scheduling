#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "process.h"

int compare_arrival_time(const void* a, const void* b);

int util_max(int a, int b);
int util_min(int a, int b);

void copy_process_array(Process* dest, const Process* src, int count);

void* safe_malloc(size_t size);

#endif
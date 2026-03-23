#ifndef PARSER_H
#define PARSER_H

#include "process.h"
#include "scheduler.h"

int load_processes_from_file(const char* filename, Process* processes, int max_processes);
int load_processes_from_string(char* str, Process* processes, int max_processes);
int load_mlfq_config(const char* filename, MLFQ_Config* config);

#endif
#ifndef PARSER_H
#define PARSER_H

#include "process.h"
#include "scheduler.h"

typedef struct {
    int run_fcfs;
    int run_sjf;
    int run_stcf;
    int run_rr;
    int run_mlfq;
    int run_all;
    int time_quantum;
    int compare_mode;
    char input_filename[256];       // Display name (base filename)
    char input_filepath[256];       // Full path for loading
    char mlfq_config_file[256];
} CommandLineArgs;

int load_processes_from_file(const char* filename, Process* processes, int max_processes);
int load_processes_from_string(char* str, Process* processes, int max_processes);
int load_mlfq_config(const char* filename, MLFQ_Config* config);

void parse_command_line_args(int argc, char* argv[], CommandLineArgs* args);
void print_usage(const char* prog_name);

#endif
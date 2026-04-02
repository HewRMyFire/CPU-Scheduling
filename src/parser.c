#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "parser.h"

void print_usage(const char* prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  --algorithm=<name>   Specify algorithm (FCFS, SJF, STCF, RR, MLFQ, ALL)\n");
    printf("  --processes=<str>    Inline processes format 'PID:Arrival:Burst,...'\n");
    printf("  --input=<file>       Specify input file containing process workloads\n");
    printf("  --mlfq-config=<file> Specify MLFQ configuration file\n");
    printf("  --compare            Run all algorithms silently and display a comparison table\n");
    printf("  --quantum=<int>      Time quantum for Round Robin (Default: 30)\n");
    printf("  -f <file>            Specify input file containing process workloads\n");
    printf("  -q <int>             Time quantum for Round Robin\n");
    printf("  -c                   Alias for --compare\n");
}

void parse_command_line_args(int argc, char* argv[], CommandLineArgs* args) {
    // Initialize defaults
    args->run_fcfs = 0;
    args->run_sjf = 0;
    args->run_stcf = 0;
    args->run_rr = 0;
    args->run_mlfq = 0;
    args->run_all = 0;
    args->time_quantum = 30;
    args->compare_mode = 0;
    strncpy(args->input_filename, "inline_workload", sizeof(args->input_filename) - 1);
    args->input_filename[sizeof(args->input_filename) - 1] = '\0';
    args->input_filepath[0] = '\0';
    args->mlfq_config_file[0] = '\0';

    int opt;
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'}, 
        {"quantum", required_argument, 0, 'q'}, 
        {"mlfq-config", required_argument, 0, 'm'}, 
        {"compare", no_argument, 0, 'c'}, 
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:q:hm:i:c", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                if (strcmp(optarg, "FCFS") == 0) args->run_fcfs = 1;
                else if (strcmp(optarg, "SJF") == 0) args->run_sjf = 1;
                else if (strcmp(optarg, "STCF") == 0) args->run_stcf = 1;
                else if (strcmp(optarg, "RR") == 0) args->run_rr = 1;
                else if (strcmp(optarg, "MLFQ") == 0) args->run_mlfq = 1;
                else if (strcmp(optarg, "ALL") == 0) args->run_all = 1;
                break;
            case 'p':
                // Note: This is handled in main with load_processes_from_string
                break;
            case 'c': 
                args->compare_mode = 1; 
                args->run_all = 1; 
                break;
            case 'i':
            case 'f': {
                // Store full path for loading
                strncpy(args->input_filepath, optarg, 255);
                args->input_filepath[255] = '\0';
                
                // Store base filename for display
                const char* base = strrchr(optarg, '/');
#ifdef _WIN32
                const char* base_win = strrchr(optarg, '\\');
                if (base_win > base) base = base_win;
#endif
                strncpy(args->input_filename, base ? base + 1 : optarg, 255);
                args->input_filename[255] = '\0';
                break;
            }
            case 'q': 
                args->time_quantum = atoi(optarg); 
                break;
            case 'm': 
                strncpy(args->mlfq_config_file, optarg, 255); 
                args->mlfq_config_file[255] = '\0';
                break;
            case 'h': 
                print_usage(argv[0]); 
                exit(EXIT_SUCCESS);
            default: 
                print_usage(argv[0]); 
                exit(EXIT_FAILURE);
        }
    }
}

int load_processes_from_file(const char* filename, Process* processes, int max_processes) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char pid[16];
        int arrival, burst;
        if (sscanf(line, "%15s %d %d", pid, &arrival, &burst) == 3) {
            if (count >= max_processes) break;
            init_process(&processes[count], pid, arrival, burst);
            count++;
        }
    }

    fclose(file);
    return count;
}

int load_processes_from_string(char* str, Process* processes, int max_processes) {
    int count = 0;
    char* token = strtok(str, ",");
    
    while (token != NULL) {
        char pid[16];
        int arrival, burst;
        if (sscanf(token, "%15[^:]:%d:%d", pid, &arrival, &burst) == 3) {
            if (count >= max_processes) break;
            init_process(&processes[count], pid, arrival, burst);
            count++;
        }
        token = strtok(NULL, ",");
    }
    return count;
}

int load_mlfq_config(const char* filename, MLFQ_Config* config) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    int q_count = 0;
    char line[256];

    while(fgets(line, sizeof(line), file)) {
        if(line[0] == 'Q' && line[1] >= '0' && line[1] <= '9') q_count++;
    }
    
    config->num_queues = q_count;
    config->time_quantums = (int*)malloc(q_count * sizeof(int));
    config->allotments = (int*)malloc(q_count * sizeof(int));

    rewind(file);
    while(fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (strncmp(line, "BOOST_PERIOD", 12) == 0) {
            sscanf(line, "BOOST_PERIOD %d", &config->boost_interval);
        } else if (line[0] == 'Q') {
            int qid, tq, allmt;
            if (sscanf(line, "Q%d %d %d", &qid, &tq, &allmt) == 3) {
                if (qid < q_count) {
                    config->time_quantums[qid] = tq;
                    config->allotments[qid] = allmt;
                }
            }
        }
    }
    fclose(file);
    return 1;
}
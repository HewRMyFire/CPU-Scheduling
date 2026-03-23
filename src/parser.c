#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

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
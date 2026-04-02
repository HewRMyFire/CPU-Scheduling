#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "process.h"
#include "scheduler.h"
#include "metrics.h"
#include "utils.h"
#include "parser.h"
#include "scheduler_runner.h"

#define MAX_PROCESSES 100

int main(int argc, char* argv[]) {
    Process original_processes[MAX_PROCESSES];
    int num_processes = 0;
    
    CommandLineArgs args;
    parse_command_line_args(argc, argv, &args);
    
    // Handle process loading separately since -p is handled during getopt
    optind = 1;  // Reset getopt
    int opt;
    while ((opt = getopt_long(argc, argv, "f:q:hm:i:c", (struct option[]){
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'}, 
        {"quantum", required_argument, 0, 'q'}, 
        {"mlfq-config", required_argument, 0, 'm'}, 
        {"compare", no_argument, 0, 'c'}, 
        {0, 0, 0, 0}
    }, NULL)) != -1) {
        if (opt == 'p') {
            num_processes = load_processes_from_string(optarg, original_processes, MAX_PROCESSES);
            break;
        }
    }
    
    // If no processes from -p, try loading from file
    if (num_processes == 0) {
        // Use input_filepath if provided, otherwise use input_filename
        const char* filepath = args.input_filepath[0] != '\0' ? args.input_filepath : args.input_filename;
        num_processes = load_processes_from_file(filepath, original_processes, MAX_PROCESSES);
    }

    if (num_processes == 0) {
        fprintf(stderr, "Error: No valid processes loaded.\n");
        return EXIT_FAILURE;
    }

    if (!args.run_fcfs && !args.run_sjf && !args.run_stcf && !args.run_rr && !args.run_mlfq) {
        args.run_all = 1;
    }

    SchedulingMetrics metrics_array[5];
    int metrics_count = run_all_schedulers(original_processes, num_processes,
                                           args.time_quantum, args.mlfq_config_file,
                                           args.run_all, args.run_fcfs, args.run_sjf,
                                           args.run_stcf, args.run_rr, args.run_mlfq,
                                           metrics_array, args.compare_mode);

    if (metrics_count > 1) print_comparative_analysis(metrics_array, metrics_count, args.input_filename);

    return EXIT_SUCCESS;
}
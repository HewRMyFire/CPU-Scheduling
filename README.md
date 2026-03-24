# CPU Scheduling Simulator

A comprehensive educational simulator for CPU scheduling algorithms. This project implements and demonstrates five major scheduling algorithms used in modern operating systems, with detailed performance metrics and visual Gantt charts.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Supported Algorithms](#supported-algorithms)
- [Project Structure](#project-structure)
- [Building the Project](#building-the-project)
- [Usage](#usage)
  - [Basic Examples](#basic-examples)
  - [Running Specific Algorithms](#running-specific-algorithms)
  - [Comparison Mode](#comparison-mode)
  - [Input File Format](#input-file-format)
  - [MLFQ Configuration](#mlfq-configuration)
- [Performance Metrics](#performance-metrics)
- [MLFQ Design Details](#mlfq-design-details)
- [Example Workloads](#example-workloads)
- [Output Format](#output-format)

## Overview

This simulator provides a realistic implementation of CPU scheduling algorithms used in operating systems. It allows you to:

- Simulate five different scheduling strategies
- Compare their performance on the same workload
- Visualize execution with Gantt charts
- Analyze key scheduling metrics (turnaround time, response time, wait time)
- Customize MLFQ (Multi-Level Feedback Queue) parameters

Perfect for students learning operating systems or professionals analyzing scheduling behavior.

## Features

✅ **Multiple Scheduling Algorithms** - FCFS, SJF, STCF, Round Robin, and MLFQ
✅ **Performance Metrics** - Turnaround time, response time, wait time, and CPU utilization
✅ **Gantt Chart Visualization** - Clear visual representation of process execution timelines
✅ **Comparison Mode** - Run all algorithms on the same workload simultaneously
✅ **Custom Input** - Define processes via command-line or input files
✅ **Configurable MLFQ** - Customize queues, time quanta, and boost periods
✅ **Event-Driven Simulation** - Accurate simulation using event queues

## Supported Algorithms

### 1. **FCFS (First Come, First Served)**
- Simplest scheduling algorithm
- Processes execute in arrival order
- Non-preemptive
- **Use case:** Batch processing systems

### 2. **SJF (Shortest Job First)**
- Non-preemptive algorithm
- Minimizes average waiting time
- Requires knowledge of CPU burst time
- **Drawback:** Long processes may starve

### 3. **STCF (Shortest Time To Completion First)**
- Preemptive variant of SJF
- Switches to shorter remaining jobs
- Better with preemption overhead consideration
- **Advantage:** Optimal average turnaround time

### 4. **Round Robin (RR)**
- Time-sharing scheduler
- Preemptive with configurable time quantum
- Fair processor access for all processes
- **Use case:** Interactive systems (default in many OS)

### 5. **MLFQ (Multi-Level Feedback Queue)**
- Adaptive, feedback-based scheduler
- Separates processes by priority levels
- Prevents process starvation with priority boosting
- **Features:** 
  - Multiple priority queues
  - Dynamic priority adjustment based on behavior
  - Configurable queue levels, time quanta, and boost intervals
  - Optimized for mixed workloads
- **See** [MLFQ Design](docs/mlfq_design.md) for detailed architecture

## Project Structure

```
.
├── Makefile                 # Build configuration
├── README.md               # This file
├── build/                  # Compiled binaries and objects
│   └── schedsim           # Executable
├── docs/
│   └── mlfq_design.md     # Detailed MLFQ architecture documentation
├── include/               # Header files
│   ├── engine.h           # Simulation engine
│   ├── events.h           # Event management
│   ├── gantt.h            # Gantt chart generation
│   ├── metrics.h          # Performance metrics calculation
│   ├── parser.h           # Input parsing
│   ├── process.h          # Process data structures
│   ├── scheduler.h        # Scheduler definitions
│   └── utils.h            # Utility functions
├── src/                   # Implementation files
│   ├── engine.c           # Core simulation engine
│   ├── events.c           # Event queue management
│   ├── fcfs.c             # FCFS implementation
│   ├── gantt.c            # Gantt chart rendering
│   ├── main.c             # CLI and entry point
│   ├── metrics.c          # Metrics calculation
│   ├── mlfq.c             # MLFQ implementation
│   ├── parser.c           # Input parser
│   ├── process.c          # Process management
│   ├── rr.c               # Round Robin implementation
│   ├── sjf.c              # SJF implementation
│   ├── stcf.c             # STCF implementation
│   └── utils.c            # Utility functions
└── tests/
    └── workload1.txt      # Example workload file
```

## Building the Project

### Prerequisites
- GCC compiler
- Make
- Linux/Unix environment (or compatible terminal on Windows)

### Compilation

```bash
# Compile the project
make

# Build output
Build successful! Run with: ./build/schedsim --algorithm=FCFS --processes="A:0:240"

# Clean build artifacts
make clean
```

**Compiler Flags Used:**
- `-Wall -Wextra -Wpedantic` - Enable all warnings
- `-Iinclude` - Include header directory
- `-g` - Debug symbols
- `-MMD -MP` - Dependency generation

## Usage

### Basic Examples

#### Run Single Algorithm with Inline Processes
```bash
./build/schedsim --algorithm=FCFS --processes="A:0:240,B:10:180,C:20:150"
```

Format: `PID:ArrivalTime:BurstTime,PID:ArrivalTime:BurstTime,...`

#### Run Round Robin with Custom Time Quantum
```bash
./build/schedsim --algorithm=RR --quantum=5 --processes="A:0:100,B:5:80"
```

#### Run MLFQ Scheduler
```bash
./build/schedsim --algorithm=MLFQ --processes="A:0:240,B:10:180"
```

#### Run from Input File
```bash
./build/schedsim --algorithm=SJF --input=tests/workload1.txt
```

### Running Specific Algorithms

```bash
# Run FCFS only
./build/schedsim --algorithm=FCFS --processes="A:0:100,B:5:50,C:10:75"

# Run STCF only
./build/schedsim --algorithm=STCF --processes="A:0:100,B:5:50"

# Run all algorithms on same workload
./build/schedsim --algorithm=ALL --processes="A:0:100,B:5:50,C:10:75"
```

### Comparison Mode

Compare all algorithms side-by-side with a summary table:

```bash
./build/schedsim --compare --processes="A:0:240,B:10:180,C:20:150,D:25:80,E:30:130"
```

**Output includes:**
- Individual algorithm Gantt charts
- Performance metrics summary table
- Statistical analysis

### Input File Format

Create a text file with processes (one per line):
```
ProcessID ArrivalTime BurstTime
A 0 240
B 10 180
C 20 150
D 25 80
E 30 130
```

Run with:
```bash
./build/schedsim --algorithm=MLFQ --input=tests/workload1.txt
```

### MLFQ Configuration

By default, MLFQ uses:
- **Queues:** 3 levels
- **Time Quanta:** 10, 30 time units
- **Allotments:** Based on algorithm parameters
- **Boost Interval:** 200 time units

To use a custom config:
```bash
./build/schedsim --algorithm=MLFQ --processes="A:0:240" --mlfq-config=custom_mlfq.conf
```

**Configuration file format:**
```
num_queues 3
time_quantums 2 4 8
allotments 2 8 16
boost_interval 20
```

## Performance Metrics

The simulator calculates and displays:

- **Turnaround Time** - Total time from arrival to completion (includes wait + execution)
- **Response Time** - Time from arrival to first execution
- **Wait Time** - Total time process spends in ready queue
- **CPU Utilization** - Percentage of time CPU is executing (vs idle)
- **Throughput** - Number of processes completed per unit time

## MLFQ Design Details

### Architecture Overview

The Multi-Level Feedback Queue implements a sophisticated scheduling strategy designed to provide:
- **Responsiveness** for interactive processes
- **Efficiency** for CPU-bound tasks
- **Fairness** through priority boosting

### Key Design Decisions

#### Three-Tier Queue Structure
- **Queue 0 (Highest Priority):** Newly arrived and interactive processes (short bursts)
- **Queue 1 (Medium Priority):** Processes requiring moderate CPU time
- **Queue 2 (Lowest Priority):** Long-running, CPU-bound tasks

#### Time Quantum Scaling
- **Queue 0:** 10 time units (responsive to interactive tasks)
- **Queue 1:** 30 time units (balanced)
- **Queue 2:** Flexible (accommodates long-running jobs)

#### Priority Demotion
- Processes that exceed their allotment at a level are moved to the next lower queue
- Prevents high-priority processes from monopolizing the CPU

#### Starvation Prevention
- **Priority Boost:** Every N time units, all processes are promoted back to Queue 0
- Ensures fairness and prevents indefinite starvation
- Default boost interval: 200 time units

### Performance Under Mixed Workloads

The design was validated against realistic workloads combining:
- Short interactive jobs (40-80 time units)
- Medium background tasks (90-150 time units)
- Long-running CPU-bound jobs (180-240 time units)

**Empirical Results:**
- ✅ Short jobs complete quickly (high responsiveness)
- ✅ Natural job filtering through queue levels
- ✅ Long jobs eventually receive CPU time (no starvation)
- ✅ Reduced context-switching overhead for heavy processes

For complete MLFQ analysis, see [MLFQ Design Documentation](docs/mlfq_design.md).

## Example Workloads

### Workload 1: Mixed Processes (Default Test Case)
Located in `tests/workload1.txt`:
```
A 0 240    # Arrives immediately, CPU intensive
B 10 180   # Short delay, moderate load
C 20 150   # Medium delay, moderate load
D 25 80    # Medium delay, light load
E 30 130   # Later arrival, moderate load
```

**Expected Behavior:**
- FCFS: Processes execute in order, long wait times for later arrivals
- SJF: D executes early, then A completes first due to ordering
- STCF: D completes first (shortest remaining), responsive to new arrivals
- RR: Fair round-robin execution, good interactive response
- MLFQ: Short jobs get priority, long jobs eventually complete

### Running the Example
```bash
./build/schedsim --algorithm=ALL --input=tests/workload1.txt
./build/schedsim --compare --input=tests/workload1.txt
```

## Output Format

### Gantt Chart
```
FCFS Gantt Chart:
+----+----+----+----+----+
| A  | B  | C  | D  | E  |
+----+----+----+----+----+
0   240  420  570  650  780
```

### Metrics Table
```
Algorithm | Avg Turnaround | Avg Response | Avg Wait | CPU Util
----------|----------------|--------------|----------|----------
FCFS      | 385.00         | 154.00      | 235.00  | 98.72%
SJF       | 325.00         | 104.00      | 175.00  | 98.72%
STCF      | 305.00         | 80.00       | 155.00  | 98.72%
RR        | 345.00         | 125.00      | 195.00  | 98.72%
MLFQ      | 315.00         | 90.00       | 165.00  | 98.72%
```

## Command-Line Reference

```
Options:
  --algorithm=<name>      Scheduling algorithm: FCFS, SJF, STCF, RR, MLFQ, ALL
  --processes=<str>       Inline processes: 'PID:Arrival:Burst,...'
  --input=<file> / -f     Load processes from file
  --quantum=<int> / -q    Time quantum for Round Robin (default: 30)
  --mlfq-config=<file>    MLFQ configuration file path
  --compare / -c          Compare all algorithms with metrics table
  -h                      Display help message
```

## Notes

- **Preemption:** STCF and RR use preemption; FCFS and SJF are non-preemptive
- **Arrival Times:** Processes can have staggered arrivals; use time 0 for immediate arrival
- **Burst Times:** Must be positive integers representing CPU time needed
- **Quantum Size:** Affects RR and MLFQ performance; typically 5-50 time units
- **Boost Interval:** MLFQ boost period should be tuned based on workload characteristics

## Educational Value

This simulator is designed to help students understand:
- Trade-offs between scheduling algorithms
- Impact of preemption on performance
- Starvation and fairness issues
- Feedback-based scheduler design
- Performance metric analysis

## License

Educational project for CMSC 125 - Operating Systems
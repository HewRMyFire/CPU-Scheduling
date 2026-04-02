# CPU Scheduling Simulator

A comprehensive educational simulator for CPU scheduling algorithms. This project implements and demonstrates five major scheduling algorithms used in modern operating systems, with detailed performance metrics and visual Gantt charts.

## Group Members

- **[Rainier RJ S. Espinal]**
- **[Matthew F. Simpas]**

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Supported Algorithms](#supported-algorithms)
- [Compilation and Building](#compilation-and-building)
- [Usage Instructions](#usage-instructions)
- [Example Commands & Expected Output](#example-commands--expected-output)
- [Known Limitations & Assumptions](#known-limitations--assumptions)
- [Project Structure](#project-structure)
- [MLFQ Design Details](#mlfq-design-details)

---

## Overview

This simulator provides a realistic implementation of CPU scheduling algorithms used in operating systems. It allows you to simulate different scheduling strategies, compare their performance on the same workload, visualize execution with Gantt charts, and analyze key scheduling metrics.

---

## Features

| Feature | Description |
|---|---|
| **Multiple Scheduling Algorithms** | FCFS, SJF, STCF, Round Robin, and MLFQ |
| **Performance Metrics** | Turnaround time, response time, wait time, context switch tracking |
| **Gantt Chart Visualization** | Clean, scalable visual representation of process execution timelines with automatic scaling for large workloads |
| **Comparison Mode** | Run all algorithms on the same workload simultaneously to output a comparative performance table |
| **Custom Input** | Define processes via inline command-line arguments or dedicated input files |
| **Configurable MLFQ** | Customize queues, time quanta, allotments, and boost periods via config files |
| **Automated Testing** | Built-in bash scripts for regression testing across algorithm implementations |

---

## Recent Improvements

- **Enhanced Gantt Chart Display** — Improved visual clarity with proper scaling for long workloads, eliminating padding overflow issues
- **Refactored Command-Line Parsing** — Moved argument parsing to `parser` module for better separation of concerns
- **Improved Code Encapsulation** — Round Robin time quantum now properly isolated within `SchedulerState` struct, eliminating dangling pointer issues
- **State Management** — Cleaner architecture with explicit state handling through structures rather than hidden global variables

---

## Supported Algorithms

1. **FCFS (First Come, First Served):** Non-preemptive. Processes execute strictly in arrival order.
2. **SJF (Shortest Job First):** Non-preemptive. Selects the waiting process with the smallest execution time.
3. **STCF (Shortest Time To Completion First):** Preemptive SJF. Switches to newly arrived jobs if their remaining time is shorter than the current running job.
4. **Round Robin (RR):** Preemptive. Time-sharing scheduler using a configurable time quantum (default is 30).
5. **MLFQ (Multi-Level Feedback Queue):** Adaptive scheduler with 3 default priority queues, dynamic priority adjustments, and starvation prevention via periodic priority boosting.

---

## Compilation and Building

### Prerequisites

- GCC compiler
- Make
- Linux/Unix environment (or WSL/MinGW on Windows)

### Build Commands

Navigate to the root directory of the project and run the following commands:

```bash
# Compile the entire simulator
make all

# Run the automated test suite to verify all algorithms work correctly
make test

# Clean all generated binaries and object files
make clean
```

---

## Usage Instructions

The executable will be located in the `build/` directory.

### Command-Line Arguments

| Flag | Description |
|---|---|
| `--algorithm=<name>` or `-a` | Specify the algorithm: `FCFS`, `SJF`, `STCF`, `RR`, `MLFQ`, or `ALL` |
| `--processes=<str>` or `-p` | Provide inline processes formatted as `PID:Arrival:Burst,...` |
| `--input=<file>` or `-f` / `-i` | Specify a text file containing the workload |
| `--quantum=<int>` or `-q` | Set the time quantum for Round Robin (Default: `30`) |
| `--mlfq-config=<file>` or `-m` | Provide a custom MLFQ configuration file |
| `--compare` or `-c` | Run all algorithms silently and display a comparative performance table |

### Input File Format

Create a `.txt` file with one process per line containing: `ProcessID ArrivalTime BurstTime`

```text
A 0 240
B 10 180
C 20 150
```

---

## Example Commands & Expected Output

### 1. Basic Round Robin Execution

**Command:**

```bash
./build/schedsim --algorithm=RR --quantum=4 --processes="A:0:12,B:2:10,C:4:8,D:6:6"
```

**Expected Output:**

```text
Running Round Robin (RR) Scheduler...

┌─ GANTT CHART ─────────────────────────────────────────────┐
  Total Time: 36 units
  Processes: A B C D A B C A D B 
├────────────────────────────────────────────────────────────┤
  [AAAA][BBBB][CCCC][DDDD][AAAA][BBBB][CCCC][AAAA][DD][BB]
  +-----+-----+-----+-----+-----+-----+-----+-----+---+---
  0     4     8     12    16    20    24    28    32  34  36
└────────────────────────────────────────────────────────────┘

=== Metrics for RR (q=4) ===
Process A:
  Arrival Time:           0
  Burst Time:             12
  Finish Time:            32
  Turnaround Time:  32 - 0 = 32
  Waiting Time:         32 - 12 = 20
  Response Time:     0 - 0 = 0

Process B:
  Arrival Time:           2
  Burst Time:             10
  Finish Time:            36
  Turnaround Time:  36 - 2 = 34
  Waiting Time:         34 - 10 = 24
  Response Time:     4 - 2 = 2

Process C:
  Arrival Time:           4
  Burst Time:             8
  Finish Time:            28
  Turnaround Time:  28 - 4 = 24
  Waiting Time:         24 - 8 = 16
  Response Time:     8 - 4 = 4

Process D:
  Arrival Time:           6
  Burst Time:             6
  Finish Time:            34
  Turnaround Time:  34 - 6 = 28
  Waiting Time:         28 - 6 = 22
  Response Time:     16 - 6 = 10

--- Average Metrics ---
Average Turnaround Time: 29.50
Average Waiting Time:    20.50
Average Response Time:   4.00
```

### 2. Comparison Mode

**Command:**

```bash
./build/schedsim --compare --input=tests/workload1.txt
```

**Expected Output:**

```text
=== Algorithm Comparison for workload1.txt ===

Algorithm  | Avg TT | Avg WT | Avg RT | Context Switches
-----------|--------|--------|--------|------------------
FCFS       |  385.0 |  235.0 |  235.0 |                4
SJF        |  325.0 |  175.0 |  175.0 |                4
STCF       |  305.0 |  155.0 |   80.0 |                7
RR (q=30)  |  345.0 |  195.0 |  125.0 |               25
MLFQ       |  315.0 |  165.0 |   90.0 |               18
```

---

## Known Limitations & Assumptions

- **Maximum Process Limit:** The simulator hardcodes a maximum of `100` processes (`MAX_PROCESSES` in `main.c`). Inputs exceeding this limit will be truncated.
- **Process ID Length:** PIDs are limited to a maximum of `15` characters.
- **Strict CPU-Bound Simulation:** The simulator assumes processes are strictly CPU-bound. It does not explicitly model I/O blocking states or distinct I/O bursts. I/O behavior must be approximated by splitting a single application into multiple discrete "processes" with delayed arrival times.
- **Zero Overhead Context Switching:** The time taken to swap processes (context switch overhead) is assumed to be `0` time units.
- **Non-Preemptive Limitations:** FCFS and SJF cannot be interrupted once scheduled, meaning a massive process arriving at time 0 will lock the CPU and create a convoy effect.
- **Integer Time Units:** All arrival times, burst times, and quanta must be whole integers.

---

## Project Structure

```
.
├── src/
│   ├── main.c              # Entry point and scheduler orchestration
│   ├── parser.c            # Command-line argument parsing and input file loading
│   ├── engine.c            # Core simulation event loop
│   ├── gantt.c             # Gantt chart tracking and visualization
│   ├── metrics.c           # Performance metrics calculation
│   ├── fcfs.c              # First Come, First Served implementation
│   ├── sjf.c               # Shortest Job First implementation
│   ├── stcf.c              # Shortest Time To Completion First implementation
│   ├── rr.c                # Round Robin implementation
│   ├── mlfq.c              # Multi-Level Feedback Queue implementation
│   ├── process.c           # Process structure and operations
│   ├── events.c            # Event queue management
│   ├── scheduler_registry.c # Algorithm registration and lookup
│   └── utils.c             # Utility functions
├── include/                # Header files
│   ├── scheduler.h         # Core data structures (SchedulerState, Process, etc.)
│   ├── parser.h            # Command-line parsing interface
│   ├── gantt.h             # Gantt chart interface
│   └── ...                 # Other module headers
├── tests/                  # Example workloads and test scripts
│   ├── workload*.txt       # Pre-configured process workloads
│   ├── test_all_algorithms.sh
│   └── run_workload_tests.sh
├── docs/                   # Design documentation
│   └── mlfq_design.md      # MLFQ architecture details
└── Makefile                # Build configuration
```

---

## MLFQ Design Details

> See `docs/` for the full MLFQ architecture documentation.

---

## Code Architecture & Design Decisions

### 1. Command-Line Argument Parsing

The `parser` module (`parser.h` / `parser.c`) centralizes all command-line handling:
- `CommandLineArgs` struct encapsulates all CLI options
- `parse_command_line_args()` handles getopt processing
- Separates parsing logic from main execution flow

### 2. State Management

- **SchedulerState** struct contains all scheduler state including:
  - `time_quantum` for Round Robin (isolated value, not pointer)
  - `algo_data` for algorithm-specific configurations
  - Gantt chart tracking via `chart` member
  - Ready queue and event queue management

### 3. Gantt Chart Design

- **GanttChart** struct tracks `dispatch_count` directly (no hidden static globals)
- Segments are linked-list based for flexible process tracking
- Dynamic scaling based on total_time to maintain readability
- Clean output format eliminated side pipes to prevent overflow on long workloads

### 4. Round Robin Quantum Isolation

Recent refactoring fixed a design issue:
- **Before**: Stored address of stack variable (`&time_quantum`)
- **After**: Copies value directly to `SchedulerState.time_quantum`
- Ensures safe access throughout simulation lifecycle

---

## MLFQ Design Details

> See `docs/` for the full MLFQ architecture documentation.

## License

Educational project for **CMSC 125 – Operating Systems**.
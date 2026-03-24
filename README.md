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
| **Performance Metrics** | Turnaround time, response time, wait time, CPU utilization, and context switch tracking |
| **Gantt Chart Visualization** | Clear visual representation of process execution timelines scaled dynamically |
| **Comparison Mode** | Run all algorithms on the same workload simultaneously to output a comparative table |
| **Custom Input** | Define processes via inline command-line arguments or dedicated input files |
| **Configurable MLFQ** | Customize queues, time quanta, allotments, and boost periods via config files |
| **Automated Testing** | Built-in bash scripts for regression testing across algorithm implementations |

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
./build/schedsim --algorithm=RR --quantum=50 --processes="P1:0:100,P2:10:40"
```

**Expected Output:**

```text
Running Round Robin (RR) Scheduler...
=== Gantt Chart ===
Scale: Each character represents ~3 time units
[----------------][-----][----------------][-----]
0                50    90               140   150

=== Metrics for RR (q=50) ===
Process P1:
  Arrival Time:           0
  Burst Time:             100
  Finish Time:            150
  Turnaround Time:  150 - 0 = 150
  Waiting Time:         150 - 100 = 50
  Response Time:     0 - 0 = 0

Process P2:
  Arrival Time:           10
  Burst Time:             40
  Finish Time:            90
  Turnaround Time:  90 - 10 = 80
  Waiting Time:         80 - 40 = 40
  Response Time:     50 - 10 = 40

--- Average Metrics ---
Average Turnaround Time: 115.00
Average Waiting Time:    45.00
Average Response Time:   20.00
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
├── src/          # Core simulation engine, algorithm implementations, and metrics calculations
├── include/      # Header files mapping the data structures (Process, SchedulerState, Event)
├── tests/        # Example workloads and bash scripts for automated validation
└── docs/         # Expanded design documentation (e.g., MLFQ architecture)
```

---

## MLFQ Design Details

> See `docs/` for the full MLFQ architecture documentation.

## License

Educational project for **CMSC 125 – Operating Systems**.
# MLFQ Scheduler Design Justification

## Overview

This implementation simulates the Multi Level Feedback Queue scheduling algorithm within the project’s modular scheduling framework. The design focuses on realistic operating system behavior, fairness across workloads, and compatibility with the existing simulation structure that includes process management, metrics calculation, and Gantt chart visualization.

The scheduler dynamically adjusts process priority based on CPU usage while preventing starvation through periodic priority boosting.

## Design Objectives

The design aims to achieve the following:

* Support dynamic process arrival during execution
* Distinguish between short interactive jobs and long CPU bound jobs
* Prevent starvation of lower priority processes
* Maintain compatibility with the project's scheduler interface
* Provide visual execution analysis through Gantt charts

## Queue Based Architecture

The scheduler uses a circular queue structure to represent each priority level.

```
typedef struct {
    Process** procs;
    int front;
    int rear;
    int size;
    int capacity;
} MLFQ_Queue;
```

This structure was selected for several reasons.

* FIFO behavior is required within each priority level
* Circular queues allow constant time enqueue and dequeue operations
* Storing pointers avoids copying the Process structure
* Memory usage is predictable because capacity equals the number of processes

This ensures efficient process scheduling even as the workload increases.

## Multiple Priority Levels

The scheduler dynamically allocates multiple queues based on the configuration.

```
MLFQ_Queue* queues = malloc(config->num_queues * sizeof(MLFQ_Queue));
```

Each queue represents a different priority level.

Queue 0 has the highest priority.
Lower queues represent progressively lower priority levels.

This structure enables the scheduler to treat interactive tasks differently from CPU intensive tasks.

### Number of Queues

The implementation uses **three priority queues**. This choice balances responsiveness and scheduling complexity.

* Queue 0 handles newly arrived and interactive processes
* Queue 1 handles processes that require moderate CPU time
* Queue 2 handles long running CPU bound processes

Using three levels allows the scheduler to separate short jobs from long jobs without creating unnecessary overhead. Increasing the number of queues can provide finer control but also increases management complexity and scheduling overhead.

## Configurable Scheduling Policy

Scheduling parameters are defined through the configuration structure.

```
MLFQ_Config mlfq_config = {
    .num_queues = 3,
    .time_quantums = {2,4,8},
    .boost_interval = 20
};
```

This approach provides flexibility.

* The number of priority levels can be modified
* Time quantum values can be adjusted per queue
* The boost interval can be tuned for fairness

Because these parameters are configurable, the scheduler can simulate different operating system policies.

### Time Quantum per Level

The selected quantum sizes follow an increasing pattern.

* Queue 0 uses a quantum of **2**
* Queue 1 uses a quantum of **4**
* Queue 2 uses a quantum of **8**

Short time slices at higher priorities allow quick response for interactive processes. Larger time slices at lower priorities reduce context switching for CPU bound processes.

Doubling the quantum between levels is a common scheduling strategy because it gradually increases CPU access while maintaining responsiveness.

### Maximum Time per Level

Each queue effectively limits how long a process can remain at that priority level.

* If a process uses its entire quantum, it is demoted to the next queue
* Processes in lower queues receive larger execution windows

This approach ensures that processes which frequently use the CPU move to lower priorities, while shorter tasks finish quickly at higher levels.

### Allotment Values

Allotment refers to the total amount of CPU time a process may consume in a queue before demotion.

In this design, the allotment equals the queue's time quantum.

* Queue 0 allotment: 2 time units
* Queue 1 allotment: 4 time units
* Queue 2 allotment: 8 time units

Using equal allotment and quantum values simplifies the implementation and ensures predictable demotion behavior.

## Arrival Time Ordering

Before scheduling begins, processes are sorted by arrival time.

```
qsort(processes, num_processes, sizeof(Process), compare_arrival_time);
```

Sorting simplifies arrival handling.

* Processes enter the system in chronological order
* The scheduler can track the next arriving process efficiently
* Repeated searches across the process list are avoided

This improves simulation efficiency.

## Dynamic Process Arrival

The scheduler continuously checks whether new processes have arrived.

```
while (idx < num_processes && processes[idx].arrival_time <= current_time)
```

New processes are inserted into the highest priority queue.

Reasons for this design:

* Interactive processes receive immediate CPU access
* Newly arrived jobs are treated as high priority tasks
* The scheduler accurately reflects real operating system behavior

## Priority Based Selection

The scheduler always selects the first available process from the highest priority queue.

```
for (int i = 0; i < config->num_queues; i++)
```

This ensures that:

* High priority processes always execute first
* Lower priority jobs do not delay interactive tasks

This rule is fundamental to Multi Level Feedback Queue scheduling.

## Adaptive Time Quantum

Execution time depends on the queue level.

```
int time_allotment = config->time_quantums[selected_q];
```

Higher priority queues use smaller time slices.
Lower priority queues use larger time slices.

Benefits of this design:

* Interactive jobs respond quickly
* Long running jobs execute more efficiently

This balances responsiveness and throughput.

## Priority Demotion

When a process uses its entire time quantum, it moves to a lower priority queue.

```
if (selected_q < config->num_queues - 1)
    p->queue_level = selected_q + 1;
```

This mechanism allows the scheduler to distinguish between different workload types.

Short jobs typically finish in high priority queues.
CPU bound processes gradually move to lower priority levels.

## Starvation Prevention

Priority boosting is implemented to prevent starvation.

```
if (current_time - last_boost_time >= config->boost_interval)
```

### Boost Period

The scheduler performs a priority boost every **20 time units**.

During a boost:

* All processes return to the highest priority queue
* Lower priority queues are emptied

This interval was selected to ensure that CPU bound processes do not remain indefinitely in low priority queues while still allowing interactive processes to retain scheduling advantages.

## Event Driven Execution

Execution segments are limited by three possible events.

* process arrival
* time quantum expiration
* priority boost

The scheduler adjusts execution time based on whichever event occurs first.

This event driven approach ensures accurate simulation of scheduling decisions.

## Gantt Chart Integration

Each execution segment is recorded in the Gantt chart.

```
add_gantt_segment(&chart, p->pid, start, current_time);
```

The Gantt chart provides:

* a visual representation of scheduling decisions
* an execution timeline for analysis
* integration with the project’s performance metrics

This supports comparison between different scheduling algorithms.

## Empirical Testing

Several test workloads were used to evaluate the effectiveness of the scheduling configuration.

Example workload:

```
# Format: PID ArrivalTime BurstTime
A 0 240
B 10 180
C 20 150
D 25 80
E 30 130
```

Observed behavior:

* Shorter jobs finished earlier due to higher initial priority
* CPU bound jobs gradually moved to lower queues
* Periodic boosting allowed long processes to regain CPU access
* The system avoided starvation even with long running processes

The results showed balanced CPU distribution between interactive and CPU intensive workloads, supporting the chosen configuration.

## Memory Management

All dynamically allocated resources are released after the simulation.

Queues are freed individually and the Gantt chart memory is released.

This prevents memory leaks during repeated scheduler executions.

## Conclusion

The MLFQ scheduler design reflects realistic operating system behavior while remaining modular and configurable. The queue based structure, dynamic priority adjustments, and starvation prevention mechanisms allow the scheduler to effectively balance responsiveness and fairness across different workloads.

The implementation integrates smoothly with the project framework and supports detailed analysis through metrics and Gantt chart visualization.

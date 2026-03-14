# MLFQ Scheduler Design Justification

## Overview

This implementation simulates the Multi Level Feedback Queue scheduling algorithm within the project’s modular scheduling framework. The design focuses on realistic operating system behavior, fairness across workloads, and compatibility with the existing simulation structure that includes process management, metrics calculation, and Gantt chart visualization.

The scheduler dynamically adjusts process priority based on CPU usage while preventing starvation through periodic priority boosting.

## Design Objectives

The design aims to achieve the following:

- Support dynamic process arrival during execution \n
- Distinguish between short interactive jobs and long CPU bound jobs \n
- Prevent starvation of lower priority processes \n
- Maintain compatibility with the project's scheduler interface \n
- Provide visual execution analysis through Gantt charts \n

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

- FIFO behavior is required within each priority level
- Circular queues allow constant time enqueue and dequeue operations
- Storing pointers avoids copying the Process structure
- Memory usage is predictable because capacity equals the number of processes

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

- The number of priority levels can be modified
- Time quantum values can be adjusted per queue
- The boost interval can be tuned for fairness

Because these parameters are configurable, the scheduler can simulate different operating system policies.

## Arrival Time Ordering

Before scheduling begins, processes are sorted by arrival time.

```
qsort(processes, num_processes, sizeof(Process), compare_arrival_time);
```

Sorting simplifies arrival handling.

- Processes enter the system in chronological order
- The scheduler can track the next arriving process efficiently
- Repeated searches across the process list are avoided

This improves simulation efficiency.

## Dynamic Process Arrival

The scheduler continuously checks whether new processes have arrived.

```
while (idx < num_processes && processes[idx].arrival_time <= current_time)
```

New processes are inserted into the highest priority queue.

Reasons for this design:

- Interactive processes receive immediate CPU access
- Newly arrived jobs are treated as high priority tasks
- The scheduler accurately reflects real operating system behavior

## Priority Based Selection

The scheduler always selects the first available process from the highest priority queue.

```
for (int i = 0; i < config->num_queues; i++)
```

This ensures that:

- High priority processes always execute first
- Lower priority jobs do not delay interactive tasks

This rule is fundamental to Multi Level Feedback Queue scheduling.

## Adaptive Time Quantum

Execution time depends on the queue level.

```
int time_allotment = config->time_quantums[selected_q];
```

Higher priority queues use smaller time slices.
Lower priority queues use larger time slices.

Benefits of this design:

- Interactive jobs respond quickly
- Long running jobs execute more efficiently

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

During a boost:

- All processes return to the highest priority queue
- Lower priority queues are emptied

This guarantees that long running processes eventually regain CPU access.

## Event Driven Execution

Execution segments are limited by three possible events.

- process arrival
- time quantum expiration
- priority boost

The scheduler adjusts execution time based on whichever event occurs first.

This event driven approach ensures accurate simulation of scheduling decisions.

## Gantt Chart Integration

Each execution segment is recorded in the Gantt chart.

```
add_gantt_segment(&chart, p->pid, start, current_time);
```

The Gantt chart provides:

- a visual representation of scheduling decisions
- an execution timeline for analysis
- integration with the project’s performance metrics

This supports comparison between different scheduling algorithms.

## Memory Management

All dynamically allocated resources are released after the simulation.

Queues are freed individually and the Gantt chart memory is released.

This prevents memory leaks during repeated scheduler executions.

## Conclusion

The MLFQ scheduler design reflects realistic operating system behavior while remaining modular and configurable. The queue based structure, dynamic priority adjustments, and starvation prevention mechanisms allow the scheduler to effectively balance responsiveness and fairness across different workloads.

The implementation integrates smoothly with the project framework and supports detailed analysis through metrics and Gantt chart visualization.

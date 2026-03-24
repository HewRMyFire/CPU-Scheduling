# Multi-Level Feedback Queue (MLFQ) Design Documentation

This document outlines the architectural decisions, parameter configurations, and empirical validations for the custom Multi-Level Feedback Queue (MLFQ) implemented in the scheduling simulator.

## 1. Detailed Justification of MLFQ Design

The primary goal of this MLFQ implementation is to optimize for both **turnaround time** (for CPU-bound batch jobs) and **response time** (for interactive jobs) without requiring prior knowledge of process execution lengths. 

The design relies on a dynamic priority system that observes process behavior. It assumes that processes heavily utilizing their time slices are CPU-bound and should be demoted to lower priorities, reducing context-switch overhead. Conversely, processes that relinquish the CPU quickly (or complete short bursts) are kept at higher priorities to maintain system responsiveness. A periodic priority boost is integrated to act as a fail-safe against starvation, ensuring that long-running jobs at the bottom of the queue structure periodically regain access to the CPU.

## 2. Parameter Choices

The scheduler is configured with specific defaults tuned for a balanced workload:

* **Number of Queues (3 Levels):** * **Queue 0 (High Priority):** Catches all new arrivals. Optimized for quick, interactive tasks.
    * **Queue 1 (Medium Priority):** A transitional queue for moderate tasks.
    * **Queue 2 (Low Priority):** The sink for massive, CPU-bound tasks.
    * *Justification:* A 3-level structure provides enough granularity to separate short, medium, and long jobs without introducing unnecessary management overhead or excessive queue-checking latency.
* **Time Quantums (`Q0 = 10`, `Q1 = 30`, `Q2 = Run-to-Completion/Preempt`):**
    * *Justification:* An increasing time quantum strategy is used. Q0 has a short slice (10) to ensure the system cycles rapidly through interactive tasks. Q1 provides a larger slice (30) to give medium jobs a chance to finish efficiently. Q2 has no strict quantum, allowing heavy tasks to run uninterrupted until a higher-priority job arrives or a boost occurs.
* **Allotments (`Q0 = 50`, `Q1 = 150`):**
    * *Justification:* Instead of demoting a process immediately after a single time slice, this design grants an "allotment" of total time at a specific priority level. A process in Q0 can run for 5 separate 10-unit quanta (totaling 50 units) before being demoted. This forgives bursty processes that might occasionally use a full time slice.
* **Boost Period (`S = 200`):**
    * *Justification:* Every 200 time units, all processes in the system are moved back to Queue 0. This interval was chosen to be significantly larger than the top-level allotments so that the MLFQ has time to filter jobs, but frequent enough to prevent complete starvation of Queue 2 processes during heavy incoming workloads.

## 3. Empirical Testing Results

The design was validated against several workloads, including staggered, heavy, and bimodal distributions (e.g., `workload_bimodal.txt` and `workload_mixed.txt`).

* **Responsiveness:** In bimodal workloads (many short jobs, few massive jobs), short jobs consistently achieved response times near 0, as they were immediately scheduled in Queue 0 and completed before exhausting their allotment.
* **Throughput and Efficiency:** For heavy jobs (e.g., 200+ burst times), the system successfully filtered them down to Queue 2. Once in Queue 2, they executed with zero context-switch overhead (aside from new arrivals), maximizing CPU utilization.
* **Starvation Prevention:** In stress tests where a constant stream of short jobs was introduced, Queue 2 jobs successfully stalled until the 200-unit mark. At this point, the boost mechanism triggered, bringing the heavy jobs to Queue 0 and guaranteeing forward progress.

## 4. Comparison with Standard 3-Level MLFQ

A textbook standard MLFQ often tightly couples the time quantum to the priority demotion (e.g., 1 Quantum = 1 Demotion). 

* **Strict Standard MLFQ:** If Q0 has a quantum of 10, a process is demoted to Q1 the moment it consumes 10 continuous units. This punishes interactive processes that occasionally have a slightly longer computation phase.
* **Our Allotment-Based MLFQ:** By divorcing the quantum from the demotion threshold (e.g., Quantum = 10, Allotment = 50), our design is much more forgiving. It allows a process to utilize the CPU for multiple short bursts (or be preempted) without losing its top-tier priority status, providing a more accurate assessment of its long-term behavior rather than punishing short-term spikes.

## 5. Discussion of Tradeoffs

While the current MLFQ design is highly adaptable, it introduces several fundamental tradeoffs:

1.  **Configuring the Boost Interval (`S`):**
    * *Tradeoff:* If the boost period is too short (e.g., 50), the system behaves almost exactly like a standard Round Robin scheduler, completely negating the efficiency benefits of the lower queues. If it is too long (e.g., 1000), Queue 2 jobs suffer severe starvation during high-traffic periods. The value of 200 is a compromise that relies heavily on the assumption of a balanced workload.
2.  **Generous Allotments vs. System Hogging:**
    * *Tradeoff:* The generous allotment in Q0 (50 units) protects bursty interactive processes, but it allows malicious or purely CPU-bound processes to "hog" the highest priority queue for a significant amount of time before the system finally recognizes them as heavy jobs and demotes them. 
3.  **Algorithmic Complexity:**
    * *Tradeoff:* Tracking exact time accumulated in queues (for allotment) across context switches and priority boosts requires significantly more state management (`time_in_queue`, `priority`, `quantum_used`) compared to simpler algorithms like STCF or strict RR, increasing the overhead of the dispatcher itself.
### **Number of Queues: 3 Levels**
A three-tier queue structure was chosen to strike an optimal balance between responsiveness and scheduling complexity. 
* **Queue 0 (Highest Priority):** Dedicated to newly arrived and highly interactive processes.
* **Queue 1 (Medium Priority):** Catches processes that require moderate CPU time.
* **Queue 2 (Lowest Priority):** Acts as the sink for long-running, CPU-bound tasks.

Increasing the number of queues beyond three would introduce unnecessary management complexity and context-switching overhead without providing a proportional benefit in process separation. 

### **Time Quantum per Level**
The scheduler uses an increasing time quantum pattern across the descending queues (e.g., doubling patterns like 2, 4, 8 in the design, or 10, 30 in the default executable).
* **High-Priority Responsiveness:** Short time slices at the top level ensure that interactive processes respond almost immediately.
* **Low-Priority Efficiency:** Larger time slices at the lowest levels drastically reduce context-switching overhead for massive CPU-bound processes, allowing them to process efficiently once interactive jobs are out of the way.

### **Allotment Values & Maximum Time per Level**
The allotment values dictate exactly how much total CPU time a process can consume at a specific priority level before it is aggressively demoted.
* **Design Simplicity:** The core design philosophy sets the allotment equal to the queue's time quantum. For instance, if Queue 0 has a quantum of 2, its allotment is also 2 time units. 
* **Predictable Demotion:** This 1:1 ratio ensures predictable behavior: if a process burns through its entire time slice without completing, it is immediately recognized as heavier than expected and bumped down to the next tier. *(Note: Your main executable allows configuring higher allotments, such as 50 and 150, allowing processes to cycle a few times before demotion)*

### **Boost Period**
Priority boosting is the hard counter to process starvation.
* **The Interval:** The design specifies periodic boosts (e.g., every 20 or 200 time units). 
* **The Rationale:** Without boosts, a continuous stream of short jobs in Queue 0 would completely lock out processes stuck in Queue 2. The chosen interval is tuned to ensure that long-running jobs are eventually dragged back up to Queue 0 to regain CPU access, striking a compromise between keeping interactive tasks fast and giving heavy tasks a fighting chance.

### **Empirical Testing Results**
The architectural choices were validated against a staggered, heavy workload (e.g., processes with bursts of 240, 180, 150, 80, and 130 arriving sequentially).
* **Observation 1:** Shorter jobs successfully finished earlier because of their placement in the highest priority queue upon arrival.
* **Observation 2:** The heavy, CPU-bound jobs naturally filtered down into the lower queues as designed, clearing the way for new arrivals.
* **Observation 3:** The priority boost mechanism triggered successfully, proving that starvation was avoided even under severe load conditions. 
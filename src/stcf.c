#include <stdio.h>
#include <stdlib.h>

#define MAX 100

typedef struct {
    char pid;
    int arrival;
    int burst;
    int remaining;
    int completion;
} Process;

int main() {
    Process p[MAX];
    int n = 0;

    printf("Enter processes (PID Arrival Burst). Press Ctrl+D to stop.\n");

    while (scanf(" %c %d %d", &p[n].pid, &p[n].arrival, &p[n].burst) == 3) {
        p[n].remaining = p[n].burst;
        n++;
    }

    int completed = 0;
    int time = 0;
    int shortest = -1;

    while (completed < n) {
        shortest = -1;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && p[i].remaining > 0) {
                if (shortest == -1 || p[i].remaining < p[shortest].remaining) {
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            time++;
            continue;
        }

        p[shortest].remaining--;
        time++;

        if (p[shortest].remaining == 0) {
            p[shortest].completion = time;
            completed++;
        }
    }

    printf("\nPID\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");

    for (int i = 0; i < n; i++) {
        int turnaround = p[i].completion - p[i].arrival;
        int waiting = turnaround - p[i].burst;

        printf("%c\t%d\t%d\t%d\t\t%d\t\t%d\n",
                p[i].pid,
                p[i].arrival,
                p[i].burst,
                p[i].completion,
                turnaround,
                waiting);
    }

    return 0;
}
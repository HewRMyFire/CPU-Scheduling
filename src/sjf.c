#include <stdio.h>

#define MAX 100

typedef struct {
    char pid;
    int arrival;
    int burst;
    int completion;
    int finished;
} Process;

int main() {
    Process p[MAX];
    int n = 0;

    printf("Enter processes (PID Arrival Burst). Press Ctrl+D to stop.\n");

    while (scanf(" %c %d %d", &p[n].pid, &p[n].arrival, &p[n].burst) == 3) {
        p[n].finished = 0;
        n++;
    }

    int completed = 0;
    int time = 0;

    while (completed < n) {
        int shortest = -1;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && p[i].finished == 0) {
                if (shortest == -1 || p[i].burst < p[shortest].burst) {
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            time++;
            continue;
        }

        time += p[shortest].burst;
        p[shortest].completion = time;
        p[shortest].finished = 1;
        completed++;
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include "process.h"
#include "util.h"

// Comparator function for Priority Scheduling (highest priority first)
int my_comparer(const void *this, const void *that) {
    const ProcessType *p1 = (const ProcessType *)this;
    const ProcessType *p2 = (const ProcessType *)that;
    return p2->pri - p1->pri;
}

// Comparator function for SJF (shortest burst time first)
int sjf_comparer(const void *this, const void *that) {
    const ProcessType *p1 = (const ProcessType *)this;
    const ProcessType *p2 = (const ProcessType *)that;
    return p1->bt - p2->bt;
}

// Function to find waiting time for all processes (FCFS with arrival time)
void findWaitingTimeFCFS(ProcessType plist[], int n) {
    int service_time[n];
    
    service_time[0] = plist[0].art;
    plist[0].wt = 0;
    
    for (int i = 1; i < n; i++) {
        service_time[i] = service_time[i-1] + plist[i-1].bt;
        
        if (service_time[i] < plist[i].art) {
            service_time[i] = plist[i].art;
        }
        
        plist[i].wt = service_time[i] - plist[i].art;
        
        if (plist[i].wt < 0) {
            plist[i].wt = 0;
        }
    }
}

// Function to find turnaround time for all processes
void findTurnAroundTime(ProcessType plist[], int n) {
    for (int i = 0; i < n; i++) {
        plist[i].tat = plist[i].bt + plist[i].wt;
    }
}

// IMPROVED: Function to find waiting time for SJF (SRTF - Preemptive)
// Now skips idle time instead of incrementing by 1
void findWaitingTimeSJF(ProcessType plist[], int n) {
    int *rem_bt = (int *)malloc(n * sizeof(int));
    int *completed = (int *)calloc(n, sizeof(int)); // Track completed processes
    int complete = 0, t = 0;
    
    // Copy burst times to remaining burst time array
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }
    
    while (complete != n) {
        int shortest = -1;
        int minm = INT_MAX;
        
        // Find process with minimum remaining time among arrived processes
        for (int j = 0; j < n; j++) {
            if (plist[j].art <= t && rem_bt[j] > 0 && rem_bt[j] < minm) {
                minm = rem_bt[j];
                shortest = j;
            }
        }
        
        // IMPROVEMENT: If no process is ready, jump to next arrival time
        if (shortest == -1) {
            int next_arrival = INT_MAX;
            for (int j = 0; j < n; j++) {
                if (rem_bt[j] > 0 && plist[j].art < next_arrival) {
                    next_arrival = plist[j].art;
                }
            }
            t = next_arrival;
            continue;
        }
        
        // Execute for 1 time unit
        rem_bt[shortest]--;
        t++;
        
        // If process is completely executed
        if (rem_bt[shortest] == 0) {
            complete++;
            int finish_time = t;
            
            // Waiting time = finish time - burst time - arrival time
            plist[shortest].wt = finish_time - plist[shortest].bt - plist[shortest].art;
            
            if (plist[shortest].wt < 0)
                plist[shortest].wt = 0;
        }
    }
    
    free(rem_bt);
    free(completed);
}

// IMPROVED: Function to find waiting time for Round Robin
// Now properly handles arrival times
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) {
    int *rem_bt = (int *)malloc(n * sizeof(int));
    int *finish_time = (int *)malloc(n * sizeof(int));
    int *in_queue = (int *)calloc(n, sizeof(int));  // Track if process is in ready queue
    int *queue = (int *)malloc(n * sizeof(int));    // Ready queue (circular)
    int front = 0, rear = 0, queue_size = 0;
    
    // Copy burst times
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        finish_time[i] = 0;
    }
    
    // Sort indices by arrival time for initial queue loading
    int *sorted_idx = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) sorted_idx[i] = i;
    
    // Simple bubble sort by arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (plist[sorted_idx[j]].art > plist[sorted_idx[j+1]].art) {
                int temp = sorted_idx[j];
                sorted_idx[j] = sorted_idx[j+1];
                sorted_idx[j+1] = temp;
            }
        }
    }
    
    int t = 0;
    int completed = 0;
    int next_arrival_idx = 0;  // Index into sorted_idx for next process to arrive
    
    // Add all processes that arrive at time 0
    while (next_arrival_idx < n && plist[sorted_idx[next_arrival_idx]].art <= t) {
        int idx = sorted_idx[next_arrival_idx];
        queue[rear] = idx;
        rear = (rear + 1) % n;
        queue_size++;
        in_queue[idx] = 1;
        next_arrival_idx++;
    }
    
    while (completed < n) {
        // If queue is empty, jump to next arrival
        if (queue_size == 0) {
            if (next_arrival_idx < n) {
                t = plist[sorted_idx[next_arrival_idx]].art;
                // Add all processes arriving at this time
                while (next_arrival_idx < n && plist[sorted_idx[next_arrival_idx]].art <= t) {
                    int idx = sorted_idx[next_arrival_idx];
                    queue[rear] = idx;
                    rear = (rear + 1) % n;
                    queue_size++;
                    in_queue[idx] = 1;
                    next_arrival_idx++;
                }
            }
            continue;
        }
        
        // Get next process from queue
        int curr = queue[front];
        front = (front + 1) % n;
        queue_size--;
        
        // Execute for quantum or remaining time, whichever is smaller
        int exec_time = (rem_bt[curr] > quantum) ? quantum : rem_bt[curr];
        t += exec_time;
        rem_bt[curr] -= exec_time;
        
        // Add newly arrived processes to queue (arrived during this quantum)
        while (next_arrival_idx < n && plist[sorted_idx[next_arrival_idx]].art <= t) {
            int idx = sorted_idx[next_arrival_idx];
            if (!in_queue[idx] && rem_bt[idx] > 0) {
                queue[rear] = idx;
                rear = (rear + 1) % n;
                queue_size++;
                in_queue[idx] = 1;
            }
            next_arrival_idx++;
        }
        
        // Check if current process is done
        if (rem_bt[curr] == 0) {
            completed++;
            finish_time[curr] = t;
            // Waiting time = finish time - burst time - arrival time
            plist[curr].wt = finish_time[curr] - plist[curr].bt - plist[curr].art;
            if (plist[curr].wt < 0) plist[curr].wt = 0;
        } else {
            // Put back in queue
            queue[rear] = curr;
            rear = (rear + 1) % n;
            queue_size++;
        }
    }
    
    free(rem_bt);
    free(finish_time);
    free(in_queue);
    free(queue);
    free(sorted_idx);
}

// Function to calculate average time for FCFS
void findavgTimeFCFS(ProcessType plist[], int n) {
    findWaitingTimeFCFS(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n*********\nFCFS\n");
}

// Function to calculate average time for Priority Scheduling
void findavgTimePriority(ProcessType plist[], int n) {
    qsort(plist, n, sizeof(ProcessType), my_comparer);
    findWaitingTimeFCFS(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n*********\nPriority\n");
}

// Function to calculate average time for SJF
void findavgTimeSJF(ProcessType plist[], int n) {
    findWaitingTimeSJF(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n*********\nSJF\n");
}

// Function to calculate average time for Round Robin
void findavgTimeRR(ProcessType plist[], int n, int quantum) {
    findWaitingTimeRR(plist, n, quantum);
    findTurnAroundTime(plist, n);
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

// Function to print metrics
void printMetrics(ProcessType plist[], int n) {
    int total_wt = 0, total_tat = 0;
    float awt, att;
    
    printf("\tProcesses\tBurst time\tWaiting time\tTurn around time\n");
    
    for (int i = 0; i < n; i++) {
        total_wt += plist[i].wt;
        total_tat += plist[i].tat;
        printf("\t%d\t\t%d\t\t%d\t\t%d\n", plist[i].pid, plist[i].bt, plist[i].wt, plist[i].tat);
    }
    
    awt = ((float)total_wt / (float)n);
    att = ((float)total_tat / (float)n);
    
    printf("\nAverage waiting time = %.2f", awt);
    printf("\nAverage turn around time = %.2f\n", att);
}

int main(int argc, char *argv[]) {
    int n = 0;
    int quantum = 2;
    ProcessType *plist = NULL;
    FILE *input_file = NULL;
    
    if (argc == 2) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            printf("Error: Could not open file %s\n", argv[1]);
            return 1;
        }
        plist = parse_file(input_file, &n);
        fclose(input_file);
    } else {
        input_file = stdin;
        plist = parse_file(input_file, &n);
    }
    
    if (plist == NULL || n == 0) {
        printf("Error: No processes to schedule\n");
        return 1;
    }
    
    // Create copies for each algorithm
    ProcessType *plist_fcfs = (ProcessType *)malloc(n * sizeof(ProcessType));
    ProcessType *plist_priority = (ProcessType *)malloc(n * sizeof(ProcessType));
    ProcessType *plist_sjf = (ProcessType *)malloc(n * sizeof(ProcessType));
    ProcessType *plist_rr = (ProcessType *)malloc(n * sizeof(ProcessType));
    
    for (int i = 0; i < n; i++) {
        plist_fcfs[i] = plist[i];
        plist_priority[i] = plist[i];
        plist_sjf[i] = plist[i];
        plist_rr[i] = plist[i];
    }
    
    findavgTimeFCFS(plist_fcfs, n);
    printMetrics(plist_fcfs, n);
    
    findavgTimePriority(plist_priority, n);
    printMetrics(plist_priority, n);
    
    findavgTimeSJF(plist_sjf, n);
    printMetrics(plist_sjf, n);
    
    findavgTimeRR(plist_rr, n, quantum);
    printMetrics(plist_rr, n);
    
    free(plist);
    free(plist_fcfs);
    free(plist_priority);
    free(plist_sjf);
    free(plist_rr);
    
    return 0;
}
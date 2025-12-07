#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>   // For INT_MAX
#include <stdbool.h>  // For bool, true, false
#include "process.h"  // For ProcessType struct
#include "util.h"     // For utility functions

// Comparator function for Priority Scheduling (highest priority first)
int my_comparer(const void *this, const void *that) {
    const ProcessType *p1 = (const ProcessType *)this;
    const ProcessType *p2 = (const ProcessType *)that;
    
    // Higher priority number = higher priority (reverse of typical comparison)
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
    int service_time[n]; // Service time = time when process starts execution
    
    // Service time for first process is its arrival time
    service_time[0] = plist[0].art;
    plist[0].wt = 0; // First process has 0 waiting time
    
    // Calculate service time and waiting time for each process
    for (int i = 1; i < n; i++) {
        // Service time = max(previous completion time, current arrival time)
        // Previous completion time = service_time[i-1] + burst_time[i-1]
        service_time[i] = service_time[i-1] + plist[i-1].bt;
        
        // If current process arrives after previous process completes
        if (service_time[i] < plist[i].art) {
            service_time[i] = plist[i].art; // CPU waits (idle) until process arrives
        }
        
        // Waiting time = service time - arrival time
        plist[i].wt = service_time[i] - plist[i].art;
        
        // Waiting time should not be negative
        if (plist[i].wt < 0) {
            plist[i].wt = 0;
        }
    }
}

// Function to find turnaround time for all processes
void findTurnAroundTime(ProcessType plist[], int n) {
    // Turnaround time = burst time + waiting time
    for (int i = 0; i < n; i++) {
        plist[i].tat = plist[i].bt + plist[i].wt;
    }
}

// Note: If util.c provides printMetrics, remove the implementation below
// and use the one from util.h instead

// Function to find waiting time for SJF (Shortest Remaining Time First - Preemptive)
void findWaitingTimeSJF(ProcessType plist[], int n) {
    int *rem_bt = (int *)malloc(n * sizeof(int));
    int complete = 0, t = 0, minm = INT_MAX;
    int shortest = 0;
    bool check = false;
    
    // Copy burst times to remaining burst time array
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }
    
    // Process until all processes are complete
    while (complete != n) {
        // Find process with minimum remaining time among arrived processes
        minm = INT_MAX;
        check = false;
        
        for (int j = 0; j < n; j++) {
            if ((plist[j].art <= t) && (rem_bt[j] < minm) && rem_bt[j] > 0) {
                minm = rem_bt[j];
                shortest = j;
                check = true;
            }
        }
        
        if (!check) {
            t++;
            continue;
        }
        
        // Reduce remaining time by one
        rem_bt[shortest]--;
        
        // Update minimum
        minm = rem_bt[shortest];
        if (minm == 0)
            minm = INT_MAX;
        
        // If a process gets completely executed
        if (rem_bt[shortest] == 0) {
            complete++;
            int finish_time = t + 1;
            
            // Calculate waiting time
            plist[shortest].wt = finish_time - plist[shortest].bt - plist[shortest].art;
            
            if (plist[shortest].wt < 0)
                plist[shortest].wt = 0;
        }
        
        // Increment time
        t++;
    }
    
    free(rem_bt);
}

// Function to find waiting time for Round Robin
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) {
    int *rem_bt = (int *)malloc(n * sizeof(int));
    
    // Copy burst times to remaining burst time array
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }
    
    int t = 0; // Current time
    
    // Keep traversing processes in round robin manner until all are done
    while (true) {
        bool done = true;
        
        // Traverse all processes one by one repeatedly
        for (int i = 0; i < n; i++) {
            // If burst time of a process is greater than 0 then only need to process further
            if (rem_bt[i] > 0) {
                done = false; // There is a pending process
                
                if (rem_bt[i] > quantum) {
                    // Increase the value of t i.e. shows how much time a process has been processed
                    t += quantum;
                    
                    // Decrease the burst_time of current process by quantum
                    rem_bt[i] -= quantum;
                } else {
                    // Last cycle for this process
                    t += rem_bt[i];
                    
                    // Waiting time is current time minus time used by this process
                    plist[i].wt = t - plist[i].bt;
                    
                    // As the process gets fully executed make its remaining burst time = 0
                    rem_bt[i] = 0;
                }
            }
        }
        
        // If all processes are done
        if (done)
            break;
    }
    
    free(rem_bt);
}

// Function to calculate average time
void findavgTimeFCFS(ProcessType plist[], int n) {
    // Find waiting time of all processes
    findWaitingTimeFCFS(plist, n);
    
    // Find turnaround time for all processes
    findTurnAroundTime(plist, n);
    
    // Display processes along with all details
    printf("\n*********\nFCFS\n");
}

// Function to calculate average time for Priority Scheduling
void findavgTimePriority(ProcessType plist[], int n) {
    // Sort processes by priority (highest priority first)
    qsort(plist, n, sizeof(ProcessType), my_comparer);
    
    // After sorting by priority, use FCFS logic
    findWaitingTimeFCFS(plist, n);
    
    // Find turnaround time for all processes
    findTurnAroundTime(plist, n);
    
    // Display processes along with all details
    printf("\n*********\nPriority\n");
}

// Function to calculate average time for SJF
void findavgTimeSJF(ProcessType plist[], int n) {
    // Find waiting time using SRTF (preemptive SJF)
    findWaitingTimeSJF(plist, n);
    
    // Find turnaround time for all processes
    findTurnAroundTime(plist, n);
    
    // Display processes along with all details
    printf("\n*********\nSJF\n");
}

// Function to calculate average time for Round Robin
void findavgTimeRR(ProcessType plist[], int n, int quantum) {
    // Find waiting time of all processes
    findWaitingTimeRR(plist, n, quantum);
    
    // Find turnaround time for all processes
    findTurnAroundTime(plist, n);
    
    // Display processes along with all details
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

// Function to print metrics
void printMetrics(ProcessType plist[], int n) {
    int total_wt = 0, total_tat = 0;
    float awt, att;
    
    printf("\tProcesses\tBurst time\tWaiting time\tTurn around time\n");
    
    // Calculate total waiting time and total turnaround time
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

// Main function - Complete implementation using util.c functions
int main(int argc, char *argv[]) {
    int n = 0;
    int quantum = 2; // Time quantum for Round Robin
    ProcessType *plist = NULL;
    
    FILE *input_file = NULL;
    
    // Check if filename provided as command line argument
    if (argc == 2) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            printf("Error: Could not open file %s\n", argv[1]);
            return 1;
        }
        
        // Use parse_file from util.c to read processes
        plist = parse_file(input_file, &n);
        fclose(input_file);
        
    } else {
        // Read from stdin
        input_file = stdin;
        printf("Enter process data (PID BT ART WT TAT PRI), Ctrl+D to end:\n");
        
        // Use parse_file from util.c
        plist = parse_file(input_file, &n);
    }
    
    if (plist == NULL || n == 0) {
        printf("Error: No processes to schedule\n");
        return 1;
    }
    
    // Create copies of the process list for each algorithm
    ProcessType *plist_fcfs = (ProcessType *)malloc(n * sizeof(ProcessType));
    ProcessType *plist_priority = (ProcessType *)malloc(n * sizeof(ProcessType));
    ProcessType *plist_sjf = (ProcessType *)malloc(n * sizeof(ProcessType));
    ProcessType *plist_rr = (ProcessType *)malloc(n * sizeof(ProcessType));
    
    // Copy process list to each array
    for (int i = 0; i < n; i++) {
        plist_fcfs[i] = plist[i];
        plist_priority[i] = plist[i];
        plist_sjf[i] = plist[i];
        plist_rr[i] = plist[i];
    }
    
    // Run FCFS scheduling
    findavgTimeFCFS(plist_fcfs, n);
    printMetrics(plist_fcfs, n);
    
    // Run Priority scheduling
    findavgTimePriority(plist_priority, n);
    printMetrics(plist_priority, n);
    
    // Run SJF scheduling
    findavgTimeSJF(plist_sjf, n);
    printMetrics(plist_sjf, n);
    
    // Run Round Robin scheduling
    findavgTimeRR(plist_rr, n, quantum);
    printMetrics(plist_rr, n);
    
    // Free allocated memory
    free(plist);
    free(plist_fcfs);
    free(plist_priority);
    free(plist_sjf);
    free(plist_rr);
    
    return 0;
}
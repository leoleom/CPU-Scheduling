#include "metrics.h"
#include <stdio.h>

void calculate_metrics(Process *processes, int n) {

    if (!processes || n <= 0) {
        printf("[DEBUG] calculate_metrics: no processes\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        
        if (p->finish_time < 0) {
            p->finish_time = 0; // temporary fix to avoid negative metrics
        }

        // Turnaround time = Finish time - Arrival time
        p->turnaround_time = p->finish_time - p->arrival_time;
        
        // Waiting time = Turnaround time - Burst time
        p->waiting_time = p->turnaround_time - p->burst_time;
        
        // Response time = Start time - Arrival time
        if (p->start_time >= 0){
        p->response_time = p->start_time - p->arrival_time;
        } else {
            p->response_time = -1; // process never started
        }
    }
}

double calculate_average_turnaround(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += processes[i].turnaround_time;
    }
    return (n==0) ? 0.0 : sum / n; // check division by 0
}

double calculate_average_waiting(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += processes[i].waiting_time;
    }
    return (n==0) ? 0.0 : sum / n; // check division by 0
}

double calculate_average_response(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += processes[i].response_time;
    }
    return (n==0) ? 0.0 : sum / n; // check division by 0
}

void print_process_metrics(Process p[], int n)
{
    int i;
    printf("\n=== METRICS ===\n");
    printf("\n%-8s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s\n",
       "Process", "AT", "BT", "FT", "TT", "WT", "RT");
    printf("-----------------------------------------------------------------------------------------\n");

    for(i = 0; i < n; i++)
    {
        printf("%-8s | %-10.1f | %-10.1f | %-10.1f | %-10.1f | %-10.1f | %-10.1f\n",
            p[i].pid,
            (float)p[i].arrival_time,
            (float)p[i].burst_time,
            (float)p[i].finish_time,
            (float)p[i].turnaround_time,
            (float)p[i].waiting_time,
            (float)p[i].response_time);
    }

    printf("-----------------------------------------------------------------------------------------\n");

    printf("%-8s | %-10s | %-10s | %-10s | %-10.1f | %-10.1f | %-10.1f\n",
        "Average", "", "", "",
        (float)calculate_average_turnaround(p, n),
        (float)calculate_average_waiting(p, n),
        (float)calculate_average_response(p, n));
}

void print_metrics_calculation(Process p[], int n)
{
    printf("\n=== DETAILED CALCULATION ===\n");
    int i;
    for(i = 0; i < n; i++)
    {
        printf("Process %s:\n", p[i].pid);

        printf("  Arrival Time:     %d\n", p[i].arrival_time);
        printf("  Burst Time:       %d\n", p[i].burst_time);
        printf("  Finish Time:      %d\n", p[i].finish_time);

        printf("  Turnaround Time:  %d - %d = %d\n",
               p[i].finish_time,
               p[i].arrival_time,
               p[i].turnaround_time);

        printf("  Waiting Time:     %d - %d = %d\n",
               p[i].turnaround_time,
               p[i].burst_time,
               p[i].waiting_time);

        printf("  Response Time:    %d - %d = %d\n\n",
               p[i].response_time + p[i].arrival_time,
               p[i].arrival_time,
               p[i].response_time);
    }

}
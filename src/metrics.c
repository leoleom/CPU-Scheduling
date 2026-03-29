#include "metrics.h"
#include <stdio.h>

void calculate_metrics(Process *processes, int n) {

    if (!processes || n <= 0) {
        printf("[DEBUG] calculate_metrics: no processes\n");
        return;
    }

    printf("[DEBUG] calculate_metrics: calculating for %d processes\n", n);

    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        
        if (p->finish_time < 0) {
            printf("[DEBUG] calculate_metrics WARNING: Process %s finish_time not set, using current_time=0\n", p->pid);
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

        printf("[DEBUG] Metrics for Process %s:\n", p->pid);
        printf("  AT=%d, BT=%d, FT=%d, TT=%d, WT=%d, RT=%d\n",
               p->arrival_time,
               p->burst_time,
               p->finish_time,
               p->turnaround_time,
               p->waiting_time,
               p->response_time);

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

    puts("+---------+-------+-------+-------+-------+-------+-------+");
    puts("| Process |   AT  |   BT  |   FT  |   TT  |   WT  |   RT  |");
    puts("+---------+-------+-------+-------+-------+-------+-------+");

    for(i = 0; i < n; i++)
    {
        printf("| %7s |  %3d  |  %3d  |  %3d  |  %3d  |  %3d  |  %3d  |\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].finish_time,
               p[i].turnaround_time,
               p[i].waiting_time,
               p[i].response_time);
        puts("+---------+-------+-------+-------+-------+-------+-------+");
    }
    printf("| Average |       |       |       | %3.1f | %3.1f | %3.1f |", (float)calculate_average_turnaround(p, n), 
    (float)calculate_average_waiting(p, n), (float)calculate_average_response(p, n));
}

void print_metrics_calculation(Process p[], int n)
{
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
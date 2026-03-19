#include "metrics.h"
#include <stdio.h>

void calculate_metrics(Process *processes, int n) {
    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        
        // Turnaround time = Finish time - Arrival time
        p->turnaround_time = p->finish_time - p->arrival_time;
        
        // Waiting time = Turnaround time - Burst time
        p->waiting_time = p->turnaround_time - p->burst_time;
        
        // Response time = Start time - Arrival time
        p->response_time = p->start_time - p->arrival_time;
    }
}

double calculate_average_turnaround(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += processes[i].turnaround_time;
    }
    return sum / n;
}

double calculate_average_waiting(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += processes[i].waiting_time;
    }
    return sum / n;
}

double calculate_average_response(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += processes[i].response_time;
    }
    return sum / n;
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
    printf("| Average |       |       |       | %3.1f | %3.1f | %3.1f |", (float)calculate_average_turnaround(p, n), (float)calculate_average_waiting(p, n), (float)calculate_average_response(p, n));
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
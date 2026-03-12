#include "scheduler.h" 
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    SchedulerState state;

    printf("Enter number of processes: ");
    scanf("%d", &state.num_processes);

    state.processes = malloc(sizeof(Process) * state.num_processes);

    printf("\nEnter Arrival Time and Burst Time:\n");

    for (int i = 0; i < state.num_processes; i++) {
        Process *p = &state.processes[i];

        p->pid[0] = 'A' + i;
        p->pid[1] = '\0'; 
        printf("Process %s\n", p->pid);

        printf("Arrival Time: ");
        scanf("%d", &p->arrival_time);

        printf("Burst Time: ");
        scanf("%d", &p->burst_time);

        p->remaining_time = p->burst_time;

        p->start_time = -1;
        p->finish_time = 0;
        p->turnaround_time = 0;
        p->waiting_time = 0;
        p->response_time = 0;
    }

    schedule_fcfs(&state);

    calculate_metrics(state.processes, state.num_processes);

    printf("\n==== METRICS ====\n\n");
    print_process_metrics(state.processes, state.num_processes);

    printf("\n=== PER PROCESS CALCULATION ===\n\n");
    print_metrics_calculation(state.processes, state.num_processes);

    free(state.processes);

    return 0;
}
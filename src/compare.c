#include "compare.h"
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>

Comparison run_algorithm(char *input, SchedulingAlgorithm algorithm, int quantum)
{
    Comparison c;
    SchedulerState state = {0};

    int count = load_processes(input, &state.processes);
    if (count <= 0)
    {
        fprintf(stderr, "Error loading processes.\n");
        exit(1);
    }

    state.num_processes = count;

    init_scheduler(&state);
    if (algorithm == RR)
        state.rr_quantum = quantum;

    if (algorithm == SJF || algorithm == STCF)
    {
        state.heap = create_heap(count);
        if (!state.heap) {
        fprintf(stderr, "Heap allocation failed\n");
        exit(1);
        }
    }
    if (algorithm == MLFQ)
    {
        MLFQConfig config = {
            .queues = 3,
            .time_quantum = {10, 30, -1},
            .allotment = {20, 100, -1},
            .boost_period = 300
        };
        init_mlfq(&state.mlfq, &config);
    }

    simulate_scheduler(&state, algorithm);

    calculate_metrics(state.processes, state.num_processes);

    c.avg_tt = calculate_average_turnaround(state.processes, state.num_processes);
    c.avg_wt = calculate_average_waiting(state.processes, state.num_processes);
    c.avg_rt = calculate_average_response(state.processes, state.num_processes);
    c.context_switches = state.context_switches;

    free(state.processes);

    if (algorithm == SJF || algorithm == STCF)
    {
        free (state.heap);
    }
    if (algorithm == MLFQ)
        free(state.mlfq.queues);

    return c;
}

void run_comparison(char *input, int quantum)
{
    Comparison fcfs = run_algorithm(input, FCFS, quantum);
    Comparison sjf  = run_algorithm(input, SJF, quantum);
    Comparison stcf = run_algorithm(input, STCF, quantum);
    Comparison rr   = run_algorithm(input, RR, quantum);

    printf("\n=== ALGORITHM COMPARISON ===\n");

    printf("\n%-8s | %-10s | %-10s | %-10s | %-10s\n",
        "Algorithm", "Avg TT", "Avg WT", "Avg RT", "Context Switch");

    printf("------------------------------------------------------------------\n");

    printf("FCFS      | %-10.1f | %-10.1f | %-10.1f | %-10d\n",
        (float)fcfs.avg_tt, (float)fcfs.avg_wt, (float)fcfs.avg_rt, fcfs.context_switches);

    printf("SJF       | %-10.1f | %-10.1f | %-10.1f | %-10d\n",
        (float)sjf.avg_tt, (float)sjf.avg_wt, (float)sjf.avg_rt, sjf.context_switches);

    printf("STCF      | %-10.1f | %-10.1f | %-10.1f | %-10d\n",
        (float)stcf.avg_tt, (float)stcf.avg_wt, (float)stcf.avg_rt, stcf.context_switches);

    printf("RR (q=%d) | %-10.1f | %-10.1f | %-10.1f | %-10d\n",
        quantum, (float)rr.avg_tt, (float)rr.avg_wt, (float)rr.avg_rt, rr.context_switches);
}
#include "compare.h"
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>

Result run_algorithm(char *input, SchedulingAlgorithm algorithm, int quantum)
{
    Result r;
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
        state.heap.process = malloc(count * sizeof(Process *));
        state.heap.capacity = count;
        state.heap.size = 0;
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

    r.avg_tt = calculate_average_turnaround(state.processes, state.num_processes);
    r.avg_wt = calculate_average_waiting(state.processes, state.num_processes);
    r.avg_rt = calculate_average_response(state.processes, state.num_processes);
    r.context_switches = state.context_switches;

    free(state.processes);

    if (algorithm == SJF || algorithm == STCF)
        free(state.heap.process);

    if (algorithm == MLFQ)
        free(state.mlfq.queues);

    return r;
}
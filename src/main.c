#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include "metrics.h"
#include "process.h"
#include "heap.h"

int main(int argc, char *argv[])
{

    char *algorithm_str = NULL;
    char *algorithm;
    char *input = NULL;
    int quantum = 20;

    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--algorithm=", 12) == 0)
        {
            algorithm_str = argv[i] + 12;
        }
        else if (strncmp(argv[i], "--input=", 8) == 0)
        {
            input = argv[i] + 8;
        }
        else if (strncmp(argv[i], "--quantum=", 10) == 0)
        {
            quantum = atoi(argv[i] + 10);
        }
    }

    if (!algorithm_str || !input)
    {
        printf("Usage: ./main --algorithm=FCFS --input=file.txt\n");
        return 1;
    }

    SchedulerState state;

    state.num_processes = load_processes(input, &state.processes);

    init_scheduler(&state);

    if (strcmp(algorithm_str, "FCFS") == 0)
    {
        algorithm = FCFS;
    }
    else if (strcmp(algorithm_str, "SJF") == 0)
    {
        algorithm = SJF;
    }
    else if (strcmp(algorithm_str, "STCF") == 0)
    {
        algorithm = STCF;
    }
    else if (strcmp(algorithm_str, "RR") == 0)
    {
        algorithm = RR;
        state.rr_quantum = quantum;
    }
    else if(strcmp(algorithm_str, "MLFQ") == 0)
    {
        algorithm = MLFQ;

        // set the configurations
        MLFQConfig config;

        config.num_queues = 3;                // Q0, Q1, Q2
        int quanta[3] = {10, 30, -1};         // Q0, Q1, Q2 (FCFS)
        int allotment[3] = {20, 100, -1};     // max time per queue
        config.time_quantum = quanta;
        config.allotment = allotment;
        config.boost_period = 300;            // priority boost every 300 units

        // initialize the MLFQ inside SchedulerState
        init_mlfq(&state.mlfq, &config);
    }
    else
    {
        printf("Unknown algorithm: %s\n", algorithm_str);
        return 0;
    }

    simulate_scheduler(&state, algorithm);

    printf("=== METRICS ===\n");
    print_process_metrics(state.processes, state.num_processes);

    printf("\n=== DETAILED CALCULATION ===\n");
    print_metrics_calculation(state.processes, state.num_processes);

    free(state.processes);

    return 0;
}
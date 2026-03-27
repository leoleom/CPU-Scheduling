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
    SchedulingAlgorithm algorithm;
    char *input = NULL;
    int quantum = 10;

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
        fprintf(stderr,"Usage: ./main --algorithm=FCFS --input=file.txt\n");
        return 1;
    }

    // defense ulet daw
    if (quantum <= 0) {
        fprintf(stderr, "Error: Quantum must be a positive integer.\n");
        return 1;
    }

    SchedulerState state;

    // security personel for processes edi wow
    int count = load_processes(input, &state.processes);
    if (count <= 0) {
        fprintf(stderr, "Error: Failed to load processes from %s.\n", input);
        return 1;
    }

    state.num_processes = count;

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
    else if (strcmp(algorithm_str, "MLFQ") == 0)
    {
        algorithm = MLFQ;

        // set the configurations
        MLFQConfig config = {
            .queues = 3,
            .time_quantum = {10, 30, -1},
            .allotment = {20, 100, -1},
            .boost_period = 300 // priority boost every 300 units (arbitrary)
        };

        // handle error
        if (config.queues > 3|| config.queues <= 0) {
            fprintf(stderr, "Error: MLFQ must have between 1 and %d queues.\n", 3);
            return 1;
        }

        // initialize the MLFQ inside SchedulerState
        init_mlfq(&state.mlfq, &config);
    }
    else
    {
        printf("Unknown algorithm: %s\n", algorithm_str);
        return 1;
    }

    if (simulate_scheduler(&state, algorithm) == -1)
    {
        fprintf(stderr, "Simulation failed.\n");
        free(state.processes);
        return 1;
    }

    printf("=== METRICS ===\n");
    print_process_metrics(state.processes, state.num_processes);

    printf("\n=== DETAILED CALCULATION ===\n");
    print_metrics_calculation(state.processes, state.num_processes);

    free(state.processes);

    return 0;
}
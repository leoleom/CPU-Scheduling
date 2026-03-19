#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include "metrics.h"
#include "process.h"
#include "heap.h"

int main(int argc, char *argv[])
{

    char *algorithm = NULL;
    char *input = NULL;
    int quantum = 3;

    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--algorithm=", 12) == 0)
        {
            algorithm = argv[i] + 12;
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

    if (!algorithm || !input)
    {
        printf("Usage: ./main --algorithm FCFS --input file.txt\n");
        return 1;
    }

    SchedulerState state;

    state.num_processes = load_processes(input, &state.processes);

    init_scheduler(&state);

    if (strcmp(algorithm, "FCFS") == 0)
    {
        schedule_fcfs(&state);
    }
    else if (strcmp(algorithm, "SJF") == 0)
    {
        MinHeap *heap = create_heap(state.num_processes);
        schedule_sjf(&state, heap);
        free_heap(heap);
    }
    else if (strcmp(algorithm, "STCF") == 0)
    {
        MinHeap *heap = create_heap(state.num_processes);
        schedule_stcf(&state, heap);
        free_heap(heap);
    }
    else if (strcmp(algorithm, "RR") == 0)
    {
        schedule_rr(&state, quantum);
    }
    else
    {
        printf("Unknown algorithm: %s\n", algorithm);
    }

    calculate_metrics(state.processes, state.num_processes);

    printf("=== METRICS ===\n");
    print_process_metrics(state.processes, state.num_processes);

    printf("\n=== DETAILED CALCULATION ===\n");
    print_metrics_calculation(state.processes, state.num_processes);

    free(state.processes);

    return 0;
}
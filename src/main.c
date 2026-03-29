#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include "metrics.h"
#include "process.h"
#include "compare.h"
#include "gantt.h"
#include "heap.h"

int main(int argc, char *argv[])
{

    char *algorithm_str = NULL;
    SchedulingAlgorithm algorithm;
    char *input = NULL;
    int quantum = 10;
    int compare_mode = 0;

    for (int i = 1; i < argc; i++)
    {
         if (strcmp(argv[i], "--compare") == 0)
        {
            compare_mode = 1;
        }
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
    // quantum validation 
    if (quantum <= 0)
    {
        fprintf(stderr, "Error: Quantum must be a positive integer.\n");
        return 1;
    }

    if (compare_mode)
    {
        run_comparison(input, quantum);
        return 0;
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


    state.num_processes = count;    // restore number of processes
    printf("[DEBUG] Loaded %d processes from '%s'\n", count, input);    
    for (int i = 0; i < count; i++) {
        printf("[DEBUG] Process %s: AT=%d, BT=%d\n",
               state.processes[i].pid,
               state.processes[i].arrival_time,
               state.processes[i].burst_time);
    }
    init_scheduler(&state);
    printf("[DEBUG] Scheduler initialized\n");
    

    if (strcmp(algorithm_str, "FCFS") == 0)
    {
        algorithm = FCFS;
    }
    else if (strcmp(algorithm_str, "SJF") == 0 || strcmp(algorithm_str, "STCF") == 0) {
        // Determine which algorithm
        algorithm = (strcmp(algorithm_str, "SJF") == 0) ? SJF : STCF;

        // Allocate heap for SJF/STCF
        state.heap = create_heap(count);
        if (!state.heap){
            fprintf(stderr, "Fatal: Heap allocation failed.\n");
            free(state.heap->process);
            free(state.processes);
            return 1;
        }
        printf("[DEBUG] Heap created with capacity %d\n", state.heap->capacity);
    } else if (strcmp(algorithm_str, "RR") == 0) 
    { 
        algorithm = RR; state.rr_quantum = quantum; 
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
        if (config.queues > MAX_QUEUES || config.queues <= 0) {
            fprintf(stderr, "Error: MLFQ must have between 1 and %d queues.\n", MAX_QUEUES);
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

    int total_time = 0;
    for (int i = 0; i < state.num_processes; i++)
    {
        total_time += state.processes[i].burst_time;

        gantt_init(total_time + 10);
        printf("[DEBUG] Gantt chart initialized with size %d\n", total_time + 10);
    }

    printf("[DEBUG] Heap=%p, processes=%p, \n", (void*)state.heap, (void*)state.processes);
    if (simulate_scheduler(&state, algorithm) == -1)
    {
        fprintf(stderr, "Simulation failed.\n");
        free(state.processes);
        return 1;
    }
      printf("[DEBUG] Scheduler simulation completed\n");

    gantt_print(state.current_time);
    gantt_free();

    print_process_metrics(state.processes, state.num_processes);

    print_metrics_calculation(state.processes, state.num_processes);

    if (algorithm == MLFQ)
        free(state.mlfq.queues);
 
    if (algorithm == SJF || algorithm == STCF)
        free(state.heap->process);

    free(state.processes);

    return 0;
}
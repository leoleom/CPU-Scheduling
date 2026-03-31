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
    int quantum = 30;
    int compare_mode = 0;
    char *command_processes = NULL;
    char *mlfq_config_file = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--compare") == 0)
        {
            compare_mode = 1;
        }
        else if (strncmp(argv[i], "--algorithm=", 12) == 0)
        {
            algorithm_str = argv[i] + 12;
        }
        else if (strncmp(argv[i], "--quantum=", 10) == 0)
        {
            quantum = atoi(argv[i] + 10);
            // printf("Using time quantum q=%d\n\n", quantum);
        }
        else if (strncmp(argv[i], "--mlfq-config=", 14) == 0)
        {
            mlfq_config_file = argv[i] + 14;
        }
        else if (strncmp(argv[i], "--input=", 8) == 0)
        {
            input = argv[i] + 8;
        }
        else if (strncmp(argv[i], "--processes=", 12) == 0)
        {
            command_processes = argv[i] + 12;
        }
    }
    // quantum validation
    if (quantum <= 0)
    {
        fprintf(stderr, "Error: Quantum must be a positive integer.\n");
        return 1;
    }

    if (compare_mode)
    {   if (input) run_comparison(input, NULL, quantum);
        else if (command_processes) run_comparison(NULL, command_processes, quantum);
        else 
        {
            fprintf(stderr, "Error: No input provided for comparison mode.\n");
            return 1;
        }
        return 0;
    }

    if (!algorithm_str || (!input && !command_processes))
    {
        fprintf(stderr, "Usage: ./main --algorithm=FCFS --input=file.txt\n");
        return 1;
    }

    // defense ulet daw
    if (quantum <= 0)
    {
        fprintf(stderr, "Error: Quantum must be a positive integer.\n");
        return 1;
    }

    SchedulerState state;

    int count = 0;
    if (command_processes)
    {
        count = load_command(command_processes, &state.processes);
    }
    else
    {
        count = load_processes(input, &state.processes);
    }

    state.num_processes = count; // restore number of processes
    init_scheduler(&state);

    if (strcmp(algorithm_str, "FCFS") == 0)
    {
        algorithm = FCFS;
    }
    else if (strcmp(algorithm_str, "SJF") == 0 || strcmp(algorithm_str, "STCF") == 0)
    {
        // Determine which algorithm
        algorithm = (strcmp(algorithm_str, "SJF") == 0) ? SJF : STCF;

        // Allocate heap for SJF/STCF
        state.heap = create_heap(count);
        if (!state.heap)
        {
            fprintf(stderr, "Fatal: Heap allocation failed.\n");
            //free(state.heap->process);
            free(state.processes);
            return 1;
        }
    }
    else if (strcmp(algorithm_str, "RR") == 0)
    {
        printf("Time quantum set to %d\n\n", quantum);
        algorithm = RR;
        state.rr_quantum = quantum;
    }
    else if (strcmp(algorithm_str, "MLFQ") == 0)
    {
        algorithm = MLFQ;
        MLFQConfig config;
        if (mlfq_config_file)
        {
            config = load_mlfq_config(mlfq_config_file);
        }
        else
        {
            config = (MLFQConfig){
                .queues = 3,
                .time_quantum = {10, 30, -1},
                .allotment = {50, 150, -1},
                .boost_period = 300
            };
        }

        // handle error
        if (config.queues > MAX_QUEUES || config.queues <= 0)
        {
            fprintf(stderr, "Error: MLFQ must have between 1 and %d queues.\n", MAX_QUEUES);
            return 1;
        }

        // initialize the MLFQ inside SchedulerState
        init_mlfq(&state.mlfq, &config);

        print_mlfq_config(&config);
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
    }
    gantt_init(total_time + 10);

    if (simulate_scheduler(&state, algorithm) == -1)
    {
        fprintf(stderr, "Simulation failed.\n");
        free(state.processes);
        return 1;
    }

        if (algorithm == MLFQ)
    {
        printf("\n=== Analysis ===\n");

        for (int i = 0; i < state.num_processes; i++)
        {
            Process *p = &state.processes[i];

            if (p->burst_time <= 100)
            {
                printf("Interactive: %s finished quickly (TT=%d)\n",
                    p->pid, p->turnaround_time);
            }
            else
            {
                printf("Long job: %s TT=%d\n",
                    p->pid, p->turnaround_time);
            }
        }
    }

    gantt_print(state.current_time);
    gantt_free();

    print_process_metrics(state.processes, state.num_processes);
    if (algorithm == FCFS)
    {
        detect_convoy_effect(&state);
    }
    printf("\nTotal context switches: %d\n", state.context_switches);
    print_metrics_calculation(state.processes, state.num_processes);

    if (algorithm == MLFQ)
        free(state.mlfq.queues);

    if (algorithm == SJF || algorithm == STCF)
        free(state.heap->process);

    free(state.processes);

    return 0;
}
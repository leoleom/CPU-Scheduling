#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include "metrics.h"
#include "process.h"

int main(int argc, char *argv[]) {

    char *algorithm = NULL;
    char *input = NULL;

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--algorithm") == 0)
            algorithm = argv[++i];

        else if (strcmp(argv[i], "--input") == 0)
            input = argv[++i];
    }

    if (!algorithm || !input) {
        printf("Usage: ./main --algorithm FCFS --input file.txt\n");
        return 1;
    }

    SchedulerState state;

    state.num_processes = load_processes(input, &state.processes);

    if (strcmp(algorithm, "FCFS") == 0) {
        schedule_fcfs(&state);
    }
    else if (strcmp(algorithm, "SJF") == 0) {
        schedule_sjf(&state);
    }
    else if (strcmp(algorithm, "STCF") == 0) {
        schedule_stcf(&state);
    }
    else {
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
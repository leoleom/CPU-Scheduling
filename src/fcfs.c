#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_fcfs(SchedulerState *state){
    if (!state || state->num_processes == 0)
    return -1;

    // initialize the scheduler state
    init_scheduler(state);

    int completed = 0;
    int time = 0;

    //  while (completed < state->num_processes) {
    //     // enqueue any new arrivals at this time
    //     for (int i = 0; i < state->num_processes; i++) {
    //         Process *p = &state->processes[i];
    //         if (p->arrival_time == time) {
    //             enqueue(&state->ready_queue, p);
    //         }
    // }

    // loop until all processes are finished
    while (completed < state->num_processes) {

        // pick next process if idle
        if (!state->current_process && state->ready_queue.size > 0) {
            Node* node = dequeue(&state->ready_queue);

            if (node) {
                state->current_process = node->process;  // assign the process pointer
                free(node);                              // freedom
            }

            // record start time
            if (state->current_process->start_time == -1)
                state->current_process->start_time = time;
        }
        // Execute the current process
        if (state->current_process) {
            state->current_process->remaining_time--;

            // Check if process has finished
            if (state->current_process->remaining_time == 0) {
                state->current_process->finish_time = time + 1;
                completed++;
                state->current_process = NULL;
            }
        }

        time++;
    }

    return 0;
}

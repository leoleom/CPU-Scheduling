#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>

// Round Robin Scheduling (Preemptive)
int schedule_rr(SchedulerState *state, int quantum) {
    static int quantum_counter = 0; // track how much time current process has used in this quantum

    // Pick next process if CPU is idle
    if (!state->current_process && state->ready_queue.size > 0) {
        Node *node = dequeue(&state->ready_queue);
        if (node) {
            state->current_process = node->process;
            free(node);

            // record start time if first execution
            if (state->current_process->start_time == -1)
                state->current_process->start_time = state->current_time;

            quantum_counter = 0; // reset quantum counter for new process
        }
    }

    // Execute current process
    if (state->current_process) {
        state->current_process->remaining_time--;
        quantum_counter++;

        // Check if process finished
        if (state->current_process->remaining_time == 0) {
            state->current_process->finish_time = state->current_time + 1;
            state->current_process = NULL;
            quantum_counter = 0; // reset quantum counter
        }
        // Quantum expired? Preempt and enqueue at the end
        else if (quantum_counter >= quantum) {
            enqueue(&state->ready_queue, state->current_process);
            state->current_process = NULL;
            quantum_counter = 0;
        }
    }

    return 0;
}
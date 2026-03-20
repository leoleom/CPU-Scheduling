#include "scheduler.h"
#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_rr(SchedulerState *state, int quantum)
{

    if (!state || state->num_processes == 0)
        return -1;

    int completed = 0;
    int time = 0;
    int quantum_counter = 0;
    int gantt_index = 0;

    gantt_init(10000);

    while (completed < state->num_processes)
    {

        handle_arrivals_queue(state, time);

        // pick next process if CPU idle
        if (!state->current_process && state->ready_queue.size > 0)
        {
            // Fix: Dequeue returns Node *, so extract Process * from it and free the node
            Node *node = dequeue(&state->ready_queue);
            state->current_process = node->process;
            free(node);

            // record start time if first execution
            if (state->current_process->start_time == -1)
                state->current_process->start_time = time;

            quantum_counter = 0; // reset quantum counter
        }

        // execute current process
        if (state->current_process)
        {
            // Add to Gantt chart
            char pid = state->current_process->pid[0];
            gantt_add(gantt_index, pid);
            gantt_index++;

            // run for 1 time unit
            state->current_process->remaining_time--;
            quantum_counter++;

            // check if finished
            if (state->current_process->remaining_time == 0)
            {
                state->current_process->finish_time = time + 1;
                state->current_process = NULL;
                quantum_counter = 0;
                completed++;
            }
            // if quantum expired, preempt and enqueue at the end
            else if (quantum_counter >= quantum)
            {
                enqueue(&state->ready_queue, state->current_process);
                state->current_process = NULL;
                quantum_counter = 0;
            }
        }

        time++;
        state->current_time = time;
    }

    gantt_print(time);
    return 0;
}
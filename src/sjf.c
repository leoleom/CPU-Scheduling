
#include "scheduler.h"
#include "heap.h"
#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_sjf(SchedulerState *state, MinHeap *heap) {

    if (!state || state->num_processes == 0)
        return -1;

    int completed = 0;
    int time = 0;
    int gantt_index = 0;
    

    while (completed < state->num_processes) {

        // ARRIVALS
        handle_arrivals_sjf(state, heap, time);

        // pick next process if idle
        if (!state->current_process && heap->size > 0) {

            state->current_process =
                heap_extract_min(heap, cmp_sjf);

            if (state->current_process->start_time == -1)
                state->current_process->start_time = time;
        }

        // execute current process
        if (state->current_process) {

            char pid = state->current_process->pid[0];
            gantt_add(gantt_index, pid);
            gantt_index++;
        
            state->current_process->remaining_time--;

            if (state->current_process->remaining_time == 0) {
                state->current_process->finish_time = time + 1;
                completed++;
                state->current_process = NULL;
            }
        }

        time++;
    }

    gantt_print(time);
    return 0;
}
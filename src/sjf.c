
#include "scheduler.h"
#include "heap.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_sjf(SchedulerState *state) {

    if (!state || state->num_processes == 0)
        return -1;

    init_scheduler(state);

    MinHeap *heap = create_heap(state->num_processes);

    int completed = 0;
    int time = 0;

    while (completed < state->num_processes) {

        // ARRIVALS
        for (int i = 0; i < state->num_processes; i++) {
            Process *p = &state->processes[i];

            if (p->arrival_time == time) {
                heap_insert(heap, p, cmp_sjf);
            }
        }

        // pick next process if idle
        if (!state->current_process && heap->size > 0) {

            state->current_process =
                heap_extract_min(heap, cmp_sjf);

            if (state->current_process->start_time == -1)
                state->current_process->start_time = time;
        }

        // execute current process
        if (state->current_process) {

            state->current_process->remaining_time--;

            if (state->current_process->remaining_time == 0) {
                state->current_process->finish_time = time + 1;
                completed++;
                state->current_process = NULL;
            }
        }

        time++;
    }

    free_heap(heap);
    return 0;
}
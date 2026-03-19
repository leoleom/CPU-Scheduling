#include "heap.h"
#include "scheduler.h"
#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_stcf(SchedulerState *state) {

    if (!state || state->num_processes == 0)
        return -1;

    init_scheduler(state);

    MinHeap *heap = create_heap(state->num_processes);

    int completed = 0;
    int time = 0;
    int gantt_index = 0;

    while (completed < state->num_processes) {

        // ARRIVALS
        for (int i = 0; i < state->num_processes; i++) {
            Process *p = &state->processes[i];

            if (p->arrival_time == time) {
                heap_insert(heap, p, cmp_stcf);
            }
        }

        // PREEMPTION CHECK
        if (heap->size > 0) {

            Process *shortest = heap_peek(heap);

            if (!state->current_process ||
                shortest->remaining_time <
                state->current_process->remaining_time) {

                // put current back
                if (state->current_process) {
                    heap_insert(heap, state->current_process, cmp_stcf);
                }

                // switch to shortest
                state->current_process =
                    heap_extract_min(heap, cmp_stcf);

                if (state->current_process->start_time == -1)
                    state->current_process->start_time = time;
            }
        }

        // EXECUTION
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
    free_heap(heap);
    return 0;
}
#include "heap.h"
#include "scheduler.h"
#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_stcf(SchedulerState *state, MinHeap *heap)
{

    if (!state || state->num_processes == 0)
        return -1;
    
    if (state->current_process != NULL)
        return 0;

    // do nothing if heap is empty
    if (heap->size == 0)
        return 0;

    // pick shortest remaining process
    Process *shortest = heap_extract_min(heap, cmp_stcf);
    state->current_process = shortest;

    // record start time
    if (shortest->start_time == -1)
        shortest->start_time = state->current_time;

    // schedule completion event
    int finish_time = state->current_time + shortest->remaining_time;
    schedule_event(state, shortest, EVENT_COMPLETION, finish_time);

    return 0;
}

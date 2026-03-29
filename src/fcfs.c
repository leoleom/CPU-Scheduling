#include "scheduler.h"
#include "metrics.h"
#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_fcfs(SchedulerState *state)
{

    if (!state || state->num_processes == 0)
        return -1;

     if (state->current_process != NULL)
        return 0;

    // do nothing if queue empty
    if (state->ready_queue.size == 0)
        return 0;

    // pick next process
    Node *node = dequeue(&state->ready_queue);
    if (!node)
        return -1;
        
    Process *p = node->process;
    free(node);

    track_context_switch(state, p);
    state->current_process = p;

    // response time tracking
    if (p->start_time == -1)
        p->start_time = state->current_time;

    //  schedule completion instead of simulating time
    int finish_time = state->current_time + p->remaining_time;
    schedule_event(state, p, EVENT_COMPLETION, finish_time);

    return 0;
}

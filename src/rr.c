#include "scheduler.h"
#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>

int schedule_rr(SchedulerState *state, int quantum)
{

    if (!state || state->num_processes == 0)
        return -1;

    if (state->current_process != NULL)
        return 0;

    // do nothing if queue empty
    if (state->ready_queue.size == 0)
        return 0;

    Node *node = dequeue(&state->ready_queue);
    if (!node)
        return -1;

    Process *p = node->process;
    free(node);

    state->current_process = p;

    // Record first execution (response time)
    if (p->start_time == -1)
        p->start_time = state->current_time;

    // Determine how long it will run
    int run_time = (p->remaining_time < quantum) ? p->remaining_time : quantum;

    int event_time = state->current_time + run_time;

    // Decide what event to schedule
    if (p->remaining_time <= quantum)
    {
        // Process will finish
        schedule_event(state, p, EVENT_COMPLETION, event_time);
    }
    else
    {
        // Quantum will expire
        schedule_event(state, p, EVENT_QUANTUM_EXPIRE, event_time);
    }

    return 0;
}
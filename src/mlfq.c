#include "scheduler.h"
#include "gantt.h"

// SCHEDULER
int schedule_mlfq(SchedulerState *state)
{

    if (!state || state->num_processes == 0)
        return -1;

    MLFQScheduler *sched = &state->mlfq;

    // Pick the next process if CPU is idle
    if (state->current_process == NULL)
    {
        for (int i = 0; i < sched->num_queues; i++)
        {
            if (sched->queues[i].size > 0)
            {
                Node *node = dequeue_mlfq(&sched->queues[i]);
                Process *p = node->process;
                free(node);

                state->current_process = p;
                track_context_switch(state, p);

                // set response time (start_time) only on the first run
                if (p->start_time == -1)
                    p->start_time = state->current_time;

                int q = sched->queues[i].time_quantum;
                int allotment = sched->queues[i].allotment;

                // calculate how much time left in priority
                int rem_allotment = (allotment == -1) ? p->remaining_time : (allotment - p->time_in_queue);

                // determine run time
                int run_time = p->remaining_time;
                if (q != -1 && q < run_time)
                    run_time = q;
                if (allotment != -1 && rem_allotment < run_time)
                    run_time = rem_allotment;

                // ensure run_time is at least 1 tick
                if (run_time <= 0)
                {
                    schedule_event(state, p, EVENT_COMPLETION, state->current_time);
                }
                else
                {
                    int event_time = state->current_time + run_time;
                    if (run_time >= p->remaining_time)
                        schedule_event(state, p, EVENT_COMPLETION, event_time);
                    else
                        schedule_event(state, p, EVENT_QUANTUM_EXPIRE, event_time);
                }
                return 0;
            }
        }
    }
    return 0;
}

// MLFQ HELPERS

void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p)
{
    if (!p)
        return;

    MLFQQueue *current_queue = &scheduler->queues[p->priority];

    // Check if process exhausted its allotment
    if (p->time_in_queue >= current_queue->allotment && current_queue->allotment != -1)
    {
        // Demote to lower priority
        if (p->priority < scheduler->num_queues - 1)
        {
            p->priority++;
            p->time_in_queue = 0; // Reset allotment
        }
    }

    // Allotment not exhausted, put back in same queue
    enqueue_mlfq(&scheduler->queues[p->priority], p);
}

void mlfq_priority_boost(MLFQScheduler *scheduler, int current_time)
{
    if (current_time - scheduler->last_boost >= scheduler->boost_period)
    {
        // Move all processes to highest priority
        for (int i = 1; i < scheduler->num_queues; i++)
        {
            MLFQQueue *queue = &scheduler->queues[i];
            while (queue->size > 0)
            {
                Node *node = dequeue_mlfq(queue);
                Process *p = node->process;
                p->priority = 0;
                p->time_in_queue = 0;
                enqueue_mlfq(&scheduler->queues[0], p);
                free(node);
            }
        }
        scheduler->last_boost = current_time;
    }
}

void mlfq_check_preemption(SchedulerState *state, MLFQScheduler *sched)
{
    if (state->current_process == NULL)
        return;

    for (int i = 0; i < state->current_process->priority; i++)
    {
        // if a higher-priority queue has a process, preempt current process
        if (sched->queues[i].size > 0)
        {

            Process *p = state->current_process;

            // put the running process back into its queue
            enqueue_mlfq(&sched->queues[p->priority], p);
            state->current_process = NULL;
            return;
        }
    }
}

void mlfq_select_next_process(SchedulerState *state, MLFQScheduler *sched)
{
    if (state->current_process != NULL)
        return; // CPU is already running a process

    for (int i = 0; i < sched->num_queues; i++)
    {
        if (sched->queues[i].size > 0)
        {
            Node *node = dequeue_mlfq(&sched->queues[i]);
            // state->current_process = node->process;
            Process *next = node->process;
            track_context_switch(state, next);
            state->current_process = next;
            free(node);
            break;
        }
    }
}

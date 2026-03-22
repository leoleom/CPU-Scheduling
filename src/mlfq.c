#include "scheduler.h"
#include "gantt.h"


void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p)
{
    MLFQQueue *current_queue = &scheduler->queues[p->priority];

    // Check if process exhausted its allotment
    if (p->time_in_queue >= current_queue->allotment)
    {
        // Demote to lower priority
        if (p->priority < scheduler->num_queues - 1)
        {
            p->priority++;
            p->time_in_queue = 0; // Reset allotment
            enqueue_mlfq(&scheduler->queues[p->priority], p);
        }
    }
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
                Process *p = dequeue_mlfq(queue);
                p->priority = 0;
                p->time_in_queue = 0;
                enqueue_mlfq(&scheduler->queues[0], p);
            }
        }
        scheduler->last_boost = current_time;
    }
}

void mlfq_check_preemption(SchedulerState *state, MLFQScheduler *sched)
{
    // if a higher-priority queue has a process, preempt current process
    if (state->current_process != NULL && state->current_process->priority > 0 &&
        sched->queues[0].size > 0)
    {
        // put the running process back into its queue
        enqueue(&sched->queues[state->current_process->priority], state->current_process);
        state->current_process = NULL;
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
            state->current_process = dequeue_mlfq(&sched->queues[i]);
            break;
        }
    }
}



// SCHEDULER
int schedule_mlfq(SchedulerState *state, MLFQConfig *config)
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

                // Record start time if first execution
                if (p->start_time == -1)
                    p->start_time = state->current_time;

                // Schedule either completion or quantum expiration
                int q = sched->queues[i].time_quantum;
                int remaining = p->remaining_time;

                if (q > 0 && remaining > q) // quantum-limited
                {
                    schedule_event(state, p, EVENT_QUANTUM_EXPIRE,
                                   state->current_time + q);
                }
                else // process will finish before quantum expires
                {
                    schedule_event(state, p, EVENT_COMPLETION,
                                   state->current_time + remaining);
                }

                break; // CPU now occupied
            }
        }
    }

    return 0;
}
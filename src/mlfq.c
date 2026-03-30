#include <stdio.h>
#include <stdlib.h>
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

void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p, int current_time)
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
            int old = p->priority;
            p->priority++;
            p->time_in_queue = 0; // Reset allotment

            char buf[100];
            sprintf(buf, "Process %s -> Q%d (exhausted Q%d allotment)",
                    p->pid, p->priority, old);
            log_mlfq_event(current_time, buf);
            
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
        log_mlfq_event(current_time, "Priority boost: all processes -> Q0");
    }
}

void mlfq_check_preemption(SchedulerState *state, MLFQScheduler *sched)
{
    if (state->current_process == NULL)
        return;

    int curr_prio = state->current_process->priority;

    for (int i = 0; i < curr_prio; i++)
    {
        if (sched->queues[i].size > 0)
        {
            Process *running = state->current_process;

            char buf[100];
            sprintf(buf, "Process %s preempted by higher priority job", running->pid);
            log_mlfq_event(state->current_time, buf);

            enqueue_mlfq(&sched->queues[running->priority], running);
            state->current_process = NULL;

            // schedule_mlfq(state);
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

MLFQConfig load_mlfq_config(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open MLFQ config");
        exit(1);
    }

    MLFQConfig config;
    fscanf(fp, "%d", &config.queues);

    for (int i = 0; i < config.queues; i++) {
        fscanf(fp, "%d %d",
               &config.time_quantum[i],
               &config.allotment[i]);
    }

    fscanf(fp, "%d", &config.boost_period);

    fclose(fp);
    return config;
}

void print_mlfq_config(MLFQConfig *c)
{
    printf("\n=== MLFQ Configuration ===\n");

    for (int i = 0; i < c->queues; i++) {
        if (c->time_quantum[i] == -1)
            printf("Queue %d: FCFS (lowest priority)\n", i);
        else
            printf("Queue %d: q=%d, allotment=%d%s\n",
                   i,
                   c->time_quantum[i],
                   c->allotment[i],
                   (i == 0) ? " (highest priority)" : "");
    }

    printf("Boost period: %d\n", c->boost_period);
}

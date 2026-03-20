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
            remove_from_queue(current_queue, p);
            p->priority++;
            p->time_in_queue = 0; // Reset allotment
            add_to_queue(&scheduler->queues[p->priority], p);
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


// SCHEDULER
int schedule_mlfq(SchedulerState *state, MLFQConfig *config)
{

    if (!state || state->num_processes == 0)
        return -1;

    int completed = 0;
    int time = 0;
    int gantt_index = 0;
    int current_quantum = 0;

    gantt_init(10000);

    MLFQQueue *queues = malloc(sizeof(MLFQQueue) * config->num_queues);

    for (int i = 0; i < config->num_queues; i++)
    {
        queues[i].level = i;
        queues[i].time_quantum = config->time_quantum[i];
        queues[i].allotment = config->allotment[i];
        queues[i].head = NULL;
        queues[i].tail = NULL;
        queues[i].size = 0;
    }

    int last_boost = 0;

    while (completed < state->num_processes)
    {

        handle_arrivals_mlfq(state, queues, time);

        // priority boost implementationism
        if (time - last_boost >= config->boost_period)
        {
            for (int i = 1; i < config->num_queues; i++)
            {
                while (queues[i].size > 0)
                {
                    Node *node = dequeue_mlfq(&queues[i]);
                    Process *p = node->process;
                    free(node);

                    p->priority = 0;
                    p->time_in_queue = 0;

                    enqueue_mlfq(&queues[0], p);
                }
            }
            last_boost = time;
        }

        // pick process
        if (!state->current_process)
        {
            for (int i = 0; i < config->num_queues; i++)
            {
                if (queues[i].size > 0)
                {
                    Node *node = dequeue_mlfq(&queues[i]);

                    if (node)
                    {
                        state->current_process = node->process;
                        free(node);
                    }

                    if (state->current_process->start_time == -1)
                        state->current_process->start_time = time;

                    current_quantum = 0;
                    break;
                }
            }
        }

        // plspls execute
        if (state->current_process)
        {

            gantt_add(gantt_index++, state->current_process->pid[0]);

            state->current_process->remaining_time--;
            state->current_process->time_in_queue++;
            current_quantum++;

            int lvl = state->current_process->priority;
            int q = config->time_quantum[lvl];
            int allot = config->allotment[lvl];

            // done
            if (state->current_process->remaining_time == 0)
            {
                state->current_process->finish_time = time + 1;
                state->current_process = NULL;
                completed++;
                current_quantum = 0;
            }
            // demote
            else if ((q > 0 && current_quantum >= q) ||
                     (allot > 0 && state->current_process->time_in_queue >= allot))
            {

                if (lvl < config->num_queues - 1)
                {
                    state->current_process->priority++;
                    state->current_process->time_in_queue = 0;
                }

                enqueue_mlfq(&queues[state->current_process->priority],
                        state->current_process);

                state->current_process = NULL;
                current_quantum = 0;
            }
        }
        else
        {
            gantt_add(gantt_index++, '-');
        }

        time++;
    }

    gantt_print(time);
    free(queues);
    return 0;
}
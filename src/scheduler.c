#include <stdio.h>
#include "scheduler.h"
#include "process.h"
#include "metrics.h"
#include "gantt.h"

int simulate_scheduler(SchedulerState *state,
                       SchedulingAlgorithm algorithm)
{
    initialize_events(state, algorithm);
    state->context_switches = 0;
    state->last_process = NULL;
    state->current_process = NULL;

    int prev_time = state->last_event_time;

    printf("Simulation started for algorithm %s\n", get_algorithm_name(algorithm));

    while (state->event_queue != NULL)
    {
        int work_left = 0;
        if (state->current_process != NULL)
        {
            work_left = 1;
        }
        else
        {
            for (int i = 0; i < state->num_processes; i++)
            {
                if (state->processes[i].remaining_time > 0)
                {
                    work_left = 1;
                    break;
                }
            }
        }

        // no one is running, IGNORE the next scheduled events
        if (!work_left)
        {
            break;
        }

        Event *current = pop_event(&state->event_queue);
        if (!current)
            break;

        if ((current->type == EVENT_QUANTUM_EXPIRE || current->type == EVENT_COMPLETION) &&
            current->process != state->current_process)
        {
            free(current);
            continue;
        }

        int delta = current->time - state->last_event_time;
        state->current_time = current->time;

        // add the time
        if (state->current_process != NULL && delta > 0)
        {
            state->current_process->remaining_time -= delta;

            // In MLFQ, time_in_queue tracks how much of the ALLOTMENT was used
            if (algorithm == MLFQ)
            {
                state->current_process->time_in_queue += delta;
            }
        }
        Process *running = state->current_process; // save currently running process

        // fill gantt chart first before handling event

        for (int t = prev_time; t < state->current_time; t++)
        {
            if (state->current_process)
                gantt_add(t, running->pid[0]);
            else
                gantt_add(t, '-');
        }

        prev_time = state->current_time;

        switch (current->type)
        {
            case EVENT_ARRIVAL:
                switch (algorithm)
                {
                case FCFS:
                case RR:
                    handle_arrivals_queue(state, state->current_time);
                    break;
                case SJF:
                    handle_arrivals_sjf(state, state->heap, state->current_time);
                    break;
                case STCF:
                    handle_arrivals_stcf(state, state->heap, state->current_time);
                    break;
                case MLFQ:
                    handle_arrivals_mlfq(state, &state->mlfq, state->current_time);
                    break;
                }
                break;

            case EVENT_COMPLETION:
                if (current->process != state->current_process)
                {
                    free(current);
                    continue;
                }

                handle_completion(state, current->process);
                state->current_process = NULL;

                break;

            case EVENT_QUANTUM_EXPIRE:
                if (current->process != state->current_process)
                {
                    free(current);
                    continue;
                }

                handle_quantum_expire(state, current->process, algorithm);
                if (algorithm == MLFQ)
                {
                    mlfq_adjust_priority(&state->mlfq, current->process);
                }
                state->current_process = NULL; // pick next process

                break;

            case EVENT_PRIORITY_BOOST:
                handle_priority_boost(state);
                break;
        }

        state->last_event_time = state->current_time;
        free(current);

        if (state->current_process == NULL)
        {
            int sched_result = 0;

            switch (algorithm)
            {
            case FCFS:
                sched_result = schedule_fcfs(state);
                break;
            case SJF:
                sched_result = schedule_sjf(state, state->heap);
                break;
            case STCF:
                sched_result = schedule_stcf(state, state->heap);
                break;
            case RR:
                sched_result = schedule_rr(state, state->rr_quantum);
                break;
            case MLFQ:
                sched_result = schedule_mlfq(state);
                break;
            default:
                printf("Unknown scheduling algorithm.\n");
            }

            if (sched_result == -1)
            {
                fprintf(stderr, "Scheduler error: failed to schedule next process "
                                "(algorithm=%d, time=%d)\n",
                        algorithm, state->current_time);
                return -1;
            }
        }
    }

    calculate_metrics(state->processes, state->num_processes);

    return 0;
}

void track_context_switch(SchedulerState *state, Process *next)
{
    if (state->last_process != NULL && state->last_process != next)
    {
        state->context_switches++;
    }

    state->last_process = next;
}
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

    printf("[DEBUG] Simulation started for algorithm %d\n", algorithm);

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
            printf("[DEBUG] All processes finished at %d. Terminating loop.\n", state->current_time);
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

        printf("[DEBUG] Processing event type %d at time %d\n", current->type, state->current_time);
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
            printf("[DEBUG] Event: ARRIVAL of process %s\n", current->process->pid);
            switch (algorithm)
            {
            case FCFS:
            case RR:
                handle_arrivals_queue(state, state->current_time);
                break;
            case SJF:
                handle_arrivals_sjf(state, state->heap, state->current_time);
                printf("[DEBUG] Heap size after arrivals: %d\n", state->heap->size);
                for (int i = 0; i < state->heap->size; i++)
                    printf("  [DEBUG] Heap[%d] Process %c BT=%d\n", i, state->heap->process[i]->pid[0], state->heap->process[i]->burst_time);
                break;
            case STCF:
                handle_arrivals_stcf(state, state->heap, state->current_time);
                break;
            case MLFQ:
                handle_arrivals_mlfq(state, &state->mlfq, state->current_time);
                printf("[DEBUG] MLFQ ready queues after arrivals\n");
                break;
            }
            break;

        case EVENT_COMPLETION:
            if (current->process != state->current_process)
            {
                free(current);
                continue;
            }
            printf("[DEBUG] Event: COMPLETION of process %s\n", current->process->pid);

            handle_completion(state, current->process);
            state->current_process = NULL;
            printf("[DEBUG] Process %s finished at time %d\n", current->process->pid, state->current_time);

            break;

        case EVENT_QUANTUM_EXPIRE:
            if (current->process != state->current_process)
            {
                free(current);
                continue;
            }

            printf("[DEBUG] Event: QUANTUM_EXPIRE for process %s\n", current->process->pid);

            handle_quantum_expire(state, current->process, algorithm);
            if (algorithm == MLFQ)
            {
                mlfq_adjust_priority(&state->mlfq, current->process);
                printf("[DEBUG] Process %s priority adjusted in MLFQ\n", current->process->pid);
            }
            state->current_process = NULL; // pick next process

            break;

        case EVENT_PRIORITY_BOOST:
            printf("[DEBUG] Event: PRIORITY_BOOST at time %d\n", state->current_time);
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
                printf("[DEBUG] Unknown scheduling algorithm.\n");
            }

            if (sched_result == -1)
            {
                fprintf(stderr, "[DEBUG] Scheduler error: failed to schedule next process "
                                "(algorithm=%d, time=%d)\n",
                        algorithm, state->current_time);
                break;
            }

            if (state->current_process)
                printf("[DEBUG] Next process scheduled: %s at time %d\n",
                       state->current_process->pid, state->current_time);
            else
                printf("[DEBUG] No process scheduled at time %d\n", state->current_time);
        }
    }

    calculate_metrics(state->processes, state->num_processes);
    printf("[DEBUG] Metrics calculated for all processes\n");

    return 0;
}

void track_context_switch(SchedulerState *state, Process *next)
{
    if (state->last_process != NULL && state->last_process != next)
    {
        state->context_switches++;
        printf("[DEBUG] Context switch: from %s to %s at time %d\n",
               state->last_process->pid,
               next ? next->pid : "-",
               state->current_time);
    }

    state->last_process = next;
}
#include <stdio.h>
#include "scheduler.h"
#include "process.h"

int simulate_scheduler(SchedulerState *state,
                        SchedulingAlgorithm algorithm)
{
    Event *event_queue = initialize_events(state);
    state->current_process = NULL;

    while (event_queue != NULL)
    {
        Event *current = pop_event(&event_queue);
        state->current_time = current->time;

        // Handle event
        if (current)
        {
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
                    handle_arrivals_sjf(state, &state->heap, state->current_time);
                    break;
                case STCF:
                    handle_arrivals_stcf(state, &state->heap, state->current_time);
                    break;
                case MLFQ:
                    handle_arrivals_mlfq(state, &state->mlfq, state->current_time);
                    break;
                }
                break;
            case EVENT_COMPLETION:
                handle_completion(state, current->process);
                state->current_process = NULL; // pick next process
                break;
            case EVENT_QUANTUM_EXPIRE:
                handle_quantum_expire(state, current->process);
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

            free(current);
        }

        if (state->current_process == NULL)
        {

            int sched_result = 0;

            switch (algorithm)
            {
            case FCFS:
                sched_result = schedule_fcfs(&state);
                break;
            case SJF:
                sched_result = schedule_sjf(&state, state->heap);
                break;
            case STCF:
                sched_result = schedule_stcf(&state, state->heap);
                break;
            case RR:
                sched_result = schedule_rr(&state, state->rr_quantum);
                break;
            case MLFQ:
                sched_result = schedule_mlfq(&state, &state->mlfq);
                break;
            }

            if (sched_result == -1)
            {
                fprintf(stderr, "Scheduler error: failed to schedule next process "
                        "(algorithm=%d, time=%d)\n",
                        algorithm, state->current_time);
                break; 
            }
        }

        // priority boost check (MLFQ only)
        if (algorithm == MLFQ && state->current_time - state->mlfq.last_boost >= state->mlfq.boost_period)
        {
            schedule_event(state, NULL, EVENT_PRIORITY_BOOST, state->current_time);
        }
    }

    calculate_metrics(state->processes, state->num_processes);
    print_results(state);
}


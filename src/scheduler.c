#include "scheduler.h"
#include "process.h"

void simulate_scheduler(SchedulerState *state,
                        SchedulingAlgorithm algorithm)
{
    Event *event_queue = initialize_events(state);

    while (event_queue != NULL)
    {
        Event *current = pop_event(&event_queue);
        state->current_time = current->time;

        switch (current->type)
        {
        case EVENT_ARRIVAL:
            handle_arrival(state, current->process);
            break;
        case EVENT_COMPLETION:
            handle_completion(state, current->process);
            break;
            // ... handle other events
        }

        free(current);
    }

    calculate_metrics(state);
    print_results(state);
}

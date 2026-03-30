#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "mlfq.h"


// INITIALIZATIONS
void init_scheduler(SchedulerState *state)
{   
    if (!state) 
        return;

    SchedulerState tmp = *state;  // save current fields
    memset(state, 0, sizeof(SchedulerState));
    state->processes = tmp.processes; // restore pointer
    state->num_processes = tmp.num_processes; 

    // defensive ulit bro
    while (state->event_queue != NULL) {
        Event *temp = pop_event(&state->event_queue);
        free(temp);
    }

    state->ready_queue.head = NULL;
    state->ready_queue.tail = NULL;
    state->ready_queue.size = 0;

    state->current_time = 0;
    state->last_event_time = 0;

    state->current_process = NULL;
    state->event_queue = NULL;

    state->gantt_chart = NULL;
    state->gantt_size = 0;
    state->rr_quantum        = 0;

    // state->heap->process      = NULL;
    // state->heap->size         = 0;
    // state->heap->capacity     = 0;

    state->heap = NULL;

    for (int i = 0; i < state->num_processes; i++)
    {
        state->processes[i].remaining_time = state->processes[i].burst_time;
        state->processes[i].start_time = -1;
        state->processes[i].finish_time = -1;
        state->processes[i].waiting_time = 0;
        state->processes[i].turnaround_time = 0;
        state->processes[i].priority = 0;
        state->processes[i].time_in_queue = 0;
    }
}

void init_mlfq(MLFQScheduler *sched, MLFQConfig *config)
{

    // extra defense
    if (!sched || !config || config->queues <= 0)
{
    fprintf(stderr, "Error: Invalid MLFQ configuration provided.\n");
    return;
}

    sched->num_queues = config->queues;
    sched->boost_period = config->boost_period;
    sched->last_boost = 0;

    sched->queues = malloc(sizeof(MLFQQueue) * config->queues);

    // safety check liwat kuno
    if (!sched->queues)
    {
        fprintf(stderr, "Fatal: Could not allocate MLFQ queues.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < config->queues; i++)
    {
        sched->queues[i].level = i;
        sched->queues[i].time_quantum = config->time_quantum[i];
        sched->queues[i].allotment = config->allotment[i];
        sched->queues[i].head = sched->queues[i].tail = NULL;
        sched->queues[i].size = 0;
    }
}

/* used for fcfs and rr */
void enqueue(Queue *queue, Process *proc)
{

    // walang sawang check 'to bro
    if (!queue || !proc)
        return;

    Node *new_node = (Node *)malloc(sizeof(Node));

    // another safety check :<
    if (!new_node)
    {
        fprintf(stderr, "Fatal: Memory allocation failed for Queue Node.\n");
        exit(EXIT_FAILURE);
    }

    new_node->process = proc;
    new_node->next = NULL;

    if (queue->tail == NULL)
    { // empty queue
        queue->head = queue->tail = new_node;
    }
    else
    {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    queue->size++;
}

Node *dequeue(Queue *queue)
{
    if (queue->head == NULL) // check if empty
        return NULL;

    Node *temp = queue->head;
    queue->head = queue->head->next; // move the head to the next process

    if (queue->head == NULL) // if queue is now empty
        queue->tail = NULL;

    temp->next = NULL; // delete the reference
    queue->size--;
    return temp;
}

// MLFQ QUEUE
void enqueue_mlfq(MLFQQueue *q, Process *p)
{
    Node *node = malloc(sizeof(Node));
    // malloc check again hays
    if (!node) {
        fprintf(stderr, "Fatal: MLFQ Node allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    node->process = p;
    node->next = NULL;

    if (!q->head)
    {
        q->head = q->tail = node;
    }
    else
    {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;
}

Node *dequeue_mlfq(MLFQQueue *q)
{
    if (!q->head)
        return NULL;
    Node *node = q->head;
    q->head = node->next;
    if (!q->head)
        q->tail = NULL;
    q->size--;
    return node;
}

// HANDLE ARRIVALS
void handle_arrivals_queue(SchedulerState *state, int time)
{
    for (int i = 0; i < state->num_processes; i++)
    {
        Process *p = &state->processes[i];

        if (p->arrival_time == time)
        {
            enqueue(&state->ready_queue, p);
        }
    }
}

void handle_arrivals_stcf(SchedulerState *state, MinHeap *heap, int time)
{
    for (int i = 0; i < state->num_processes; i++)
    {
        Process *p = &state->processes[i];

        if (p->arrival_time == time)
        {
            heap_insert(heap, p, cmp_stcf);
        }
    }

    // preempt if needed
    if (state->current_process != NULL && heap->size > 0)
    {
        Process *shortest = heap_peek(heap);
        if (shortest->remaining_time < state->current_process->remaining_time)
        {
            printf("DEBUG: %s last_start=%d, now=%d, delta=%d\n",
            shortest->pid,
            shortest->last_start_time,
            state->current_time,
            state->current_time - shortest->last_start_time);

            printf("Process %s was preempted at t=%d (remaining: %d)\n",
            state->current_process->pid,
            state->current_time,
            state->current_process->remaining_time);

            state->current_process->was_preempted = 1; //marked as preempted

            heap_insert(heap, state->current_process, cmp_stcf);
            state->current_process = NULL;
        }
    }
}

void handle_arrivals_sjf(SchedulerState *state, MinHeap *heap, int time)
{
    for (int i = 0; i < state->num_processes; i++)
    {
        Process *p = &state->processes[i];
        if (p->arrival_time <= time && p->start_time == -1)
        {
            heap_insert(heap, p, cmp_sjf); // uses burst_time
        }
    }
}

void handle_arrivals_mlfq(SchedulerState *state, MLFQScheduler *sched, int time)
{
    int arrived = 0;
    for (int i = 0; i < state->num_processes; i++)
    {
        Process *p = &state->processes[i];
        char buff[100];
        if (p->arrival_time == time)
        {
            p->priority = 0;
            p->time_in_queue = 0;
            enqueue_mlfq(&sched->queues[0], p);
            arrived = 1;
            sprintf(buff, "Process %s enters Q%d", p->pid, p->priority);
            int current_time = state->current_time;
            log_mlfq_event(current_time, buff);
        }
    }

    if (arrived) {
        // If CPU is empty, start the process that just arrived
        if (state->current_process == NULL) {
            schedule_mlfq(state);
        } 
        // If someone is running, check if we need to preempt (optional but good)
        else {
            mlfq_check_preemption(state, sched);
        }
    }
}

// SIMULATOR ENGINE HELPERS

void initialize_events(SchedulerState *state, SchedulingAlgorithm algorithm) {
    if (!state) {
        return;
    }
    if (!state->processes) {
        return;
    }

    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];
        schedule_event(state, p, EVENT_ARRIVAL, p->arrival_time);
    }
    
    if (algorithm == MLFQ) {
        schedule_event(state, NULL, EVENT_PRIORITY_BOOST, state->mlfq.boost_period);
    }

    Event *tmp = state->event_queue;
    while (tmp) {
        tmp = tmp->next;
    }
} 

Event *pop_event(Event **event_queue)
{
    if (!event_queue || !*event_queue)
        return NULL;

    Event *front = *event_queue; // take the head (earliest event)
    *event_queue = front->next;  // move queue forward
    front->next = NULL;          // clean up dangling pointer
    return front;                // caller owns and frees this
}

// helper: define priority (lower = higher priority)
int get_priority(EventType t)
{
    switch (t)
    {
        case EVENT_ARRIVAL:         return 0; // HIGHEST
        case EVENT_PRIORITY_BOOST:  return 1;
        case EVENT_COMPLETION:      return 2;
        case EVENT_QUANTUM_EXPIRE:  return 3; // LOWEST
        default:                   return 4;
    }
}

void schedule_event(SchedulerState *state, Process *p, EventType type, int event_time)
{
    Event *event = malloc(sizeof(Event));
    if (!event) {
        fprintf(stderr, "Fatal: Event allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    event->time = event_time;
    event->type = type;
    event->process = p;
    event->next = NULL;

    // list is empty, new event becomes head
    if (state->event_queue == NULL)
    {
        state->event_queue = event;
        return;
    }

    // check if new event should go at the front
    if (event->time < state->event_queue->time ||
        (event->time == state->event_queue->time &&
         get_priority(type) < get_priority(state->event_queue->type)))
    {
        event->next = state->event_queue;
        state->event_queue = event;
        return;
    }

    Event *current = state->event_queue;

    while (current->next != NULL)
    {
        if (current->next->time > event->time)
            break;

        if (current->next->time == event->time &&
            get_priority(type) < get_priority(current->next->type))
            break;

        current = current->next;
    }
    // insert new event after current
    event->next = current->next;
    current->next = event;
}

void handle_completion(SchedulerState *state, Process *p)
{
    if (!p || !state) 
        return;

    p->finish_time = state->current_time;
    p->remaining_time = 0;

    state->current_process = NULL;
}

void handle_quantum_expire(SchedulerState *state, Process *p, SchedulingAlgorithm algorithm)
{
    if (!p) 
        return;

    if (algorithm != MLFQ)
        enqueue(&state->ready_queue, p);

}

void handle_priority_boost(SchedulerState *state)
{
    if (!state) 
        return;

    MLFQScheduler *sched = &state->mlfq;
    // perform boost
    mlfq_priority_boost(sched, state->current_time);
    
    // if (state->current_process) {
    //     state->current_process->priority = 0;
    //     state->current_process->time_in_queue = 0;
    //     enqueue_mlfq(&state->mlfq.queues[0], state->current_process);
    //     state->current_process = NULL; 
    // }

    if (state->current_process){
        Process *p = state -> current_process;
        p->priority = 0;
        p->time_in_queue = 0;
        enqueue_mlfq(&state->mlfq.queues[0], p);
        state->current_process = NULL;
    }

    // check for processes
    int processes_remaining = 0;
    for (int i = 0; i < state->num_processes; i++) {
        // A process is only "active" if it hasn't finished yet
        if (state->processes[i].finish_time == -1) {
            processes_remaining = 1;
            break;
        }
    }

    // only boost if there is still processes
    if (processes_remaining) {
        int next_time = state->current_time + state->mlfq.boost_period;
        schedule_event(state, NULL, EVENT_PRIORITY_BOOST, next_time);
    } else {
    }
}

void detect_convoy_effect(SchedulerState *state) {
    if (!state || state->num_processes == 0)
        return;

    // compute average waiting time
    int total_wait = 0;
    for (int i = 0; i < state->num_processes; i++)
        total_wait += state->processes[i].waiting_time;

    int convoy_found = 0;

    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];

        if (p->waiting_time > 0) {
            if (!convoy_found) {
                printf("Convoy effect detected:\n");
                convoy_found = 1;
            }
            printf("  Process %s waited %d time units\n", p->pid, p->waiting_time);
        }
    }
}

const char* get_algorithm_name(SchedulingAlgorithm algo)
{
    switch (algo)
    {
        case FCFS: return "FCFS";
        case SJF:  return "SJF";
        case STCF: return "STCF";
        case RR:   return "Round Robin";
        case MLFQ: return "MLFQ";
        default:   return "Unknown";
    }
}

void log_mlfq_event(int time, const char *msg)
{
    printf("t=%-4d: %s\n", time, msg);
}
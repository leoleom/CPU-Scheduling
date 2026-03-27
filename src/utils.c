#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

// INITIALIZATIONS
void init_scheduler(SchedulerState *state)
{

    // defensive ulit bro
    while (state->event_queue != NULL) {
        Event *temp = pop_event(&state->event_queue);
        free(temp);
    }

    state->ready_queue.head = NULL;
    state->ready_queue.tail = NULL;
    state->ready_queue.size = 0;
    state->current_process = NULL;
    state->gantt_chart = NULL;
    state->gantt_size = 0;
    state->current_time = 0;
    state->event_queue = NULL;

    for (int i = 0; i < state->num_processes; i++)
    {
        state->processes[i].remaining_time = state->processes[i].burst_time;
        state->processes[i].start_time = -1;
        state->processes[i].finish_time = -1;
        state->processes[i].waiting_time = 0;
        state->processes[i].turnaround_time = 0;
    }
}

void init_mlfq(MLFQScheduler *sched, MLFQConfig *config)
{

    // extra defense
    if (!sched || !config || !config->time_quantum || !config->allotment)
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
            heap_insert(heap, state->current_process, cmp_stcf);
            state->current_process = heap_extract_min(heap, cmp_stcf);
        }
    }
}

void handle_arrivals_sjf(SchedulerState *state, MinHeap *heap, int time)
{
    for (int i = 0; i < state->num_processes; i++)
    {
        Process *p = &state->processes[i];

        if (p->arrival_time == time)
        {
            heap_insert(heap, p, cmp_sjf); // uses burst_time
        }
    }
}

void handle_arrivals_mlfq(SchedulerState *state, MLFQScheduler *sched, int time)
{
    for (int i = 0; i < state->num_processes; i++)
    {
        Process *p = &state->processes[i];

        if (p->arrival_time == time)
        {
            p->priority = 0;
            p->time_in_queue = 0;
            enqueue_mlfq(&sched->queues[0], p);
        }
    }

    // boost priorities if needed
    mlfq_priority_boost(sched, time);

    // preempt if higher-priority process exists
    mlfq_check_preemption(state, sched);

    // pick next process to run
    mlfq_select_next_process(state, sched);
}

// SIMULATOR ENGINE HELPERS

void initialize_events(SchedulerState *state, SchedulingAlgorithm algorithm) {
    if (!state || !state->processes) return;

    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];
        schedule_event(state, p, EVENT_ARRIVAL, p->arrival_time);
    }
    
    // for mlfq implementations 
    if (algorithm == MLFQ) {
        schedule_event(state, NULL, EVENT_PRIORITY_BOOST, state->mlfq.boost_period);
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
    if (event->time < state->event_queue->time)
    {
        event->next = state->event_queue;
        state->event_queue = event;
        return;
    }

    Event *current = state->event_queue;
    while (current->next != NULL && current->next->time <= event->time)
    {
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

    if (algorithm == MLFQ) {
        enqueue_mlfq(&state->mlfq.queues[p->priority], p);
    } else {
        enqueue(&state->ready_queue, p);
    }
}

void handle_priority_boost(SchedulerState *state)
{
    // perform the priority boost
    mlfq_priority_boost(&state->mlfq, state->current_time);

    // preempt current process if a higher-priority process exists
    mlfq_check_preemption(state, &state->mlfq);

    // pick next process to run if CPU is idle
    mlfq_select_next_process(state, &state->mlfq);

    // schedule the next priority boost event
    schedule_event(state, NULL, EVENT_PRIORITY_BOOST,
                   state->current_time + state->mlfq.boost_period);
}
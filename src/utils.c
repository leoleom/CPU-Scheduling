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

    printf("[DEBUG] init_scheduler: starting\n");

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
        printf("[DEBUG] init_scheduler: Process %s initialized, AT=%d, BT=%d\n",
               state->processes[i].pid,
               state->processes[i].arrival_time,
               state->processes[i].burst_time);
    }
        printf("[DEBUG] init_scheduler: complete\n");
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
    printf("[DEBUG] enqueue: Process %s added to queue, new size=%d\n", proc->pid, queue->size);
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

    printf("[DEBUG] dequeue: Process %s removed from queue, new size=%d\n", temp->process->pid, queue->size);
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
            int delta = state->current_time - state->last_event_time;
            state->current_process->remaining_time -= delta;

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

        printf("[DEBUG] Checking process %s: AT=%d, start_time=%d, current_time=%d\n",
       p->pid, p->arrival_time, p->start_time, time);
        if (p->arrival_time <= time && p->start_time == -1)
        {
            printf("[DEBUG] handle_arrivals_sjf: heap size before=%d\n", heap->size);
            heap_insert(heap, p, cmp_sjf); // uses burst_time
            printf("[DEBUG] handle_arrivals_sjf: inserted %s, heap size after=%d\n", p->pid, heap->size);
            //p -> start_time = -2; // mark as added to heap
            printf("[DEBUG] handle_arrivals_sjf: Process %s added to heap at time %d\n", p->pid, time);
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

    // preempt if higher-priority process exists
    mlfq_check_preemption(state, sched);

    // pick next process to run
    mlfq_select_next_process(state, sched);
}

// SIMULATOR ENGINE HELPERS

void initialize_events(SchedulerState *state, SchedulingAlgorithm algorithm) {
    if (!state) {
        printf("[DEBUG] initialize_events: state is NULL\n");
        return;
    }
    if (!state->processes) {
        printf("[DEBUG] initialize_events: state->processes is NULL\n");
        return;
    }

    printf("[DEBUG] initialize_events: num_processes=%d\n", state->num_processes);

    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];
        printf("[DEBUG] Scheduling arrival for process %s at time %d\n", p->pid, p->arrival_time);
        schedule_event(state, p, EVENT_ARRIVAL, p->arrival_time);
    }
    
    if (algorithm == MLFQ) {
        schedule_event(state, NULL, EVENT_PRIORITY_BOOST, state->mlfq.boost_period);
    }

    printf("[DEBUG] Event queue initialized\n");
    Event *tmp = state->event_queue;
    while (tmp) {
        printf("  Event type=%d time=%d process=%s\n", tmp->type, tmp->time, tmp->process->pid);
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

     printf("[DEBUG] schedule_event: Event type=%d for process %s at time=%d\n",
           type, p ? p->pid : "-", event_time);
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
    printf("[DEBUG] handle_completion: Process %s finished at time=%d\n", p->pid, state->current_time);
}

void handle_quantum_expire(SchedulerState *state, Process *p, SchedulingAlgorithm algorithm)
{
    if (!p) 
        return;

    // update internal clock boss ass b****
    int q = (algorithm == MLFQ) ? state->mlfq.queues[p->priority].time_quantum : state->rr_quantum;
    
    p->remaining_time -= q; 
    if (algorithm == MLFQ) {
        p->time_in_queue += q;
    }

    if (algorithm != MLFQ)
        enqueue(&state->ready_queue, p);

    printf("[DEBUG] handle_quantum_expire: Process %s remaining_time=%d\n", p->pid, p->remaining_time);
}

void handle_priority_boost(SchedulerState *state)
{
    printf("[DEBUG] handle_priority_boost: boosting at time=%d\n", state->current_time);
    if (state->current_process != NULL) {
        state->current_process->priority = 0;
        state->current_process->time_in_queue = 0;
    }

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

void detect_convoy_effect(SchedulerState *state) {
    if (!state || state->num_processes == 0)
        return;

    // compute average waiting time
    int total_wait = 0;
    for (int i = 0; i < state->num_processes; i++)
        total_wait += state->processes[i].waiting_time;

    float avg_wait = (float)total_wait / state->num_processes;

    int convoy_found = 0;

    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];

        // threshold: any process waiting longer than average
        if (p->waiting_time > avg_wait) {
            if (!convoy_found) {
                printf("Convoy effect detected:\n");
                convoy_found = 1;
            }
            printf("  Process %s waited %d time units\n", p->pid, p->waiting_time);
        }
    }
}
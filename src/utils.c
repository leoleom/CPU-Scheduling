#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

// INITIALIZATIONS
void init_scheduler(SchedulerState *state)
{
    state->ready_queue.head = NULL;
    state->ready_queue.tail = NULL;
    state->ready_queue.size = 0;
    state->current_process = NULL;
    state->gantt_chart = NULL;
    state->gantt_size = 0;
    state->event_queue = NULL;
    

    for (int i = 0; i < state->num_processes; i++)
    {
        state->processes[i].remaining_time = state->processes[i].burst_time;
        state->processes[i].start_time = -1;
        state->processes[i].finish_time = -1;
    }
}

void init_mlfq(MLFQScheduler *sched, MLFQConfig *config) {
    sched->num_queues = config->queues;
    sched->boost_period = config->boost_period;
    sched->last_boost = 0;

    sched->queues = malloc(sizeof(MLFQQueue) * config->queues);

    for (int i = 0; i < config->queues; i++) {
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
    Node *new_node = (Node *)malloc(sizeof(Node));
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

        if (p->start_time == -1)
            p->start_time = time;
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

        if (p->start_time == -1)
            p->start_time = time;
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
        
        if (p->start_time == -1)
            p->start_time = time;
    }
}

void handle_arrivals_mlfq(SchedulerState *state, MLFQScheduler *sched, int time) {
    for (int i = 0; i < state->num_processes; i++) {
        Process *p = &state->processes[i];

        if (p->arrival_time == time) {
            p->priority = 0;
            p->time_in_queue = 0;
            enqueue(&sched->queues[0], p);
            
        }

        if (p->start_time == -1)
            p->start_time = time;
    }

    // boost priorities if needed
    mlfq_priority_boost(sched, time);

    // preempt if higher-priority process exists
    mlfq_check_preemption(state, sched);

    // pick next process to run
    mlfq_select_next_process(state, sched);
}

// SIMULATOR ENGINE HELPERS

Event *pop_event(Event **event_queue)
{
    if (!event_queue || !*event_queue)
        return NULL;

    Event *front = *event_queue;       // take the head (earliest event)
    *event_queue = front->next;        // move queue forward
    front->next = NULL;                // clean up dangling pointer
    return front;                      // caller owns and frees this
}


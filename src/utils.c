#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>


void init_scheduler(SchedulerState *state) {
    state->ready_queue.head = NULL;
    state->ready_queue.tail = NULL;
    state->ready_queue.size = 0;
    state->current_process = NULL;
    state->gantt_chart = NULL;
    state->gantt_size = 0;

    for (int i = 0; i < state->num_processes; i++) {
        state->processes[i].remaining_time = state->processes[i].burst_time;
        state->processes[i].start_time = -1;
        state->processes[i].finish_time = -1;
    }
}

/* used for all algorithms */
void enqueue(Queue *queue, Process *proc) {
    Node *new_node = (Node*) malloc(sizeof(Node));
    new_node->process = proc;
    new_node->next = NULL;

    if (queue->tail == NULL) {  // empty queue
        queue->head = queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    queue->size++;
}

Node* dequeue(Queue *queue) {
    if (queue->head == NULL)        //check if empty
        return NULL;

    Node *temp = queue->head;
    queue->head = queue->head->next;   //move the head to the next process

    if (queue->head == NULL)           //if queue is now empty
        queue->tail = NULL;

    temp->next = NULL;                //delete the reference
    queue->size--;

    return temp;
}


//make min-heap here
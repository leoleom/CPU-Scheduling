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
void enqueue(MLFQQueue *queue, Process *proc) {
    proc->next = NULL;

    //check if empty
    if (queue->tail == NULL) {
        queue->head = queue->tail = proc;
    } else {
        queue->tail->next = proc;       //link last process to new one
        queue->tail = proc;             //update the tail
    }

    queue->size++;
}

Process* dequeue(MLFQQueue *queue) {
    if (queue->head == NULL)        //check if empty
        return NULL;

    Process *proc = queue->head;       //save the process
    queue->head = queue->head->next;   //move the head to the next process

    if (queue->head == NULL)           //if queue is now empty
        queue->tail = NULL;

    proc->next = NULL;                //delete the reference
    queue->size--;

    return proc;
}

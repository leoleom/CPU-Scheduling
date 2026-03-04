#include "mlfq.h"
#include <stdlib.h>
#include <stdio.h>

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

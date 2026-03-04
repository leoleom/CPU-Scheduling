#include "scheduler.h"

void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p) {
    MLFQQueue *current_queue = &scheduler->queues[p->priority];
    
    // Check if process exhausted its allotment
    if (p->time_in_queue >= current_queue->allotment) {
        // Demote to lower priority
        if (p->priority < scheduler->num_queues - 1) {
            remove_from_queue(current_queue, p);
            p->priority++;
            p->time_in_queue = 0;  // Reset allotment
            add_to_queue(&scheduler->queues[p->priority], p);
        }
    }
}

void mlfq_priority_boost(MLFQScheduler *scheduler, int current_time) {
    if (current_time - scheduler->last_boost >= scheduler->boost_period) {
        // Move all processes to highest priority
        for (int i = 1; i < scheduler->num_queues; i++) {
            MLFQQueue *queue = &scheduler->queues[i];
            while (queue->size > 0) {
                Process *p = dequeue(queue);
                p->priority = 0;
                p->time_in_queue = 0;
                enqueue(&scheduler->queues[0], p);
            }
        }
        scheduler->last_boost = current_time;
    }
}
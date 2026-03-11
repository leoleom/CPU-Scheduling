#ifndef MLFQ_H
#define MLFQ_H

#include "process.h"

typedef struct {
    int level;              // Queue priority level (0 = highest)
    int time_quantum;       // Time slice for this queue (-1 for FCFS)
    int allotment;          // Max time before demotion (-1 for infinite)
    Node *head;          // head of linked list
    Node *tail;          // tail of linked list
    int size;               // Current queue size
} MLFQQueue;

typedef struct {
    MLFQQueue *queues;      // Array of queues
    int num_queues;         // Number of priority levels
    int boost_period;       // Period for priority boost (S)
    int last_boost;         // Last boost time
} MLFQScheduler;


// static rules
typedef struct {
    int num_queues;         
    int *time_quantum;      
    int *allotment;         
    int boost_period;       
} MLFQConfig;

void enqueue(MLFQQueue *queue, Process *proc);
Process* dequeue(MLFQQueue *queue);

void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p);
void mlfq_priority_boost(MLFQScheduler *scheduler, int current_time);

#endif
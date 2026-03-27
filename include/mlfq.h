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
    int queues;         
    int time_quantum[3];      
    int allotment[3];         
    int boost_period;       
} MLFQConfig;

void enqueue_mlfq(MLFQQueue *q, Process *p);
Node *dequeue_mlfq(MLFQQueue *q);

int schedule_mlfq(SchedulerState *state, MLFQConfig *config);
void mlfq_check_preemption(SchedulerState *state, MLFQScheduler *sched);
void mlfq_select_next_process(SchedulerState *state, MLFQScheduler *sched);

void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p);
void mlfq_priority_boost(MLFQScheduler *scheduler, int current_time);

#endif
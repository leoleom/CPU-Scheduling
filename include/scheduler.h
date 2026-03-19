#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <stdlib.h>


typedef struct Node {
    Process *process;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int size;
} Queue;

#include "mlfq.h"

typedef struct {
    Process *processes;     // Array of all processes
    int num_processes;      // Number of processes
    int current_time;       // Current simulation time

    Queue ready_queue;
    Process *current_process;  // current running process
    char *gantt_chart;       
    int gantt_size;          

    //store metrics
} SchedulerState;

typedef enum {
    EVENT_ARRIVAL,
    EVENT_COMPLETION,
    EVENT_QUANTUM_EXPIRE,
    EVENT_PRIORITY_BOOST
} EventType;

typedef struct Event {
    int time;
    EventType type;
    Process *process;
    struct Event *next;
} Event;

typedef enum {
    FCFS,
    SJF,
    STCF,
    RR,
    MLFQ
} SchedulingAlgorithm;

void init_scheduler(SchedulerState *state);

void enqueue(Queue *queue, Process *proc);
Node* dequeue(Queue *queue);

// Return 0 on success, -1 on error (command line etiquette)
int schedule_fcfs(SchedulerState *state);
int schedule_sjf(SchedulerState *state);
int schedule_stcf(SchedulerState *state);
int schedule_rr(SchedulerState *state, int quantum);
int schedule_mlfq(SchedulerState *state, MLFQConfig *config);

#endif

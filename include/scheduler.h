#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "heap.h"
#include <stdlib.h>

typedef struct Node
{
    Process *process;
    struct Node *next;
} Node;

#include "mlfq.h"

typedef struct
{
    Node *head;
    Node *tail;
    int size;
} Queue;

// scheduling algo used in event
typedef enum {
    FCFS = 1,
    SJF,
    STCF,
    RR,
    MLFQ
} SchedulingAlgorithm;

typedef enum
{
    EVENT_ARRIVAL,
    EVENT_COMPLETION,
    EVENT_QUANTUM_EXPIRE,
    EVENT_PRIORITY_BOOST
} EventType;

typedef struct Event
{
    int time;
    EventType type;
    Process *process;
    struct Event *next;
} Event;

typedef struct SchedulerState
{
    Process *processes; // Array of all processes
    int num_processes;  // Number of processes
    int current_time;   // Current simulation time

    Queue ready_queue;
    Process *current_process; // current running process
    char *gantt_chart;
    int gantt_size;

    int rr_quantum;

    MinHeap heap;

    MLFQScheduler mlfq;

    Event *event_queue;
    int last_event_time;
    
    // store metrics
} SchedulerState;


// initializations
void init_scheduler(SchedulerState *state);
void init_mlfq(MLFQScheduler *sched, MLFQConfig *config);

// simulation engine
int simulate_scheduler(SchedulerState *state, SchedulingAlgorithm algorithm);
void schedule_event(SchedulerState *state, Process *p, EventType type, int event_time);

// handling event helpers
void handle_completion(SchedulerState *state, Process *p);
void handle_quantum_expire(SchedulerState *state, Process *p, SchedulingAlgorithm algorithm);
void handle_priority_boost(SchedulerState *state);
void initialize_events(SchedulerState *state, SchedulingAlgorithm algorithm);
Event *pop_event(Event **event_queue);

// enqueue for fcfs and rr
void enqueue(Queue *queue, Process *proc);
Node *dequeue(Queue *queue);

// fcfs and rr
void handle_arrivals_queue(SchedulerState *state, int current_time);
// sjf scheduler
void handle_arrivals_sjf(SchedulerState *state, MinHeap *heap, int current_time);
// stcf scheduler
void handle_arrivals_stcf(SchedulerState *state, MinHeap *heap, int current_time);
// mlfq scheduler
void handle_arrivals_mlfq(SchedulerState *state, MLFQScheduler *sched, int current_time);

// Return 0 on success, -1 on error (command line etiquette)
int schedule_fcfs(SchedulerState *state);
int schedule_sjf(SchedulerState *state, MinHeap *heap);
int schedule_stcf(SchedulerState *state, MinHeap *heap);
int schedule_rr(SchedulerState *state, int quantum);
int schedule_mlfq(SchedulerState *state);

#endif

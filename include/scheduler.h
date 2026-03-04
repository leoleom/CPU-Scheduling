#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "mlfq.h"

typedef struct {
    Process *processes;     // Array of all processes
    int num_processes;      // Number of processes
    int current_time;       // Current simulation time

    MLFQQueue ready_queue;   //for fcfs and rr
    Process *current_process; 

    char *gantt_chart;       
    int gantt_size;          
} SchedulerState;

// Return 0 on success, -1 on error (command line etiquette)
int schedule_fcfs(SchedulerState *state);
int schedule_sjf(SchedulerState *state);
int schedule_stcf(SchedulerState *state);
int schedule_rr(SchedulerState *state, int quantum);
int schedule_mlfq(SchedulerState *state, MLFQConfig *config);

#endif
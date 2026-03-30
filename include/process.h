#ifndef PROCESS_H
#define PROCESS_H


typedef struct {
    char pid[16];
    int arrival_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int finish_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int priority;        // For MLFQ
    int time_in_queue;   // For MLFQ allotment tracking
    int was_preempted;  
    int last_start_time; // For accurate remaining time calculation on preemption
    
} Process;

int load_processes(const char *filename, Process **processes);
int load_command(const char *str, Process **processes);


#endif
#ifndef METRICS_H
#define METRICS_H

#include "process.h"

void calculate_metrics(Process *processes, int n);
double calculate_average_turnaround(Process *processes, int n);
double calculate_average_waiting(Process *processes, int n);
double calculate_average_response(Process *processes, int n);
void print_process_metrics(Process p[], int n);
void print_metrics_calculation(Process p[], int n);

#endif


#ifndef METRICS_H
#define METRICS_H

#include "process.h"

void calculate_metrics(Process *processes, int n);
double calculate_average_turnaround(Process *processes, int n);

#endif


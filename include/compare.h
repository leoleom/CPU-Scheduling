#ifndef COMPARE_H
#define COMPARE_H

#include "scheduler.h"

typedef struct {
    double avg_tt;
    double avg_wt;
    double avg_rt;
    int context_switches;
} Result;

Result run_algorithm(char *input, SchedulingAlgorithm algorithm, int quantum);

#endif
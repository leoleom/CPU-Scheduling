#include <stdio.h>
#include <stdlib.h>
#include "process.h"

int load_processes(const char *filename, Process **p) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(1);
    }

    Process *list = malloc(sizeof(Process) * 10);  // max 10 processes for now
    int count = 0;

    while (fscanf(fp, " %s %d %d",
                  list[count].pid,
                  &list[count].arrival_time,
                  &list[count].burst_time) == 3) {
        count++;
    }

    fclose(fp);

    *p = list;
    return count;
}
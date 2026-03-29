#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

int load_processes(const char *filename, Process **p) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(1);
    }

    Process *list = malloc(sizeof(Process) * 20);  // max 20 processes for now
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

int load_command(const char *str, Process **p)
{
    char *copy = strdup(str);
    char *token = strtok(copy, ",");

    Process *list = malloc(sizeof(Process) * 20); // adjust if needed
    int count = 0;

    while (token != NULL)
    {
        char pid[10];
        int at, bt;

        if (sscanf(token, "%[^:]:%d:%d", pid, &at, &bt) == 3)
        {
            strcpy(list[count].pid, pid);
            list[count].arrival_time = at;
            list[count].burst_time = bt;
            count++;
        }
        else
        {
            fprintf(stderr, "Invalid process format: %s\n", token);
        }

        token = strtok(NULL, ",");
    }

    free(copy);
    *p = list;
    return count;
}
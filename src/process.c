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

    int capacity = 4;
    int count = 0;
    Process *list = malloc(sizeof(Process) * capacity);
    if (!list) {
        perror("Malloc failed");
        exit(1);
    }

    while (fscanf(fp, " %s %d %d",
                  list[count].pid,
                  &list[count].arrival_time,
                  &list[count].burst_time) == 3)
    {
        // resize if needed
        if (count >= capacity - 1) {
            capacity *= 2;
            Process *temp = realloc(list, sizeof(Process) * capacity); // doubles capacity
            if (!temp) {
                perror("Reallocation failed");
                free(list);
                exit(1);
            }
            list = temp;
        }

        count++;
    }

    fclose(fp);
    *p = list;
    return count;
}

int load_command(const char *str, Process **p)
{
    char *copy = strdup(str);
    if (!copy) {
        perror("strdup failed");
        exit(1);
    }

    int capacity = 4;
    int count = 0;
    Process *list = malloc(sizeof(Process) * capacity);
    if (!list) {
        perror("Malloc failed");
        free(copy);
        exit(1);
    }

    char *token = strtok(copy, ",");

    while (token != NULL)
    {
        if (count >= capacity - 1)
        {
            capacity *= 2;
            Process *temp = realloc(list, sizeof(Process) * capacity); // doubles capacity
            if (!temp) {
                perror("Reallocation failed");
                free(list);
                free(copy);
                exit(1);
            }
            list = temp;
        }

        char pid[16];
        int at, bt;

        if (sscanf(token, "%[^:]:%d:%d", pid, &at, &bt) == 3)
        {
            // safe copy
            strncpy(list[count].pid, pid, sizeof(list[count].pid) - 1);
            list[count].pid[sizeof(list[count].pid) - 1] = '\0';

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
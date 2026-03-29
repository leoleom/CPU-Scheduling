#include <stdio.h>
#include <stdlib.h>
#include "gantt.h"

static char *gantt;
static int max_size = 0;

void gantt_init(int size) { // call before scheduling
    if (size <= 0)
    {
        fprintf(stderr, "Error: Invalid gantt size.\n");
        exit(1);
    }

    max_size = size;
    gantt = (char *)malloc(sizeof(char) * max_size);

    if (!gantt)
    {
        fprintf(stderr, "Error: Failed to allocate gantt chart.\n");
        exit(1);
    }

    for (int i = 0; i < max_size; i++)
        gantt[i] = '-'; // idle
}

void gantt_add(int time, char pid) {
    if (!gantt || time < 0 || time >= max_size) return;
    gantt[time] = pid;
}

void gantt_print(int n)
{
    if (!gantt || n <= 0)
        return;

    int scale = (n > 50) ? (n + 49) / 50 : 1;

    printf("\n=== GANTT CHART ===\n");
    if (scale > 1)
        printf("Scale: 1 block = %d time units\n\n", scale);

    int i = 0;

    //poper alignment for time markers
    int *positions = malloc(sizeof(int) * n); //store position of [
    int *times = malloc(sizeof(int) * n); //store time values
    int pos_count = 0;

    int opening_pos = 0; // [ position tracker
    int time = 0;

    if (!positions || !times) {
    printf("Memory allocation failed\n");
    return;
    }

    // TOP BAR
    while (i < n)
    {
        char current = gantt[i];
        int count = 0;

        while (i < n && gantt[i] == current)
        {
            i++;
            count++;
        }

        int blocks = (count + scale - 1) / scale;

        // record [ position
        positions[pos_count] = opening_pos;
        times[pos_count] = time;
        pos_count++;

        printf("[");
        opening_pos++;
        for (int j = 0; j < blocks; j++)
        {
            printf("%c", current);
            opening_pos++;
        }
        printf("]");
        opening_pos++;  
        time += count;
    }

    // last time marker
    positions[pos_count] = opening_pos;
    times[pos_count] = time;
    pos_count++;

    printf("\n");

    // TIME MARKERS
    int printed_time = 0;

    for (int p = 0; p < pos_count; p++)
    {
        int target = positions[p];

        // print spaces until the position of the time marker
        while (printed_time < target)
        {
            printf(" ");
            printed_time++;
        }

        printf("%d", times[p]);
        printed_time += snprintf(NULL, 0, "%d", times[p]); // move printed_time forward by the number of digits
    }

    printf("\n");
}

void gantt_free()
{
    if (gantt)
    {
        free(gantt);
        gantt = NULL;
    }
}
#include <stdio.h>
#include "gantt.h"

static char gantt[10000]; // temporary fixed size
static int max_size = 10000;

void gantt_init(int size) { // call before scheduling
    if (size < max_size)
        max_size = size;

    for (int i = 0; i < max_size; i++)
        gantt[i] = '-';   // idle marker
}

void gantt_add(int time, char pid) {
    if (time < 0 || time >= max_size) return;
    gantt[time] = pid;
}

void gantt_print(int n)
{
    int i, j;

    if (n <= 50)
    {
        i = 0;
        while (i < n)
        {
            char current = gantt[i];

            printf("[");

            while (i < n && gantt[i] == current)
            {
                printf("%c", current);
                i++;
            }

            printf("]");
        }

        printf("\n");

        i = 0;
        int time = 0;

        printf("0");

        while (i < n)
        {
            char current = gantt[i];
            int count = 0;

            while (i < n && gantt[i] == current)
            {
                i++;
                count++;
            }

            time += count;

            printf("     %d", time);
        }

        printf("\n");
        return;
    }

    int scale = n / 50;
    if (scale < 1) scale = 1;

    printf("Scale: 1 block = %d time units\n\n", scale);

    //scaled
    i = 0;
    int time = 0;

    while (i < n)
    {
        char current = gantt[i];
        int count = 0;

        while (i < n && gantt[i] == current)
        {
            i++;
            count++;
        }

        if (count == 0)
            continue;

        printf("[");

        int blocks = (count + scale - 1) / scale;

        for (j = 0; j < blocks; j++)
            printf("%c", current);

        printf("]");

        time += count;
    }

    printf("\n");

    i = 0;
    time = 0;

    printf("0");

    while (i < n)
    {
        char current = gantt[i];
        int count = 0;

        while (i < n && gantt[i] == current)
        {
            i++;
            count++;
        }

        time += count;

        printf("     %d", time);
    }

    printf("\n");
}
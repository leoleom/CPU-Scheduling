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

// void gantt_print(int n)
// {
//      if (!gantt || n <= 0)
//         return;
//     int i, j;

//     if (n <= 50)
//     {
//         i = 0;
//         while (i < n)
//         {
//             char current = gantt[i];

//             printf("[");

//             while (i < n && gantt[i] == current)
//             {
//                 printf("%c", current);
//                 i++;
//             }

//             printf("]");
//         }

//         printf("\n");

//         i = 0;
//         int time = 0;

//         printf("0");

//         while (i < n)
//         {
//             char current = gantt[i];
//             int count = 0;

//             while (i < n && gantt[i] == current)
//             {
//                 i++;
//                 count++;
//             }

//             time += count;

//             printf("     %d", time);
//         }

//         printf("\n");
//         return;
//     }

//     int scale = n / 50;
//     if (scale < 1) scale = 1;

//     printf("Scale: 1 block = %d time units\n\n", scale);

//     //scaled
//     i = 0;
//     int time = 0;

//     while (i < n)
//     {
//         char current = gantt[i];
//         int count = 0;

//         while (i < n && gantt[i] == current)
//         {
//             i++;
//             count++;
//         }

//         if (count == 0)
//             continue;

//         printf("[");

//         int blocks = (count + scale - 1) / scale;

//         for (j = 0; j < blocks; j++)
//             printf("%c", current);

//         printf("]");

//         time += count;
//     }

//     printf("\n");

//     i = 0;
//     time = 0;

//     printf("0");

//     while (i < n)
//     {
//         char current = gantt[i];
//         int count = 0;

//         while (i < n && gantt[i] == current)
//         {
//             i++;
//             count++;
//         }

//         time += count;

//         printf("     %d", time);
//     }

//     printf("\n");
// }
void gantt_print(int n)
{
    if (!gantt || n <= 0)
        return;

    int scale = (n > 50) ? (n + 49) / 50 : 1;

    if (scale > 1)
        printf("Scale: 1 block = %d time units\n\n", scale);

    int i = 0;

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

        printf("[");
        for (int j = 0; j < blocks; j++)
            printf("%c", current);
        printf("]");
    }

    printf("\n");

    // TIME MARKERS
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
}

void gantt_free()
{
    if (gantt)
    {
        free(gantt);
        gantt = NULL;
    }
}
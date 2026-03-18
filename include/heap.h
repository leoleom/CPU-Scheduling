#ifndef HEAP_H
#define HEAP_H

#include "process.h"  

typedef struct {
    Process **process;  // array of Process structs
    int size;           // current number of elements
    int capacity;       // max allocated size
} MinHeap;

// create a new heap
MinHeap* create_heap(int capacity);

// heap utils
void heap_insert(MinHeap *heap, Process *proc, int (*cmp)(Process, Process));
Process* heap_extract_min(MinHeap *heap, int (*cmp)(Process, Process));
Process* heap_peek(MinHeap *heap);

// freedom
void free_heap(MinHeap *heap);

// compare functions
int cmp_sjf(Process a, Process b);   // compare burst_time
int cmp_stcf(Process a, Process b);  // compare remaining_time

#endif
#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

static void swap(Process **a, Process **b) {
    Process *temp = *a;
    *a = *b;
    *b = temp;
}

static void heapify_up(MinHeap *heap, int index, int (*cmp)(Process*, Process*)) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (cmp(heap->process[index], heap->process[parent]) < 0) {
            swap(&heap->process[index], &heap->process[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

static void heapify_down(MinHeap *heap, int index, int (*cmp)(Process*, Process*)) {
    int left, right, smallest;

    while (1) {
        left = 2 * index + 1;
        right = 2 * index + 2;
        smallest = index;

        if (left < heap->size && cmp(heap->process[left], heap->process[smallest]) < 0)
            smallest = left;
        if (right < heap->size && cmp(heap->process[right], heap->process[smallest]) < 0)
            smallest = right;

        if (smallest != index) {
            swap(&heap->process[index], &heap->process[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

// create a new heap
MinHeap* create_heap(int capacity) {
    MinHeap *heap = malloc(sizeof(MinHeap));
    heap->process = malloc(capacity * sizeof(Process*));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// insert a process
void heap_insert(MinHeap *heap, Process *proc, int (*cmp)(Process*, Process*)) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->process = realloc(heap->process, heap->capacity * sizeof(Process*));
    }

    heap->process[heap->size] = proc;
    heapify_up(heap, heap->size, cmp);
    heap->size++;
}

// extract min process
Process* heap_extract_min(MinHeap *heap, int (*cmp)(Process *, Process*)) {
    if (heap->size == 0) return NULL;  // empty heap

    Process *min_proc = heap->process[0];
    heap->process[0] = heap->process[heap->size - 1];
    heap->size--;
    heapify_down(heap, 0, cmp);

    return min_proc;
}

// peek only, don't remove
Process* heap_peek(MinHeap *heap) {
    if (heap->size == 0) return NULL;
    return heap->process[0];
}



// free heap memory
void free_heap(MinHeap *heap) {
    if (heap) {
        free(heap->process);
        free(heap);
    }
}

// compare
int cmp_sjf(Process *a, Process *b) {
    return a->burst_time - b->burst_time;
}

int cmp_stcf(Process *a, Process *b) {
    return a->remaining_time - b->remaining_time;
}
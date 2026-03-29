#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

static void swap(Process **a, Process **b)
{
    Process *temp = *a;
    *a = *b;
    *b = temp;
    printf("[DEBUG] swap: %c <-> %c\n", (*a)->pid[0], (*b)->pid[0]);
}

static void heapify_up(MinHeap *heap, int index, int (*cmp)(Process *, Process *))
{
    while (index > 0)
    {
        int parent = (index - 1) / 2;
        if (cmp(heap->process[index], heap->process[parent]) < 0)
        {
            printf("[DEBUG] heapify_up: swapping %c with parent %c\n",
                   heap->process[index]->pid[0], heap->process[parent]->pid[0]);
            swap(&heap->process[index], &heap->process[parent]);
            index = parent;
        }
        else
        {
            break;
        }
    }
}

static void heapify_down(MinHeap *heap, int index, int (*cmp)(Process *, Process *))
{
    int left, right, smallest;

    while (1)
    {
        left = 2 * index + 1;
        right = 2 * index + 2;
        smallest = index;

        if (left < heap->size && cmp(heap->process[left], heap->process[smallest]) < 0)
            smallest = left;
        if (right < heap->size && cmp(heap->process[right], heap->process[smallest]) < 0)
            smallest = right;

        if (smallest != index)
        {
            printf("[DEBUG] heapify_down: swapping %c with child %c\n",
                   heap->process[index]->pid[0], heap->process[smallest]->pid[0]);
            swap(&heap->process[index], &heap->process[smallest]);
            index = smallest;
        }
        else
        {
            break;
        }
    }
}

// create a new heap
MinHeap *create_heap(int capacity)
{
    if (capacity <= 0) {
        fprintf(stderr, "Error: Invalid heap capacity %d.\n", capacity);
        return NULL; 
    }

    MinHeap *heap = malloc(sizeof(MinHeap));
    if (!heap) {
        perror("Failed to allocate MinHeap struct.");
        return NULL;
    }

    heap->process = malloc(capacity * sizeof(Process *));
    if (!heap->process) {
        perror("Failed to allocate heap process array.");
        free(heap); 
        return NULL;
    }

    heap->size = 0;
    heap->capacity = capacity;

    printf("[DEBUG] create_heap: new heap created with capacity %d\n", capacity);
    return heap;
}

// insert a process
void heap_insert(MinHeap *heap, Process *proc, int (*cmp)(Process *, Process *))
{
    if (!heap || !proc) return;
    
    if (heap->size >= heap->capacity)
    {
        int new_capacity = heap->capacity * 2;
        Process **temp = realloc(heap->process, new_capacity * sizeof(Process *));

        if (temp == NULL) {
            fprintf(stderr, "Fatal: Heap expansion failed (Out of Memory)\n");
            return; 
        }

        heap->capacity = new_capacity;
        heap->process = temp;
        printf("[DEBUG] heap_insert: heap expanded to capacity %d\n", new_capacity);
    }

    heap->process[heap->size] = proc;
    heapify_up(heap, heap->size, cmp);
    heap->size++;

    printf("[DEBUG] heap_insert: heap size now %d\n", heap->size);
}

// extract min process
Process *heap_extract_min(MinHeap *heap, int (*cmp)(Process *, Process *))
{
    if (!heap || heap->size == 0)
        return NULL; // empty heap

    Process *min_proc = heap->process[0];
    printf("[DEBUG] heap_extract_min: removing %c from top\n", min_proc->pid[0]);

    heap->process[0] = heap->process[heap->size - 1];
    heap->size--;

    // only check if there are still items
    if (heap->size > 0)
    {
        heapify_down(heap, 0, cmp);
    }

    return min_proc;
}

// peek only, don't remove
Process *heap_peek(MinHeap *heap)
{
    if (!heap || heap->size == 0)
        return NULL;
    printf("[DEBUG] heap_peek: top is %c\n", heap->process[0]->pid[0]);
    return heap->process[0];
}

// free heap memory
void free_heap(MinHeap *heap)
{
    if (heap)
    {
        free(heap->process);
        free(heap);
        printf("[DEBUG] free_heap: heap memory freed\n");
    }
}

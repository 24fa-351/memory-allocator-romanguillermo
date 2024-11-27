#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "malloc.h"

// Structure for a free block
typedef struct free_block {
    size_t size;
    struct free_block* next;
} free_block_t;

// Heap metadata
static void* heap_start = NULL;
static size_t heap_size = 0;

// Define alignment to 8 bytes
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Define the maximum number of free blocks (adjust as needed)
#define MAX_FREE_BLOCKS 1024

// Min-heap
typedef struct min_heap {
    free_block_t* blocks[MAX_FREE_BLOCKS];
    int size;
} min_heap_t;

static min_heap_t free_heap = {.size = 0};

// Helper function to get a block of memory
void* get_me_blocks(ssize_t how_much) {
    void* ptr = sbrk(0);
    void* result = sbrk(how_much);
    if (result == (void*) -1) {
        return NULL;
    }
    return ptr;
}

// Swap two elements in the heap
static void swap(free_block_t** a, free_block_t** b) {
    free_block_t* temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify up to maintain min-heap property
static void heapify_up(int index) {
    if (index == 0) return;
    int parent = (index - 1) / 2;
    if (free_heap.blocks[parent]->size > free_heap.blocks[index]->size) {
        swap(&free_heap.blocks[parent], &free_heap.blocks[index]);
        heapify_up(parent);
    }
}

// Heapify down to maintain min-heap property
static void heapify_down(int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < free_heap.size && free_heap.blocks[left]->size < free_heap.blocks[smallest]->size) {
        smallest = left;
    }
    if (right < free_heap.size && free_heap.blocks[right]->size < free_heap.blocks[smallest]->size) {
        smallest = right;
    }
    if (smallest != index) {
        swap(&free_heap.blocks[smallest], &free_heap.blocks[index]);
        heapify_down(smallest);
    }
}

// Insert a block into the min-heap
static void heap_insert(free_block_t* block) {
    if (free_heap.size >= MAX_FREE_BLOCKS) {
        fprintf(stderr, "Error: Free heap overflow.\n");
        return;
    }
    free_heap.blocks[free_heap.size] = block;
    heapify_up(free_heap.size);
    free_heap.size++;
}

// Extract the smallest suitable block from the min-heap
static free_block_t* heap_extract_min(size_t required_size) {
    for (int i = 0; i < free_heap.size; i++) {
        if (free_heap.blocks[i]->size >= required_size) {
            free_block_t* suitable_block = free_heap.blocks[i];
            // Replace with last element and heapify
            free_heap.size--;
            free_heap.blocks[i] = free_heap.blocks[free_heap.size];
            heapify_down(i);
            return suitable_block;
        }
    }
    return NULL; // No suitable block found
}

// Initialize heap
static void init_heap(size_t size) {
    heap_start = get_me_blocks(size);
    if (heap_start == NULL) {
        fprintf(stderr, "Error: Failed to acquire heap memory.\n");
        exit(1);
    }
    heap_size = size;

    // Initialize the min-heap with the entire heap as one free block
    free_block_t* initial_block = (free_block_t*) heap_start;
    initial_block->size = size;
    heap_insert(initial_block);
}

// Helper function to check if a pointer is valid
static int is_valid_pointer(void* ptr) {
    if (ptr == NULL) return 0;
    if (ptr < heap_start) return 0;
    if (ptr >= heap_start + heap_size) return 0;
    return 1;
}

// My malloc implementation
void* xmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Initialize the heap if it hasn't been initialized yet
    if (heap_start == NULL) {
        init_heap(ALIGN(1024 * 1024)); // Initialize with 1MB aligned
    }

    // Align the requested size
    size = ALIGN(size);

    // Add space for the block size
    size_t total_size = size + sizeof(size_t);

    // Extract the smallest suitable block from the heap
    free_block_t* curr = heap_extract_min(total_size);

    // If no suitable block is found, return NULL
    if (curr == NULL) {
        fprintf(stderr, "Error: Out of memory.\n");
        return NULL;
    }

    // Allocate the block
    void* block = (void*) curr;
    size_t remaining_size = curr->size - total_size;

    // If there is enough space left, split the block
    if (remaining_size >= sizeof(free_block_t)) {
        free_block_t* new_free_block = (free_block_t*) ((char*) block + total_size);
        new_free_block->size = remaining_size;
        heap_insert(new_free_block);
    }

    // Store the block size before the allocated memory
    *((size_t*) block) = total_size;

    // Pointer to the allocated memory
    void* user_ptr = (void*) ((char*) block + sizeof(size_t));

    // Reset allocated memory to zero
    memset(user_ptr, 0, size);

    // Return a pointer to the allocated memory
    return user_ptr;
}

// My free implementation with block coalescing
void xfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    if (!is_valid_pointer(ptr)) {
        fprintf(stderr, "Error: Attempt to free invalid pointer %p.\n", ptr);
        return;
    }

    // Get the block size
    void* block = (char*) ptr - sizeof(size_t);
    size_t block_size = *((size_t*) block);

    // Create a new free block
    free_block_t* new_free_block = (free_block_t*) block;
    new_free_block->size = block_size;

    // Insert the new free block into the heap
    heap_insert(new_free_block);

    // Coalesce adjacent blocks
    // Iterate through the heap to find and merge adjacent blocks
    for (int i = 0; i < free_heap.size; i++) {
        for (int j = 0; j < free_heap.size; j++) {
            if (i == j) continue;
            free_block_t* a = free_heap.blocks[i];
            free_block_t* b = free_heap.blocks[j];
            if ((char*)a + a->size == (char*)b) {
                // Merge a and b
                a->size += b->size;
                // Remove b from the heap
                free_heap.size--;
                free_heap.blocks[j] = free_heap.blocks[free_heap.size];
                heapify_down(j);
                i = -1; // Restart since heap has changed
                break;
            }
            if ((char*)b + b->size == (char*)a) {
                // Merge b and a
                b->size += a->size;
                // Remove a from the heap
                free_heap.size--;
                free_heap.blocks[i] = free_heap.blocks[free_heap.size];
                heapify_down(i);
                i = -1; // Restart since heap has changed
                break;
            }
        }
    }
}

// My realloc implementation
void* xrealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return xmalloc(size);
    }

    if (size == 0) {
        xfree(ptr);
        return NULL;
    }

    if (!is_valid_pointer(ptr)) {
        fprintf(stderr, "Error: Attempt to realloc invalid pointer %p.\n", ptr);
        return NULL;
    }

    // Get the old block size
    void* old_block = (char*) ptr - sizeof(size_t);
    size_t old_block_size = *((size_t*) old_block);

    // Align the new size
    size = ALIGN(size);
    size_t new_total_size = size + sizeof(size_t);

    // If the new size is smaller or equal, reuse the block
    if (new_total_size <= old_block_size) {
        return ptr;
    }

    // Allocate a new block
    void* new_ptr = xmalloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    // Copy only the user data
    size_t copy_size = old_block_size - sizeof(size_t);
    memcpy(new_ptr, ptr, copy_size < size ? copy_size : size);

    // Free the old block
    xfree(ptr);

    // Return the new pointer
    return new_ptr;
}

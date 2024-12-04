#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "test_cases.h"

// Simple allocation
int test_simple_alloc_free() {
    printf("Running test_simple_alloc_free...\n");
    size_t size = 100;
    char* ptr = (char*)xmalloc(size);
    if (ptr == NULL) {
        printf("FAIL: Allocation failed.\n");
        return 0;
    }
    // Check if memory is zeroed
    for (size_t i = 0; i < size; i++) {
        if (ptr[i] != 0) {
            printf("FAIL: Memory not zeroed at byte %zu.\n", i);
            xfree(ptr);
            return 0;
        }
    }
    xfree(ptr);
    printf("PASS: test_simple_alloc_free.\n");
    return 1;
}

// Multiple Allocations and Frees
int test_multiple_alloc_free() {
    printf("Running test_multiple_alloc_free...\n");
    size_t num_allocs = 10;
    size_t size = 50;
    char* ptrs[num_allocs];

    // Allocate memory blocks
    for (size_t i = 0; i < num_allocs; i++) {
        ptrs[i] = (char*)xmalloc(size);
        if (ptrs[i] == NULL) {
            printf("FAIL: Allocation %zu failed.\n", i);
            // Free previously allocated blocks
            for (size_t j = 0; j < i; j++) {
                xfree(ptrs[j]);
            }
            return 0;
        }
        // Initialize memory with a pattern
        memset(ptrs[i], 'A' + (int)i, size);
    }

    // Verify memory contents
    for (size_t i = 0; i < num_allocs; i++) {
        for (size_t j = 0; j < size; j++) {
            if (ptrs[i][j] != ('A' + (int)i)) {
                printf("FAIL: Memory corruption in block %zu at byte %zu.\n", i, j);
                // Free all blocks
                for (size_t k = 0; k < num_allocs; k++) {
                    xfree(ptrs[k]);
                }
                return 0;
            }
        }
    }

    // Free memory blocks
    for (size_t i = 0; i < num_allocs; i++) {
        xfree(ptrs[i]);
    }

    printf("PASS: test_multiple_alloc_free.\n");
    return 1;
}

// Reallocation to Larger Size
int test_realloc_larger() {
    printf("Running test_realloc_larger...\n");
    size_t initial_size = 100;
    size_t new_size = 200;
    char* ptr = (char*)xmalloc(initial_size);
    if (ptr == NULL) {
        printf("FAIL: Initial allocation failed.\n");
        return 0;
    }
    // Initialize memory
    memset(ptr, 'B', initial_size);
    // Reallocate to larger size
    char* new_ptr = (char*)xrealloc(ptr, new_size);
    if (new_ptr == NULL) {
        printf("FAIL: Reallocation to larger size failed.\n");
        xfree(ptr);
        return 0;
    }
    // Verify old data
    for (size_t i = 0; i < initial_size; i++) {
        if (new_ptr[i] != 'B') {
            printf("FAIL: Data corruption after realloc at byte %zu.\n", i);
            xfree(new_ptr);
            return 0;
        }
    }
    // Check if new memory is zeroed
    for (size_t i = initial_size; i < new_size; i++) {
        if (new_ptr[i] != 0) {
            printf("FAIL: New memory not zeroed at byte %zu.\n", i);
            xfree(new_ptr);
            return 0;
        }
    }
    xfree(new_ptr);
    printf("PASS: test_realloc_larger.\n");
    return 1;
}

// Reallocation to Smaller Size
int test_realloc_smaller() {
    printf("Running test_realloc_smaller...\n");
    size_t initial_size = 200;
    size_t new_size = 100;
    char* ptr = (char*)xmalloc(initial_size);
    if (ptr == NULL) {
        printf("FAIL: Initial allocation failed.\n");
        return 0;
    }
    // Initialize memory
    memset(ptr, 'C', initial_size);
    // Reallocate to smaller size
    char* new_ptr = (char*)xrealloc(ptr, new_size);
    if (new_ptr == NULL) {
        printf("FAIL: Reallocation to smaller size failed.\n");
        xfree(ptr);
        return 0;
    }
    // Verify data
    for (size_t i = 0; i < new_size; i++) {
        if (new_ptr[i] != 'C') {
            printf("FAIL: Data corruption after realloc at byte %zu.\n", i);
            xfree(new_ptr);
            return 0;
        }
    }
    xfree(new_ptr);
    printf("PASS: test_realloc_smaller.\n");
    return 1;
}

// Allocate Zero Bytes
int test_alloc_zero() {
    printf("Running test_alloc_zero...\n");
    char* ptr = (char*)xmalloc(0);
    if (ptr != NULL) {
        printf("FAIL: Allocation with size 0 should return NULL.\n");
        xfree(ptr); // Although ptr should be NULL
        return 0;
    }
    printf("PASS: test_alloc_zero.\n");
    return 1;
}

// Free NULL Pointer
int test_free_null() {
    printf("Running test_free_null...\n");
    // Should not crash or produce errors
    xfree(NULL);
    printf("PASS: test_free_null.\n");
    return 1;
}

// Define the test_cases array and num_tests
test_func_t test_cases[] = {
    test_simple_alloc_free,
    test_multiple_alloc_free,
    test_realloc_larger,
    test_realloc_smaller,
    test_alloc_zero,
    test_free_null,
};

int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

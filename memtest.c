#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// define "x" for system malloc, include for our versions. Don't do both
#ifdef SYSTEM_MALLOC
#define xfree free
#define xmalloc malloc
#define xrealloc realloc
#else
#include "malloc.h"
#endif

int rand_between(int min, int max) {
    return rand() % (max - min + 1) + min; 
}

#define TEST_SIZE 10

#define MIN(a,b) ((a) < (b) ? (a) : (b))

int main(int argc, char *argv[]) {
    char *test_string = "Now is the time for all good people to come to the aid of their country.";

    if (argc > 1) {
        test_string = argv[1];
    }

    char *ptrs[TEST_SIZE];

    for (int ix=0; ix<TEST_SIZE; ix++) {
        int size = rand_between(1, 30);
        // Occasionally allocate large chunks
        if (rand_between(1, 10) == 1) {
            size = rand_between(1024, 1024 * 1024); // 1KB to 1MB
        } else {
            size = rand_between(1, 30);
        }
        fprintf(stderr, "[%d] size: %d\n", ix, size);

        ptrs[ix] = xmalloc(size);
        if (ptrs[ix] == NULL) {
            printf("[%d] malloc failed\n", ix);
            exit(1);
        }

        int len_to_copy = MIN(strlen(test_string), size-1);

        fprintf(stderr, "[%d] ptrs[%d]: %p, going to copy %d chars\n", ix, ix, ptrs[ix], len_to_copy);
        
        strncpy(ptrs[ix], test_string, len_to_copy);
        ptrs[ix][len_to_copy] = '\0';

        fprintf(stderr, "[%x] %s\n", ix, ptrs[ix]);

        // Occasionally realloc buffers
        if (rand_between(1, 5) == 1) {
            int new_size;
            // Realloc larger or smaller
            if (rand_between(1, 2) == 1) {
                new_size = size + rand_between(1, 100);
            } else {
                new_size = size - rand_between(1, size / 2);
            }
            fprintf(stderr, "[%d] reallocing %p to size %d\n", ix, ptrs[ix], new_size);
            ptrs[ix] = (char *)xrealloc(ptrs[ix], new_size);
            if (ptrs[ix] == NULL) {
                printf("[%d] realloc failed\n", ix);
                exit(1);
            }
            fprintf(stderr, "[%d] realloced %p to %p\n", ix, ptrs[ix], ptrs[ix]);
        }
    }

    for (int ix=0; ix<TEST_SIZE; ix++) {
        fprintf(stderr, "[%d] freeing %p (%s)\n", ix, ptrs[ix], ptrs[ix]);
        xfree(ptrs[ix]);
        fprintf(stderr, "[%d] freed %p\n", ix, ptrs[ix]);
    }

    return 0;
}
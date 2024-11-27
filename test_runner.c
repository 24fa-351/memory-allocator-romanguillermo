#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_cases.h"

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        if (argc < 3) {
            printf("Usage: %s -t <test_number>\n", argv[0]);
            return 1;
        }
        int test_num = atoi(argv[2]);
        if (test_num < 0 || test_num >= num_tests) {
            printf("Invalid test number. Available tests: 0 to %d\n", num_tests - 1);
            return 1;
        }
        printf("Executing Test %d:\n", test_num);
        int result = test_cases[test_num]();
        printf("Test %d %s.\n", test_num, result ? "PASSED" : "FAILED");
        return result ? 0 : 1;
    }

    // Execute all tests
    printf("Executing all tests...\n");
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: ", i);
        if (test_cases[i]()) {
            printf("PASSED.\n");
            passed++;
        } else {
            printf("FAILED.\n");
        }
    }
    printf("Passed %d/%d tests.\n", passed, num_tests);
    return (passed == num_tests) ? 0 : 1;
}

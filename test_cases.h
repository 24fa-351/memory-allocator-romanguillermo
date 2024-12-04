#ifndef TEST_CASES_H
#define TEST_CASES_H

typedef int (*test_func_t)(void);

int test_simple_alloc_free();
int test_multiple_alloc_free();
int test_large_allocation();
int test_realloc_larger();
int test_realloc_smaller();
int test_alloc_zero();
int test_free_null();
int test_double_free();

extern test_func_t test_cases[];
extern int num_tests;

#endif

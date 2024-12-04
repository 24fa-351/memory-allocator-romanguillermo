all: test_system test_malloc test_cases

test_system: memtest.c
	gcc -o test_system -DSYSTEM_MALLOC memtest.c

test_malloc: memtest.c malloc.c malloc.h
	gcc -o test_malloc memtest.c malloc.c

test_cases: test_runner.c test_cases.c malloc.c malloc.h test_cases.h
	gcc -o test_cases test_runner.c test_cases.c malloc.c

clean:
	rm -f test_system test_malloc test_cases
/**
 * SO, 2017 - Lab #07, Profiling
 * Task #2, Linux
 *
 * Row major
 */
#include <stdio.h>

#define SIZE 4096

char x[SIZE][SIZE];

int
main(void)
{
	int i, j;

	for (i = 0; i < SIZE; ++i) {
		for (j = 0; j < SIZE; ++j) {
			/* TODO */
			x[i][j] += 1;
		}
	}
	return 0;
}

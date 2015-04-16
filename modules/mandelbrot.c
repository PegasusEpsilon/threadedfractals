#include <stdio.h>	/* printf() puts() */
#include <stdlib.h>	/* exit() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SAMPLER ARGS\n", myself);
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	exit(1);
}

#include "loader.h"         	/* get_sampler() */
#include "complex_sampler.h"	/* sampler() */

static sampler(complex_sample);

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (2 > argc) usage(argv[0]);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	complex_sample = (sampler())get_sampler(&argv[1]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
FLOAT sample (complex FLOAT *const point) {
	complex FLOAT zero = 0 + 0 * I;
	return complex_sample(&zero, point);
}

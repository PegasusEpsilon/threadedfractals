#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SAMPLER ARGS", myself);
	puts("	SAMPLER	shared object file containing sampler function");
	puts("	ARGS	any arguments needed by SAMPLER");
	exit(1);
}

#include <complex.h>	/* complex, cabsl() */
#include <math.h>   	/* log2l(), logl() */

#include "loader.h"
#include "complex_sampler.h"

static sampler(complex_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (1 > argc) usage(argv[0]);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	complex_sample = (sampler())get_sampler(&argv[1]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
long double sample (long double complex *const z, long double complex *const c) {
	long double count = complex_sample(z, c);
	return count - log2l(logl(cabsl(*z)));
}

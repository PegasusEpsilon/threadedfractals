#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SAMPLER ARGS", myself);
	puts("	SAMPLER	shared object file containing sampler function");
	puts("	ARGS	any arguments needed by SAMPLER");
	exit(1);
}

#include <complex.h>	/* complex, cabs*() */
#include <math.h>   	/* log2*(), log*() */

#include "loader.h"
#include "complex_sampler.h"

static sampler(complex_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (2 > argc) usage(argv[0]);
	printf("args: %d\n", argc);
	complex_sample = (sampler())get_sampler(&argv[1]);
}

__attribute__((pure hot))
FLOAT sample (complex FLOAT *const z, complex FLOAT *const c) {
	FLOAT count = complex_sample(z, c);
	return count - LOG2(LOG(CABS(*z)));
}

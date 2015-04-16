#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s REAL IMAG\n", myself);
	puts("	REAL	amount to shift rightward on the complex plane");
	puts("	IMAG	amoutn to shift upward on the complex plane");
	exit(1);
}

#include <complex.h>	/* complex */

#include "loader.h" 	/* get_sampler() */
#include "sampler.h"	/* sampler() */

static complex FLOAT center;
static sampler(real_sample);

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (4 > argc) usage(argv[0]);
	FLOAT tmp1 = strtold(argv[1], NULL);
	FLOAT tmp2 = strtold(argv[2], NULL);
	center = tmp1 + tmp2 * I;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	/* load the sampler we've been asked to pass to */
	real_sample = (sampler())get_sampler(&argv[3]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
FLOAT sample (complex FLOAT *point) {
	*point += center;
	return real_sample(point);
}

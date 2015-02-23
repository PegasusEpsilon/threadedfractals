#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */
#include <math.h>   	/* sin(), cos() */
#include <complex.h>	/* complex */

#include "constants.h"	/* M_PI */
#include "loader.h" 	/* get_sampler() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s REAL IMAG\n", myself);
	puts("	REAL	amount to shift rightward on the complex plane");
	puts("	IMAG	amoutn to shift upward on the complex plane");
	exit(1);
}

static long double complex center;
static long double (*real_sample) (long double complex *);

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (4 > argc) usage(argv[0]);
	long double tmp1 = strtold(argv[1], NULL);
	long double tmp2 = strtold(argv[2], NULL);
	center = tmp1 + tmp2 * I;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	/* load the sampler we've been asked to pass to */
	real_sample = (long double (*) (long double complex *))get_sampler(&argv[3]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
long double sample (long double complex *point) {
	*point += center;
	return real_sample(point);
}

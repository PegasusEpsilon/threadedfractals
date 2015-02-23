#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */
#include <math.h>   	/* sin(), cos() */
#include <complex.h>	/* complex */

#include "constants.h"	/* M_PI */
#include "loader.h" 	/* get_sampler() */
#include "sampler.h"	/* sampler(), real_sample() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SCALE\n", myself);
	puts("	SCALE	smaller = zoom more");
	exit(1);
}

static long double radius;

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (3 > argc) usage(argv[0]);
	radius = strtold(argv[1], NULL);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	/* load the sampler we've been asked to pass to */
	real_sample = (sampler())get_sampler(&argv[2]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
long double sample (long double complex *point) {
	*point *= radius;
	return real_sample(point);
}

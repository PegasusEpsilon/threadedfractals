#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */
#include <math.h>   	/* sin(), cos() */
#include <complex.h>	/* creall(), cimagl() */

#include "constants.h"	/* M_PI */
#include "loader.h" 	/* get_sampler() */

#define DEG2RAD(x) (x * M_PI / 180)

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s THETA\n", myself);
	puts("	THETA	angle to rotate the image");
	exit(1);
}

static long double complex theta = 0;
static long double (*real_sample) (long double complex *);

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (3 > argc) usage(argv[0]);
	/* convert degrees to rotation matrix and store for later */
	theta = DEG2RAD(strtold(argv[1], NULL));
	theta = cosl(theta) - sinl(theta) * I;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	/* load the sampler we've been asked to pass to */
	real_sample = (long double (*) (long double complex *))get_sampler(&argv[2]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
long double sample (long double complex *point) {
	*point *= theta;
	return real_sample(point);
}

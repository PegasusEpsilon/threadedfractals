#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s THETA\n", myself);
	puts("	THETA	angle to rotate the image");
	exit(1);
}

#include <math.h>   	/* sin*(), cos*() */

#include "constants.h"	/* M_PI */
#include "sampler.h"	/* sampler() */
#include "loader.h" 	/* get_sampler() */
#include "config.h"

static complex FLOAT theta = 0;
static sampler(real_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (3 > argc) usage(argv[0]);
	/* convert degrees to rotation matrix and store for later */
	theta = strtold(argv[1], NULL) * M_PI / 180;
	theta = COS(theta) + SIN(theta) * I;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	real_sample = (sampler())get_sampler(&argv[2]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
FLOAT sample (complex FLOAT *point) {
	*point *= theta;
	return real_sample(point);
}

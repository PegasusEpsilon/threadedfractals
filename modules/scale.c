#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold))
__attribute__((noreturn))
__attribute__((always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SCALE\n", myself);
	puts("	SCALE	smaller = zoom more");
	exit(1);
}

#include <complex.h>	/* complex */

#include "constants.h"	/* M_PI */
#include "loader.h" 	/* get_sampler() */
#include "sampler.h"	/* sampler() */

static FLOAT radius;
static sampler(real_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (3 > argc) usage(argv[0]);
	radius = strtold(argv[1], NULL);
	real_sample = (sampler())get_sampler(&argv[2]);
}

__attribute__((pure))
__attribute__((hot))
FLOAT sample (complex FLOAT *point) {
	*point *= radius;
	return real_sample(point);
}

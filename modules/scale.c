#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold, noreturn, always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SCALE SAMPLER ARGS\n\n", myself);
	puts("	SCALE	smaller = zoom more");
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

#include <complex.h>	/* complex */

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

__attribute__((hot, pure))
FLOAT sample (COMPLEX *point) {
	*point *= radius;
	return real_sample(point);
}

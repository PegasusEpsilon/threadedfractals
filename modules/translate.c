#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold))
__attribute__((noreturn))
__attribute__((always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s REAL IMAG SAMPLER ARGS\n\n", myself);
	puts("	REAL	amount to shift rightward on the complex plane");
	puts("	IMAG	amoutn to shift upward on the complex plane");
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
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
	/* load the sampler we've been asked to pass to */
	real_sample = (sampler())get_sampler(&argv[3]);
}

__attribute__((pure))
__attribute__((hot))
FLOAT sample (complex FLOAT *point) {
	*point += center;
	return real_sample(point);
}

#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold, noreturn, always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SAMPLER ARGS\n\n", myself);
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

#include <complex.h>	/* complex */

#include "loader.h" 	/* get_sampler() */
#include "sampler.h"	/* sampler() */

static sampler(real_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (2 > argc) usage(argv[0]);
	real_sample = (sampler())get_sampler(&argv[1]);
}

__attribute__((hot, pure))
FLOAT sample (COMPLEX *point) {
	// I feel like there should be a simpler operation here...
	*point = CREAL(*point) - CIMAG(*point) * I;
	return real_sample(point);
}

#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold, noreturn, always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SAMPLER ARGS\n\n", myself);
	puts("	SAMPLER	shared object file containing sampler function");
	puts("	ARGS	any arguments needed by SAMPLER");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

#include <complex.h>	/* complex, cabs*() */
#include <math.h>   	/* log2*(), log*() */

#define COMPLEX
#include "loader.h"
#include "sampler.h"

static sampler(complex_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (2 > argc) usage(argv[0]);
	complex_sample = (sampler())get_sampler(&argv[1]);
}

__attribute__((hot, pure))
FLOAT sample (complex FLOAT *const z, complex FLOAT *const c) {
	FLOAT count = complex_sample(z, c);
	return count - LOG2(LOG(CABS(*z)));
}

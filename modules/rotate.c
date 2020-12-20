#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold, noreturn, always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s THETA SAMPLER ARGS\n\n", myself);
	puts("	THETA	angle to rotate the image");
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

#include <math.h>   	/* sin*(), cos*() */

#include "constants.h"	/* M_PI */
#include "sampler.h"	/* sampler() */
#include "loader.h" 	/* get_sampler() */
#include "config.h"

static COMPLEX theta = 0;
static sampler(real_sample);

__attribute__((cold))
void init (char **argv) {
	int argc = 0;
	while (argv[++argc]);
	if (3 > argc) usage(argv[0]);
	/* convert degrees to rotation matrix and store for later */
	theta = strtold(argv[1], NULL) * M_PI / 180;
	theta = COS(CREAL(theta)) + SIN(CREAL(theta)) * I;
	real_sample = (sampler())get_sampler(&argv[2]);
}

__attribute__((hot, pure))
FLOAT sample (COMPLEX *point) {
	*point *= theta;
	return real_sample(point);
}

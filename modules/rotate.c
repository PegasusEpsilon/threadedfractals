#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold))
__attribute__((noreturn))
__attribute__((always_inline)) static inline
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
	real_sample = (sampler())get_sampler(&argv[2]);
}

__attribute__((pure))
__attribute__((hot))
FLOAT sample (complex FLOAT *point) {
	*point *= theta;
	return real_sample(point);
}

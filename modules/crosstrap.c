#include <stdio.h>  	/* puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold, noreturn, always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s RANGE START ANGLE\n\n", myself);
	puts("	RANGE	range within which a trap takes effect");
	puts("		Note: larger ranges make thicker lines");
	puts("	START	iteration on which to start trapping");
	puts("		Note: higher values remove more foreground lines");
	puts("	ANGLE	angle of the cross, 0-90");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

#include <stdbool.h>	/* true, false */
#include <complex.h>	/* complex, cabs*() */
#include <math.h>   	/* sin*(), cos*(), sqrt*(), fabs*() */

#include "config.h"
#include "types.h"
#include "constants.h" /* M_PI */

#define ESCAPE 16
#define   likely(x) __builtin_expect(x, true )	/* branch prediction */
#define unlikely(x) __builtin_expect(x, false)	/* branch prediction */

static struct trap trap;

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (4 > argc) usage(argv[0]);
	trap.range = strtold(argv[1], NULL);
	trap.start = atoi(argv[2]);
	trap.theta = M_PI * strtold(argv[3], NULL) / 180;
	trap.sin = SIN(trap.theta);
	trap.cos = COS(trap.theta);
	trap.hyp = SQRT(trap.sin * trap.sin + trap.cos * trap.cos);
}

__attribute__((pure, hot))
FLOAT sample (COMPLEX *z_ptr, COMPLEX *c_ptr) {
	COMPLEX z = *z_ptr, c = *c_ptr;
	COMPLEX oz = 255 + 255 * I;
	unsigned i, deadline = 1;
	FLOAT d = (FLOAT)((unsigned long long)-1);

	for (i = 0; likely(i != (unsigned)-1); i++) {
		if (unlikely(CABS(z) > ESCAPE)) break;
		z = z * z + c;

		if (i > trap.start) {
			FLOAT v, h, n;
			v = FABS(trap.sin * CREAL(z) + trap.cos * CIMAG(z));
			h = FABS(trap.cos * CREAL(z) - trap.sin * CIMAG(z));
			n = (h < v ? h : v) / trap.hyp;
			if (n < d) d = n;
		}

		if (unlikely(oz == z)) return d / 2;
		if (i > deadline) { oz = z; deadline <<= 1; }
	}

	return trap.range < d ? -1 : trap.range - d;
}

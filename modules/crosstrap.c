#include <stdio.h>  	/* puts() */
#include <stdlib.h> 	/* exit() */

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s RANGE START ANGLE\n", myself);
	puts("	RANGE	range within which a trap takes effect");
	puts("		Note: larger ranges make bigger points");
	puts("	START	iteration on which to start trapping");
	puts("		Note: higher values remove more foreground lines");
	puts("	ANGLE	angle of the cross, 0-90");
	exit(1);
}

#include <stdbool.h>	/* true, false */
#include <complex.h>	/* complex, cabsl() */
#include <math.h>   	/* log2l(), logl() */

#include "types.h"

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
	trap.angle = strtold(argv[3], NULL);
	trap.sin = sinl(trap.angle);
	trap.cos = cosl(trap.angle);
	trap.hyp = sqrtl(trap.sin * trap.sin + trap.cos * trap.cos); /* squirtle! squirtle! */
}

__attribute__((pure hot))
long double sample (long double complex *z_ptr, long double complex *c_ptr) {
	long double complex z = *z_ptr, c = *c_ptr;
	long double complex oz = 255 + 255 * I;
	unsigned i, deadline = 1;
	long double d = (long double)((unsigned long long)-1);

	for (i = 0; likely(i != (unsigned)-1); i++) {
		if (unlikely(cabsl(z) > ESCAPE)) break;
		z = z * z + c;

		if (i > trap.start) {
			long double v, h, n;
			v = fabsl(trap.sin * creall(z) + trap.cos * cimagl(z));
			h = fabsl(trap.cos * creall(z) + trap.sin * cimagl(z));
			n = (h < v ? h : v) / trap.hyp;
			if (n < d) d = n;
		}

		if (unlikely(oz == z)) return d / 2;
		if (i > deadline) { oz = z; deadline <<= 1; }
	}

	return trap.range < d ? -1 : trap.range - d;
}

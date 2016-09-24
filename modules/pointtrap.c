#include <stdio.h>  	/* puts() */
#include <stdlib.h> 	/* exit() */
#include "config.h"

__attribute__((cold))
__attribute__((noreturn))
__attribute__((always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s RANGE START\n", myself);
	puts("	RANGE	range within which a trap takes effect");
	puts("		Note: larger ranges make bigger points");
	puts("	START	iteration on which to start trapping");
	puts("		Note: higher values remove more foreground points");
	exit(1);
}

#include <complex.h>	/* complex, cabs*() */
#include <math.h>   	/* log2*(), log*() */
#include <stdbool.h>	/* bool */

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
	if (3 > argc) usage(argv[0]);
	trap.range = strtold(argv[1], NULL);
	trap.start = atoi(argv[2]);
}

__attribute__((pure))
__attribute__((hot))
FLOAT sample (complex FLOAT *z_ptr, complex FLOAT *c_ptr) {
	complex FLOAT z = *z_ptr, c = *c_ptr;
	complex FLOAT oz = 255 + 255 * I;
	unsigned i, deadline = 1;
	FLOAT d = (FLOAT)((unsigned long long)-1);

	for (i = 0; likely(i != (unsigned)-1); i++) {
		if (unlikely(CABS(z) > ESCAPE)) break;
		z = z * z + c;

		if (i > trap.start) {
			FLOAT n = CABS(z);
			if (n < d) d = n;
		}

		if (unlikely(oz == z)) return d / 2;
		if (i > deadline) { oz = z; deadline <<= 1; }
	}

	return trap.range < d ? -1 : trap.range - d;
}

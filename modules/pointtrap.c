#include <stdio.h>  	/* puts() */
#include <stdlib.h> 	/* exit() */
#include "config.h"
#include <complex.h>	/* complex, cabs*() */
#include <math.h>   	/* log2*(), log*() */
#include <stdbool.h>	/* bool */

#include "types.h"
#include "utils.h"

#define ESCAPE 16
#define   likely(x) __builtin_expect(x, true )	/* branch prediction */
#define unlikely(x) __builtin_expect(x, false)	/* branch prediction */

static struct trap trap;

char *myself;
__attribute__((cold, noreturn, always_inline)) static inline
void usage (void) {
	printf("Usage: ... %s RANGE START\n\n", myself);
	puts("	RANGE	range within which a trap takes effect");
	puts("		Note: larger ranges make bigger points");
	puts("	START	iteration on which to start trapping");
	puts("		Note: higher values remove more foreground points");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

__attribute__((cold))
void init (char **argv) {
	myself = argv[0];
	/* FIXME: ABI change - count args */
	int argc = 0;
	while (argv[++argc]);
	if (3 > argc) usage();
	trap.range = safe_strtold(argv[1], NULL, "RANGE", &usage);
	trap.start = safe_strtoui(argv[2], NULL, 0, "START", &usage);
}

__attribute__((hot, pure))
FLOAT sample (COMPLEX *z_ptr, COMPLEX *c_ptr) {
	COMPLEX z = *z_ptr, c = *c_ptr;
	COMPLEX oz = 255 + 255 * I;
	unsigned i, deadline = 1;
	FLOAT d = (FLOAT)((long long unsigned)-1);

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

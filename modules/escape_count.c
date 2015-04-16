#include <stdio.h>  	/* printf(), puts() */
#include <stdlib.h> 	/* exit() */
#include <complex.h>	/* complex, cabs*() */
#include <stdbool.h>	/* bool */

#include "types.h"
#include "config.h"

#define ESCAPE 16
#define   likely(x) __builtin_expect(x, true )	/* branch prediction */
#define unlikely(x) __builtin_expect(x, false)	/* branch prediction */

__attribute__((pure hot always_inline)) static inline
bool inset (complex FLOAT c) {
	FLOAT r = CREAL(c), i = CIMAG(c);
	FLOAT rm = r - 0.25, rp2 = r + 1, i2 = i * i;
	FLOAT t = rm * rm + i2; rp2 *= rp2;
	/* period 1 cardoid */
	if (4 * t * (t + rm) / i2 < 1) return true;
	/* period 2 disc */
	if (16 * (rp2 + i2) < 1) return true;
	/* chaos line (needle) */
	if (unlikely(0 == i) && -1 > r && r > -2) return true;
	return false;
}

__attribute__((cold noreturn always_inline)) static inline
void usage (const char *restrict const myself) {
	printf("Usage: ... %s RADIUS\n", myself);
	puts("	RADIUS	radius outside which a point is considered to have escaped");
	exit(1);
}

static FLOAT escape;

__attribute__((cold))
void init (const char *restrict const *restrict const argv) {
	int argc = 0;
	while (argv[++argc]);
	if (2 > argc) usage(argv[0]);
	escape = strtold(argv[1], NULL);
}

__attribute__((pure hot))
FLOAT sample (
	complex FLOAT *const z_ptr,
	complex FLOAT *const c_ptr
) {
	complex FLOAT z = *z_ptr, c = *c_ptr;
	complex FLOAT oz = (unsigned long long)-1 + (unsigned long long)-1 * I;
	unsigned i, deadline = 1;

	if (inset(c)) return -1;

	for (i = 0; likely(i != (unsigned)-1); i++) {
		if (unlikely(CABS(z) > escape)) break;
		z = z * z + c;

		if (unlikely(oz == z)) return -1;
		if (i == deadline) { oz = z; deadline <<= 1; }
	}

	/* set magnitude for the renormalizer */
	*z_ptr = z;

	return i;
}

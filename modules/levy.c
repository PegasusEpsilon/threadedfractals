#include <complex.h>	/* complex, cabsl() */
#include <stdbool.h>	/* bool */
#include <math.h>   	/* sinl(), cosl(), cabsl() */

#include "types.h"
#include "constants.h"

__attribute__((cold))
void init (const char *restrict const *restrict const argv) { (void)argv; }

__attribute__((pure hot)) static
long double complex F1 (long double complex z) { return z * (1 + I); }

__attribute__((pure hot)) static
long double complex F2 (long double complex z) { return (2 + z) * (1 - I); }

__attribute__((hot always_inline)) static inline
long double recurse (
	long double complex (*) (long double complex),
	long double complex (*) (long double complex),
	long double complex, long double
);

__attribute__((hot)) static
long double mutate (
	long double complex (*f1) (long double complex),
	long double complex (*f2) (long double complex),
	long double complex z, long double trap
) {
	z = f1(z);
	trap *= sqrt(2);

	if (fabsl(creall(z)) < trap && fabsl(cimagl(z)) < trap) return -1;
	if (cabsl(z) > 8) return 1;

	return recurse(f1, f2, z, trap);
}

__attribute__((hot always_inline)) static inline
long double recurse (
	long double complex (*f1) (long double complex),
	long double complex (*f2) (long double complex),
	long double complex z, long double trap
) {
	long double one = mutate(f1, f2, z, trap);
	if (0 > one) return one;
	long double two = mutate(f2, f1, z, trap);
	if (0 > two) return two;
	return one + two;
}

__attribute__((hot))
long double sample (long double complex *const z) {
	return recurse(F1, F2, *z, (long double)1/4000);
}

#include <complex.h>	/* complex, cabs*() */
#include <stdbool.h>	/* bool */
#include <math.h>   	/* sqrt*() */

#include "constants.h"
#include "types.h"
#include "config.h"

__attribute__((hot, pure)) static
COMPLEX F1 (COMPLEX z) { return z * (1 + I); }

__attribute__((hot, pure)) static
COMPLEX F2 (COMPLEX z) { return (2 + z) * (1 - I); }

__attribute__((hot, always_inline)) static inline
FLOAT recurse (COMPLEX (*) (COMPLEX), COMPLEX (*) (COMPLEX), COMPLEX, FLOAT);

__attribute__((hot)) static
FLOAT mutate (
	COMPLEX (*f1) (COMPLEX), COMPLEX (*f2) (COMPLEX), COMPLEX z, FLOAT trap
) {
	z = f1(z);
	trap *= SQRT(2);

	if (FABS(CREAL(z)) < trap && FABS(CIMAG(z)) < trap) return -1;
	if (CABS(z) > 8) return 1;

	return recurse(f1, f2, z, trap);
}

__attribute__((hot, always_inline)) static inline
FLOAT recurse (
	COMPLEX (*f1) (COMPLEX), COMPLEX (*f2) (COMPLEX), COMPLEX z, FLOAT trap
) {
	FLOAT one = mutate(f1, f2, z, trap);
	if (0 > one) return one;
	FLOAT two = mutate(f2, f1, z, trap);
	if (0 > two) return two;
	return one + two;
}

__attribute__((hot))
FLOAT sample (COMPLEX *const z) {
	return recurse(F1, F2, *z, (FLOAT)1/4000);
}

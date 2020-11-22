/* mapper.c, from threadedfractals
 * by "Pegasus Epsilon" <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 */

#include <complex.h>	/* complex, creal*(), cimag*() */
#include "types.h"
#include "config.h"

__attribute__((pure, cold))
COMPLEX calculate_pixelsize (
	const struct pixel *const img, const COMPLEX *const radius
) {
	return (
		2 * CREAL(*radius) / (img->real - 1) +
		2 * CIMAG(*radius) / (img->imag - 1) * I
	);
}

/* caching the difference in coordinates (real and imaginary)
 * between two consecutive pixels, and adding the difference instead,
 * will slowly build rounding errors. don't do that.
 */
COMPLEX pixel2vector (
	const struct pixel *const in,
	const COMPLEX *const size, const COMPLEX *const radius
) {
	return in->real * CREAL(*size) + in->imag * CIMAG(*size) * I - *radius;
}

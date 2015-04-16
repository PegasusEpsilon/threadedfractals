#include <complex.h>	/* complex, creal*(), cimag*() */
#include "types.h"
#include "config.h"

complex FLOAT calculate_pixelsize (
	const struct pixel *const img,
	const complex FLOAT *const radius
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
complex FLOAT pixel2vector (
	const struct pixel *const in,
	const complex FLOAT *const size,
	const complex FLOAT *const radius
) {
	return in->real * CREAL(*size) + in->imag * CIMAG(*size) * I - *radius;
}

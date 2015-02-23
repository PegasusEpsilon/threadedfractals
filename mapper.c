#include <math.h>
#include <complex.h>
#include "types.h"

long double complex calculate_pixelsize (
	const struct pixel *const img,
	const long double complex *const radius
) {
	return (
		2 * creall(*radius) / (img->real - 1) +
		2 * cimagl(*radius) / (img->imag - 1) * I
	);
}

/* caching the difference in coordinates (real and imaginary)
 * between two consecutive pixels, and adding the difference instead,
 * will slowly build rounding errors. don't do that.
 */
long double complex pixel2vector (
	const struct pixel *const in,
	const long double complex *const size,
	const long double complex *const radius
) {
	return in->real * creall(*size) + in->imag * cimagl(*size) * I - *radius;
}

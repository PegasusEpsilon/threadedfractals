/* mapper.c/v2.0 - pixelmapper for threadedfractals
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * Distribute Unmodified -- http://pegasus.pimpninjas.org/license
 *
 *  Changelog:
 *  v2.0 -- stole from complexfractals and completely rewrote everything
 *  v1.1 -- added thetaless pixel2vector and wrapper to choose between
 *          thetaless or thetaful versions.
 *  v1.0 -- pulled into seperate module
 */

#include <math.h>
#include <complex.h>
#include "types.h"

long double complex calculate_pixelsize (
	const struct pixel *const img,
	const struct region *const window
) {
	return (
		2 * creall(window->radius) / (img->real - 1) +
		2 * cimagl(window->radius) / (img->imag - 1) * I
	);
}

long double complex pixel2vector (
	const struct pixel *const in, const long double complex *const size,
	const struct region *const window, const long double *const theta
) {
	long double complex out = in->real * creall(*size) + in->imag * cimagl(*size) * I - window->radius;
	if (theta && *theta) out = ( /* rotate if theta */
		(creall(out) * cosl(*theta) - cimagl(out) * sinl(*theta)) +
		(creall(out) * sinl(*theta) + cimagl(out) * cosl(*theta)) * I
	);
	return out + window->center;
}

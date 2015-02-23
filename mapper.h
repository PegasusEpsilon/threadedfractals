/* mapper.h/v1.0 - pixelmapper for complexfractals
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * Distribute Unmodified -- http://pegasus.pimpninjas.org/license
 *
 *  Changelog:
 *  v1.0 -- pulled into seperate module
 */

#include "types.h"

long double complex calculate_pixelsize (
	const struct pixel *const,
	const long double complex *const
);

long double complex pixel2vector (
	const struct pixel *const,
	const long double complex *const,
	const long double complex *const
);

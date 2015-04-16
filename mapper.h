/* mapper.h/v1.0 - pixelmapper for complexfractals
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * Distribute Unmodified -- http://pegasus.pimpninjas.org/license
 *
 *  Changelog:
 *  v1.0 -- pulled into seperate module
 */

#ifndef MAPPER_H
#define MAPPER_H

#include "types.h"
#include "config.h"

FLOAT complex calculate_pixelsize (
	const struct pixel *const,
	const FLOAT complex *const
);

FLOAT complex pixel2vector (
	const struct pixel *const,
	const FLOAT complex *const,
	const FLOAT complex *const
);

#endif /* MAPPER_H */

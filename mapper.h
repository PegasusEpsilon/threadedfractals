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

COMPLEX calculate_pixelsize (const struct pixel *const, const COMPLEX *const);

COMPLEX pixel2vector (
	const struct pixel *const, const COMPLEX *const, const COMPLEX *const
);

#endif /* MAPPER_H */

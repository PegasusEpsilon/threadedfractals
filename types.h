/* types.h/v1 by Pegasus Epsilon <pegasus@pimpninjas.org>
 *
 * Too short to reasonably copyright, so whatevs.
 */
#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <complex.h>

#include "config.h"

/* would be nice if we could have long long unsigned complex in C99
 * but as it is, such things are GCC extensions. so we won't use them.
 */
struct pixel { long long unsigned real, imag; };

struct trap {
	FLOAT theta, range, sin, cos, hyp;
	uint32_t start;
};

#endif /* TYPES_H */

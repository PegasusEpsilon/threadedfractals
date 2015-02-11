#include <complex.h>	/* complex, cabsl() */
#include <math.h>   	/* log2l(), logl() */
#include <stdbool.h>	/* bool */

#include "types.h"

#define ESCAPE 16
#define _pure __attribute__((pure))	/* external state doesn't matter */
#define _hot __attribute__((hot))	/* called often */
#define   likely(x) __builtin_expect(x, true )	/* branch prediction */
#define unlikely(x) __builtin_expect(x, false)	/* branch prediction */


static inline bool _pure _hot inset (long double complex c) {
	long double r = creall(c), i = cimagl(c);
	long double rm = r - 0.25, rp2 = r + 1, i2 = i * i;
	long double t = rm * rm + i2; rp2 *= rp2;
	/* period 1 cardoid */
	if (4 * t * (t + rm) / i2 < 1) return true;
	/* period 2 disc */
	if (16 * (rp2 + i2) < 1) return true;
	/* chaos line (needle) */
	if (unlikely(0 == i) && -1 > r && r > -2) return true;
	return false;
}

long double _pure _hot sample (struct coordinates_4d *coordinates) {
	long double complex z = coordinates->z, c = coordinates->c,
	                   oz = 255 + 255 * I;
	unsigned i, deadline = 1;

	if (inset(coordinates->c)) return -1;

	for (i = 0; likely(i != (unsigned)-1); i++) {
		if (unlikely(cabsl(z) > ESCAPE)) break;
		z = z * z + c;

		if (unlikely(oz == z)) return -1;
		if (i == deadline) { oz = z; deadline <<= 1; }
	}

	return (long double)i - log2l(logl(cabsl(z)));
}

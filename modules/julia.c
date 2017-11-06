#include <stdio.h>	/* printf() puts() */
#include <stdlib.h>	/* exit() */

#include "types.h"
#include "loader.h"
#include "complex_sampler.h"

__attribute__((cold, noreturn, always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SEED_REAL SEED_IMAG SAMPLER ARGS\n\n", myself);
	puts("	SEED_REAL	real coordinates on the mandelbrot plane");
	puts("	SEED_IMAG	imaginary coordinates on the mandelbrot plane");
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

static complex FLOAT mandelbrot_coords;
static sampler(complex_sample);

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (4 > argc) usage(argv[0]);
	FLOAT real = strtold(argv[1], NULL);
	FLOAT imag = strtold(argv[2], NULL);
	mandelbrot_coords = real + imag * I;
	complex_sample = (sampler())get_sampler(&argv[3]);
}

__attribute__((hot, pure))
FLOAT sample (complex FLOAT *const point) {
	complex FLOAT copy = mandelbrot_coords;
	return complex_sample(point, &copy);
}

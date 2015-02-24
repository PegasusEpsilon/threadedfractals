#include <stdio.h>	/* printf() puts() */
#include <stdlib.h>	/* exit() */

#include "types.h"
#include "loader.h"
#include "complex_sampler.h"

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SEED_REAL SEED_IMAG SAMPLER ARGS\n", myself);
	puts("	SEED_REAL	real coordinates on the mandelbrot plane");
	puts("	SEED_IMAG	imaginary coordinates on the mandelbrot plane");
	puts("	SAMPLER	shared object file containing complex sampler function");
	puts("	ARGS	any extra arguments required by the complex sampler");
	exit(1);
}

static long double complex mandelbrot_coords;
static sampler(complex_sample);

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[++argc]);
	if (4 > argc) usage(argv[0]);
	long double real = strtold(argv[1], NULL);
	long double imag = strtold(argv[2], NULL);
	mandelbrot_coords = real + imag * I;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	complex_sample = (sampler())get_sampler(&argv[3]);
#pragma GCC diagnostic pop
}

__attribute__((pure hot))
long double sample (long double complex *const point) {
	long double complex copy = mandelbrot_coords;
	return complex_sample(point, &copy);
}

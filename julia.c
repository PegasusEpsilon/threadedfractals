#include <stdio.h>	/* printf() puts() */
#include <stdlib.h>	/* exit() */

#include "types.h"
#include "loader.h"

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	printf("Usage: ... %s SAMPLER ARGS\n", myself);
	puts("	SAMPLER	shared object file containing REAL sampler function");
	puts("	ARGS	any extra arguments required by the REAL sampler");
	exit(0);
}

static long double (*real_sample) (struct coordinates_4d *);
static long double complex mandelbrot_coords;

__attribute__((pure hot))
long double sample (struct coordinates_4d *coords) {
	coords->z = coords->c;
	coords->c = mandelbrot_coords;
	return real_sample(coords);
}

__attribute__((cold))
void init (char **argv) {
	/* count args */
	int argc = 0;
	while (argv[argc++]);
	if (4 > argc) usage(argv[0]);
	long double real = strtold(argv[1], NULL);
	long double imag = strtold(argv[2], NULL);
	mandelbrot_coords = real + imag * I;
	real_sample = get_sampler(&argv[3]);
}

#define _XOPEN_SOURCE 600	/* pthread_barrier_* */

#include <stdio.h>  	/* printf(), puts(), fopen(), fwrite() */
#include <stdlib.h> 	/* strtold() */
#include <stdarg.h> 	/* va_list, va_start(), va_end() */
#include <stdbool.h>	/* bool, true, false */

#include "loader.h"
#include "mapper.h"
#include "sample.h"
#include "types.h"

#define unlikely(x) __builtin_expect(x, 0)

long double *buffer;
FILE *output_file;
unsigned long long next_line = 0;
struct pixel max;

__attribute__((hot always_inline)) static inline
void display (void) {
	printf(
		"%llu/%llu (%02.02f%%)\x1b[K\r",
		next_line, max.imag, next_line / (float)max.imag * 100
	);
	fflush(stdout);
}

FILE *output_file;
__attribute__((hot always_inline)) static inline
void output (void) {
	fwrite(buffer, sizeof(long double), max.real, output_file);
	display();
	next_line++;
}

long double theta;
long double complex pixelsize;
struct region viewport;
__attribute__((hot always_inline)) static inline
void iterate_line () {
	struct coordinates_4d coordinates = { .z = 0 + 0 * I };
	struct pixel this = { .imag = next_line };

	for (this.real = 0; this.real < max.real; this.real++) {
		coordinates.c = pixel2vector(&this, &pixelsize, &viewport, &theta);
		buffer[this.real] = sample(&coordinates);
	}
}

__attribute__((cold always_inline)) static inline
void thread (void) {
	do {
		iterate_line();
		output();
	} while (next_line != max.imag);
}

__attribute__((cold noreturn always_inline)) static inline
void usage (char *myself) {
	puts("Unthreaded Mandelbrot sampler\n");
	printf("Usage: %s WIDTH HEIGHT CEN_REAL CEN_IMAG RAD_REAL RAD_IMAG THETA OUTFILE SAMPLER\n\n", myself);
	puts("	WIDTH	number of horizontal samples");
	puts("	HEIGHT	number of vertical samples");
	puts("	center coordinates (CEN_REAL, CEN_IMAG)");
	puts("		real     	horizontal center of the sampled area on the complex plane");
	puts("		imaginary	vertical \"");
	puts("	radius dimensions (RAD_REAL, RAD_IMAG)");
	puts("		real    	width of the sampled area on the complex plane");
	puts("		imaginary	height \"");
	puts("	THETA	angle to rotate the sample matrix around the center coordinate");
	puts("	OUTFILE	name of file to write output to");
	puts("	SAMPLER	shared object file containing sampler function");
	puts("	ARGS	any extra arguments required by the sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(0);
}

int main (int argc, char **argv) {
	if (10 > argc) usage(argv[0]);

	sample = get_sampler(&argv[9]);

	max.real = atoi(argv[1]);
	max.imag = atoi(argv[2]);
	/* force GCC -ffast-math to be sensible */
	long double tmp1 = strtold(argv[3], NULL);
	long double tmp2 = strtold(argv[4], NULL);
	viewport.center = tmp1 - tmp2 * I;
	tmp1 = strtold(argv[5], NULL);
	tmp2 = strtold(argv[6], NULL);
	viewport.radius = tmp1 + tmp2 * I;
	theta = strtold(argv[7], NULL);
	output_file = fopen(argv[8], "w");

	/* cache some math */
	pixelsize = calculate_pixelsize(&max, &viewport);

	/* allocate output buffer */
	buffer = calloc(max.real, sizeof(long double));
	thread();
	display();
	puts("\n\nDone!");

	fclose(output_file);

	return 0;
}

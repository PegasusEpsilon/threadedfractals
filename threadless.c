#define _XOPEN_SOURCE 600	/* pthread_barrier_* */

#include <stdio.h>  	/* printf(), puts(), fopen(), fwrite() */
#include <stdlib.h> 	/* strtold() */
#include <stdarg.h> 	/* va_list, va_start(), va_end() */
#include <stdbool.h>	/* bool, true, false */

#include "mapper.h"
#include "sample.h"
#include "types.h"

#define new(x) malloc(sizeof(x))
#define _hot  __attribute__((hot ))	/* called often */
#define _cold __attribute__((cold))	/* called rarely */
#define unlikely(x) __builtin_expect(x, 0)

struct pixel max;
unsigned long long render = 0, write;

void display (void) {
	printf("%llu/%llu (%llu%%)\r", render, max.imag, 100 * render / max.imag);
	fflush(stdout);
}

FILE *output_file;
static inline _hot unsigned long long output (long double *data) {
	fwrite(data, sizeof(long double), max.real, output_file);
	free(data);
	fflush(output_file);
	unsigned long long next = (render == max.imag ? (unsigned long long)-1 : render++);
	return next;
}

long double theta;
long double complex pixelsize;
struct region viewport;
static inline _hot long double *iterate_line (
	const unsigned long long line
) {
	struct coordinates_4d coordinates = { .z = 0 + 0 * I };
	long double *buffer = malloc(max.real * sizeof(long double));
	struct pixel this = { .imag = line };

	for (this.real = 0; this.real < max.real; this.real++) {
		coordinates.c = pixel2vector(&this, &pixelsize, &viewport, &theta);
		buffer[this.real] = sample(&coordinates);
	}
	return buffer;
}

static inline void *thread (void *ptr) {
	unsigned long long thread = (unsigned long long)ptr;
	while ((unsigned long long)-1 != (
		thread = output(iterate_line(thread))
	)) { display(); }
	return NULL;
}

__attribute__((noreturn))
void _cold usage (char *myself) {
	puts("Threaded Mandelbrot sampler\n");
	printf("Usage: %s THREADS LIMIT WIDTH HEIGHT CEN_REAL CEN_IMAG RAD_REAL RAD_IMAG THETA OUTFILE\n\n", myself);
	puts("	THREADS	how many threads to spawn");
	puts("	LIMIT	max pending writes before threads block");
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
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(0);
}

int main (int argc, char **argv) {

	if (9 > argc) usage(argv[0]);

	max.real = atoi(argv[1]);
	max.imag = atoi(argv[2]);
	viewport.center = strtold(argv[3], NULL) - strtold(argv[4], NULL) * I;
	viewport.radius = strtold(argv[5], NULL) + strtold(argv[6], NULL) * I;
	theta = strtold(argv[7], NULL);
	output_file = fopen(argv[8], "w");

	pixelsize = calculate_pixelsize(&max, &viewport);

	thread((void *)render);

	return 0;
}

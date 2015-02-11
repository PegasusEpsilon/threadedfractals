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

unsigned long long next_write = 0;
struct pixel max, current;

void display (void) {
	printf("%llu/%llu (%llu%%)\x1b[K\r", next_write, max.imag, 100 * next_write / max.imag);
	fflush(stdout);
}

static inline unsigned long long _hot next_line () {
	if (unlikely(current.imag >= max.imag)) return -1;

	++next_write;
	unsigned long long copy = current.imag++;

	return copy;
}

FILE *output_file;
#define write_buffer(x) fwrite(x, sizeof(long double), max.real, output_file)
static inline void _hot output (long double *data) {
	write_buffer(data);
	fflush(output_file);
	free(data);
}

long double theta;
long double complex pixelsize;
struct region viewport;
static inline void _hot *iterate_line (void) {
	struct coordinates_4d coordinates = { .z = 0 + 0 * I };
	long double *line = malloc(max.real * sizeof(long double));
	struct pixel this = { .imag = next_line() };
	display();

	if ((unsigned long long)-1 == this.imag) return NULL;
	for (this.real = 0; this.real < max.real; this.real++) {
		coordinates.c = pixel2vector(&this, &pixelsize, &viewport, &theta);
		line[this.real] = sample(&coordinates);
	}
	output(line);
	return NULL;
}

static inline void *persistent_thread (void) {
	while (unlikely(current.imag < max.imag)) iterate_line();
	return NULL;
}

__attribute__((noreturn))
void _cold usage (char *myself) {
	puts("Threaded Mandelbrot sampler\n");
	printf("Usage: %s WIDTH HEIGHT CEN_REAL CEN_IMAG RAD_REAL RAD_IMAG THETA OUTFILE\n\n", myself);
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

	if (11 > argc) usage(argv[0]);

	max.real = atoi(argv[3]);
	max.imag = atoi(argv[4]);
	viewport.center = strtold(argv[5], NULL) - strtold(argv[6], NULL) * I;
	viewport.radius = strtold(argv[7], NULL) + strtold(argv[8], NULL) * I;
	theta = strtold(argv[9], NULL);
	output_file = fopen(argv[10], "w");

	pixelsize = calculate_pixelsize(&max, &viewport);

	persistent_thread();

	return 0;
}

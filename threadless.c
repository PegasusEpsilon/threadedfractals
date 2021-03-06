/* threadless.c, from threadedfractals
 * by "Pegasus Epsilon" <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 */

#define _XOPEN_SOURCE 600	/* pthread_barrier_* */

#include <stdio.h>  	/* printf(), puts(), fopen(), fwrite() */
#include <stdlib.h> 	/* strtold() */
#include <stdarg.h> 	/* va_list, va_start(), va_end() */
#include <stdbool.h>	/* bool, true, false */
#include <errno.h>  	/* errno */

#include "modules/sampler.h"
#include "loader.h"
#include "mapper.h"
#include "types.h"
#include "utils.h"

FLOAT *buffer;
FILE *output_file;
long long unsigned next_line = 0;
struct pixel max;
char *myself;

__attribute__((hot, always_inline)) static inline
void display (void) {
	printf(
		"%llu/%llu (%02.02" FMT "%%)\x1b[K\r",
		next_line, max.imag, next_line / (FLOAT)max.imag * 100
	);
	fflush(stdout);
}

FILE *output_file;
__attribute__((hot, always_inline)) static inline
void output (void) {
	fwrite(buffer, sizeof(FLOAT), max.real, output_file);
	display();
	next_line++;
}

COMPLEX pixelsize;
COMPLEX ratio;
static sampler(sample);
__attribute__((hot, always_inline)) static inline
void iterate_line () {
	COMPLEX point;
	struct pixel this = { .imag = next_line };

	for (this.real = 0; this.real < max.real; this.real++) {
		point = pixel2vector(&this, &pixelsize, &ratio);
		buffer[this.real] = sample(&point);
	}
}

__attribute__((cold, always_inline)) static inline
void thread (void) {
	do {
		iterate_line();
		output();
	} while (next_line != max.imag);
}

__attribute__((cold, noreturn, always_inline)) static inline
void usage (void) {
	puts("Unthreaded fractal sampler\n");
	printf("Usage: %s WIDTH HEIGHT OUTFILE SAMPLER ARGS\n\n", myself);
	puts("	WIDTH	number of horizontal samples");
	puts("	HEIGHT	number of vertical samples");
	puts("	OUTFILE	name of file to write output to");
	puts("	SAMPLER	shared object file containing sampler function");
	puts("	ARGS	any extra arguments required by the sampler");
	puts("\nReport bugs to pegasus@pimpninjas.org");
	exit(1);
}

int main (int argc, char **argv) {
	myself = argv[0];
	if (5 > argc) usage();

	max.real = safe_strtoull(argv[1], NULL, 0, "WIDTH", &usage);
	max.imag = safe_strtoull(argv[2], NULL, 0, "HEIGHT", &usage);

	/* run sampler argument checks before creating output file */
	sample = get_sampler(&argv[4]);

	/* create output file */
	output_file = fopen(argv[3], "w");

	ratio = 1 - (FLOAT)max.imag / max.real * I;
	/* cache some math */
	pixelsize = calculate_pixelsize(&max, &ratio);

	/* allocate output buffer */
	buffer = calloc(max.real, sizeof(FLOAT));
	thread();
	display();
	puts("\n\nDone!");

	fclose(output_file);

	return 0;
}

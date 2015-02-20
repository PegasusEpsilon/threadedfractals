#define _XOPEN_SOURCE 600	/* pthread_barrier_* */

#include <stdio.h>  	/* printf(), puts(), fopen(), fwrite() */
#include <stdlib.h> 	/* strtold() */
#include <stdarg.h> 	/* va_list, va_start(), va_end() */
#include <pthread.h>	/* pthread_* */
#include <stdbool.h>	/* bool, true, false */

#include "mapper.h"
#include "sample.h"
#include "types.h"

#define _hot  __attribute__((hot ))	/* called often */
#define _cold __attribute__((cold))	/* called rarely */
#define unlikely(x) __builtin_expect(x, 0)

struct line { long double *data; bool ready; };

struct line *buffer_start, *buffer_end, *buffer_read, *buffer_write;
bool buffer_full = false;
FILE *output_file;
unsigned long long next_line = 0;
unsigned long long thread_count, *queue;
struct pixel max;

static inline _hot void display (void) {
	putchar('[');
	for (struct line *tmp = buffer_start; tmp < buffer_end; tmp++)
		putchar(tmp->ready ? '#' : ' ');
	printf("]\n");
	for (unsigned long long i = 0; i < thread_count; i++)
		printf("%llu, ", queue[i]);
	printf(
		"%llu/%llu (%02.02f%%)\x1b[K\x1b[A\r",
		next_line, max.imag, next_line / (float)max.imag * 100
	);
	fflush(stdout);
}

pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_written = PTHREAD_COND_INITIALIZER;
FILE *output_file;
static inline _hot unsigned long long output (struct line **line) {
	pthread_mutex_lock(&write_lock);

	while (buffer_read->ready) {
		fwrite(buffer_read->data, sizeof(long double), max.real, output_file);
		buffer_full = buffer_read->ready = false;
		if (++buffer_read == buffer_end) buffer_read = buffer_start;
	}

	if (!buffer_full) pthread_cond_broadcast(&data_written);
	else while (buffer_full) pthread_cond_wait(&data_written, &write_lock);

	if (next_line == max.imag) {
		pthread_mutex_unlock(&write_lock);
		return -1;
	}

	*line = buffer_write;

	if (++buffer_write == buffer_end) buffer_write = buffer_start;

	if (buffer_write == buffer_read) buffer_full = true;

	display();

	pthread_mutex_unlock(&write_lock);

	return next_line++;
}

long double theta;
long double complex pixelsize;
struct region viewport;
static inline _hot struct line **iterate_line (
	struct line **line, unsigned long long imag
) {
	struct coordinates_4d coordinates = { .z = 0 + 0 * I };
	struct pixel this = { .imag = imag };

	for (this.real = 0; this.real < max.real; this.real++) {
		coordinates.c = pixel2vector(&this, &pixelsize, &viewport, &theta);
		(*line)->data[this.real] = sample(&coordinates);
	}
	// Turns out my CPU does witchcraft. This will put a stop to that.
	__sync_synchronize();
	(*line)->ready = true;
	return line;
}

static void *thread (void *ptr) {
	unsigned long long thread = (unsigned long long)ptr;
	struct line *line = buffer_start + thread;
	queue[thread] = thread;
	while ((unsigned long long)-1 != (
		queue[thread] = output(iterate_line(&line, queue[thread]))
	));
	queue[thread] = 0;
	pthread_exit(0);
	return NULL;
}

__attribute__((noreturn))
void _cold usage (char *myself) {
	puts("Threaded Mandelbrot sampler\n");
	printf("Usage: %s THREADS WIDTH HEIGHT CEN_REAL CEN_IMAG RAD_REAL RAD_IMAG THETA OUTFILE\n\n", myself);
	puts("	THREADS	how many threads to spawn");
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

	if (10 > argc) usage(argv[0]);

	thread_count = atoi(argv[1]);
	max.real = atoi(argv[2]);
	max.imag = atoi(argv[3]);
	/* force GCC -ffast-math to be sensible */
	long double tmp1 = strtold(argv[4], NULL);
	long double tmp2 = strtold(argv[5], NULL);
	viewport.center = tmp1 - tmp2 * I;
	tmp1 = strtold(argv[6], NULL);
	tmp2 = strtold(argv[7], NULL);
	viewport.radius = tmp1 + tmp2 * I;
	theta = strtold(argv[8], NULL);
	output_file = fopen(argv[9], "w");

	/* cache some math */
	pixelsize = calculate_pixelsize(&max, &viewport);

	/* allocate output buffer */
	unsigned long long buffer_size = 8 * thread_count;
	buffer_read = buffer_start = calloc(buffer_size, sizeof(struct line));
	buffer_write = buffer_read + thread_count;
	buffer_end = buffer_start + buffer_size;
	for (unsigned long long i = 0; i < buffer_size; i++)
		buffer_start[i].data = calloc(max.real, sizeof(long double));

	/* allocate process tracking space */
	queue = calloc(thread_count, sizeof(unsigned long long));
	pthread_t *threads = calloc(thread_count - 1, sizeof(pthread_t));

	printf("spinning up %llu threads\n", thread_count);

	for (next_line = 0; next_line < thread_count - 1; next_line++)
		pthread_create(threads + next_line, NULL, &thread, (void *)next_line);
	thread((void *)next_line++);
	while (--thread_count) pthread_join(threads[thread_count], NULL);
	display();
	puts("\n\nDone!");

	free(threads);
	free(queue);
	free(buffer_start);
	fclose(output_file);

	return 0;
}

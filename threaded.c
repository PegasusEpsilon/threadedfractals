#define _XOPEN_SOURCE 600	/* pthread_barrier_* */

#include <stdio.h>  	/* printf(), puts(), fopen(), fwrite() */
#include <stdlib.h> 	/* strtold() */
#include <stdarg.h> 	/* va_list, va_start(), va_end() */
#include <pthread.h>	/* pthread_* */
#include <stdbool.h>	/* bool, true, false */

#include "indexedlist.h"
#include "mapper.h"
#include "sample.h"
#include "types.h"

#define _hot  __attribute__((hot ))	/* called often */
#define _cold __attribute__((cold))	/* called rarely */
#define unlikely(x) __builtin_expect(x, 0)

unsigned long long lines_waiting = 0, max_lines_waiting;
unsigned long long thread_count, *queue;
struct pixel max;
unsigned long long render, write;

void display (void) {
	printf("%llu/%llu (%llu%%), queue: ", render, max.imag, 100 * render / max.imag);
	for (unsigned long long i = 0; i < thread_count; i++)
		printf("%llu, ", queue[i]);
	printf("buffer: %llu/%llu (%llu%%)\x1b[K\r",
		lines_waiting, max_lines_waiting, 100 * lines_waiting / max_lines_waiting);
	fflush(stdout);
}

pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_written = PTHREAD_COND_INITIALIZER;
FILE *output_file;
list_t output_buffer;
static inline _hot unsigned long long output (
	long double *data, const unsigned long long sequence
) {
	pthread_mutex_lock(&write_lock);
	while (lines_waiting >= max_lines_waiting && write != sequence)
		pthread_cond_wait(&data_written, &write_lock);
	if (write == sequence) {
		for (;;) {
			fwrite(data, sizeof(long double), max.real, output_file);
			free(data);
			if (++write != list_first_index(output_buffer)) break;
			data = list_pop(output_buffer);
			--lines_waiting;
		}
		fflush(output_file);
		pthread_cond_broadcast(&data_written);
	} else {
		++lines_waiting;
		list_insert(output_buffer, data, sequence);
		list_first_index(output_buffer);
	}
	unsigned long long next = (render == max.imag ? (unsigned long long)-1 : render++);
	pthread_mutex_unlock(&write_lock);
	return next;
}

long double theta;
long double complex pixelsize;
struct region viewport;
static inline _hot long double *iterate_line (
	const unsigned long long line
) {
	struct coordinates_4d coordinates = { .z = 0 + 0 * I };
	long double *buffer = calloc(max.real, sizeof(long double));
	struct pixel this = { .imag = line };

	for (this.real = 0; this.real < max.real; this.real++) {
		coordinates.c = pixel2vector(&this, &pixelsize, &viewport, &theta);
		buffer[this.real] = sample(&coordinates);
	}
	return buffer;
}

static inline void *thread (void *ptr) {
	unsigned long long thread = (unsigned long long)ptr;
	queue[thread] = thread;
	while ((unsigned long long)-1 != (
		queue[thread] = output(iterate_line(queue[thread]), queue[thread])
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

	if (11 > argc) usage(argv[0]);

	render = thread_count = atoi(argv[1]);
	max_lines_waiting = atoi(argv[2]);
	max.real = atoi(argv[3]);
	max.imag = atoi(argv[4]);
	viewport.center = strtold(argv[5], NULL) - strtold(argv[6], NULL) * I;
	viewport.radius = strtold(argv[7], NULL) + strtold(argv[8], NULL) * I;
	theta = strtold(argv[9], NULL);
	output_file = fopen(argv[10], "w");

	pixelsize = calculate_pixelsize(&max, &viewport);

	output_buffer = new_list();
	queue = calloc(thread_count, sizeof(unsigned long long));
	pthread_t *threads = calloc(thread_count - 1, sizeof(pthread_t));

	printf("spinning up %llu threads\n", thread_count);

	unsigned long long i = 0;
	for (; i < thread_count - 1; i++)
		pthread_create(threads + i, NULL, &thread, (void *)i);
	thread((void *)i);
	while (--i < thread_count) pthread_join(threads[i], NULL);

	free(threads);
	free(queue);
	list_destroy(output_buffer);
	fclose(output_file);

	return 0;
}

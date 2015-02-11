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

#define new(x) malloc(sizeof(x))
#define _hot  __attribute__((hot ))	/* called often */
#define _cold __attribute__((cold))	/* called rarely */
#define unlikely(x) __builtin_expect(x, 0)

unsigned long long lines_waiting = 0, next_write = 0;
unsigned long long max_lines_waiting, thread_count, *queue;
struct pixel max, current;

void display (void) {
	printf("%llu/%llu (%llu%%), queue: ", next_write, max.imag, 100 * next_write / max.imag);
	for (unsigned long long i = 0; i < thread_count; i++)
		printf("%llu, ", queue[i]);
	printf("buffer: %llu/%llu (%llu%%)\x1b[K\r",
		lines_waiting, max_lines_waiting, 100 * lines_waiting / max_lines_waiting);
	fflush(stdout);
}

pthread_mutex_t current_lock = PTHREAD_MUTEX_INITIALIZER;
static inline unsigned long long _hot next_line () {
	if (unlikely(current.imag >= max.imag)) return -1;

	pthread_mutex_lock(&current_lock);
	unsigned long long copy = current.imag++;
	pthread_mutex_unlock(&current_lock);

	return copy;
}

pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
FILE *output_file;
list_t output_buffer;
pthread_cond_t data_written = PTHREAD_COND_INITIALIZER;
#define write_buffer(x) fwrite(x, sizeof(long double), max.real, output_file)
static inline void _hot output (long double *data, unsigned long long sequence) {
	pthread_mutex_lock(&write_lock);
	while (lines_waiting >= max_lines_waiting && next_write != sequence)
		pthread_cond_wait(&data_written, &write_lock);
	if (next_write == sequence) {
		for (;;) {
			write_buffer(data);
			free(data);
			unsigned long long next_line = list_first_index(output_buffer);
			if (++next_write != next_line) break;
			--lines_waiting;
			data = list_pop(output_buffer);
		}
		fflush(output_file);
		pthread_cond_broadcast(&data_written);
	} else {
		++lines_waiting;
		list_insert(output_buffer, data, sequence);
		list_first_index(output_buffer);
	}
	pthread_mutex_unlock(&write_lock);
}

long double theta;
long double complex pixelsize;
struct region viewport;
static inline void _hot *iterate_line (unsigned long long thread) {
	struct coordinates_4d coordinates = { .z = 0 + 0 * I };
	long double *line = malloc(max.real * sizeof(long double));
	struct pixel this = { .imag = next_line() };
	queue[thread] = this.imag;
	display();

	if ((unsigned long long)-1 == this.imag) return NULL;
	for (this.real = 0; this.real < max.real; this.real++) {
		coordinates.c = pixel2vector(&this, &pixelsize, &viewport, &theta);
		line[this.real] = sample(&coordinates);
	}
	output(line, this.imag);
	return NULL;
}

static inline void *persistent_thread (void *thread) {
	while (unlikely(current.imag < max.imag)) iterate_line((unsigned long long)thread);
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

	thread_count = atoi(argv[1]);
	max_lines_waiting = atoi(argv[2]);
	max.real = atoi(argv[3]);
	max.imag = atoi(argv[4]);
	viewport.center = strtold(argv[5], NULL) - strtold(argv[6], NULL) * I;
	viewport.radius = strtold(argv[7], NULL) + strtold(argv[8], NULL) * I;
	theta = strtold(argv[9], NULL);
	output_file = fopen(argv[10], "w");

	pixelsize = calculate_pixelsize(&max, &viewport);

	output_buffer = new_list();

	printf("spinning up %llu threads\n", thread_count);
	queue = malloc(thread_count * sizeof(unsigned long long));
	pthread_attr_t thread_attributes;
	pthread_attr_init(&thread_attributes);
	pthread_attr_setdetachstate(&thread_attributes, PTHREAD_CREATE_DETACHED);
	pthread_t pointless; /* can't be null */
	for (unsigned long long i = 0; i < thread_count; i++)
		pthread_create(&pointless, &thread_attributes, &persistent_thread, (void *)i);

	pthread_exit(0);
	return 0;
}

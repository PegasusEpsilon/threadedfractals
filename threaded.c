/* threaded.c, from threadedfractals
 * by "Pegasus Epsilon" <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 */

#include <stdio.h>  	/* printf(), puts(), fopen(), fwrite() */
#include <stdlib.h> 	/* strtold() */
#include <stdarg.h> 	/* va_list, va_start(), va_end() */
#include <stdbool.h>	/* bool, true, false */
#include <pthread.h>	/* pthread_* */

#include "circularlist.h"
#include "modules/sampler.h"
#include "loader.h"
#include "mapper.h"
#include "types.h"
#include "utils.h"

#define line_t FLOAT[max.real]

struct pixel max;
list output_buffer;
long long unsigned thread_count, *queue, next_line = 0;
char *myself;

__attribute__((hot, always_inline)) static inline
void display (void) {
	/* queue */
	printf("Q: ");
	for (long long unsigned i = 0; i < thread_count; i++)
		printf("%llu, ", queue[i]);
	/* progress */
	printf("P: %llu/%llu (%0.2f%%), ", next_line, max.imag,
		100 * (float)next_line / (float)max.imag);
	/* buffer */
	long long unsigned used = list_used(output_buffer);
	long long unsigned length = list_length(output_buffer);
	printf("B: %llu/%llu (%0.2f%%)\x1b[K\r", used, length,
		100 * (float)used / (float)length);
	fflush(stdout);
}

pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_written = PTHREAD_COND_INITIALIZER;
FILE *output_file;
__attribute__((hot, always_inline)) static inline
long long unsigned output (list_buffer *line) {
	pthread_mutex_lock(&write_lock);

	while ((*line = list_read(output_buffer))) {
		fwrite(**line, sizeof(FLOAT), max.real, output_file);
		pthread_cond_broadcast(&data_written);
	}

	if (next_line == max.imag) {
		/* no work left */
		pthread_mutex_unlock(&write_lock);
		return (long long unsigned)-1;
	}

	*line = list_get_write_ptr(output_buffer);
	if (NULL == **line) **line = new(line_t);

	display();

	/* remember to increment inside the loop, which requires returning a copy */
	long long unsigned copy = next_line++;

	pthread_mutex_unlock(&write_lock);

	return copy;
}

COMPLEX pixelsize;
COMPLEX ratio;
static sampler(sample);
__attribute__((hot, always_inline)) static inline
void iterate_line (list_buffer *line, long long unsigned imag) {
	COMPLEX point;
	struct pixel this = { .imag = imag };

	for (this.real = 0; this.real < max.real; this.real++) {
		point = pixel2vector(&this, &pixelsize, &ratio);
		((FLOAT *)**line)[this.real] = sample(&point);
	}

	list_mark_ready(*line);
}

list_buffer *thread_buffers;
static void *thread (void *ptr) {
	long long unsigned i = (long long unsigned)ptr;
	list_buffer *line = &thread_buffers[i];
	while ((long long unsigned)-1 != queue[i]) {
		iterate_line(line, queue[i]);
		queue[i] = output(line);
	}
	queue[i] = max.imag;
	return NULL;
}

__attribute__((cold, noreturn, always_inline)) static inline
void usage (void) {
	puts("Threaded fractal sampler\n");
	printf("Usage: %s THREADS WIDTH HEIGHT OUTFILE SAMPLER ARGS\n\n", myself);
	puts("	THREADS	how many threads to spawn");
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
	if (6 > argc) usage();

	thread_count = safe_strtoull(argv[1], NULL, 0, "THREADS", &usage);
	max.real = safe_strtoull(argv[2], NULL, 0, "WIDTH", &usage);
	max.imag = safe_strtoull(argv[3], NULL, 0, "HEIGHT", &usage);

	/* run sampler argument checks before creating the output file */
	sample = get_sampler(&argv[5]);

	/* create the output file */
	output_file = fopen(argv[4], "w");

	/* reverse vertical axis so positive = up */
	ratio = 1 - (FLOAT)max.imag / max.real * I;

	/* cache some math */
	pixelsize = calculate_pixelsize(&max, &ratio);

	/* allocate process tracking space */
	queue = new(long long unsigned [thread_count]);
	pthread_t *threads = new(pthread_t[thread_count]);

	printf("spinning up %llu threads\n", thread_count);

	/* allocate output buffer */
	output_buffer = new_list(thread_count);
	thread_buffers = new(list_buffer[thread_count]);

	next_line = thread_count;
	for (long long unsigned i = 0; i < thread_count; i++) {
		thread_buffers[i] = list_get_write_ptr(output_buffer);
		*(thread_buffers[i]) = new(line_t);
		queue[i] = i;
		pthread_create(&threads[i], NULL, thread, (void *)i);
	}
	display();

	for (long long unsigned i = 0; i < thread_count; i++)
		while (pthread_join(threads[i], NULL));

	display();
	puts("\nDone!");
	fflush(stdout);

	free(threads);
	free(queue);
	free(thread_buffers);
	delete_list(output_buffer);
	fclose(output_file);

	return 0;
}

/* render.c/v3.0a3 - final RGB renderer for complexfractals.
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * Distribute Unmodified -- http://pegasus.pimpninjas.org/license
 *
 *  CHANGELOG:
 *  3.0a1 -- first version with a separated sampler/renderer.
 *  3.0a2 -- fixed bug where "in-set" indicator was ignored.
 *  3.0a3 -- added logarithm command line options.
 *
 *  TODO:
 *  -- Add a way to specify the color for samples that are in-set.
 */

#define _POSIX_SOURCE 	/* fileno() */
#include <stdio.h>    	/* printf(), puts(), FILE, fopen(), fwrite(), fclose(), fileno() */
#include <stdlib.h>   	/* exit(), atoi(), atof() */
#include <sys/mman.h> 	/* mmap() */
#include <sys/types.h>	/* fstat(), open(), close() */
#include <sys/stat.h> 	/* fstat(), open(), close() */
#include <fcntl.h>    	/* open(), close() */
#include <unistd.h>   	/* fstat() */
#include <math.h>     	/* log() */
#include <string.h>   	/* strcmp() */

typedef struct {
	size_t size;
	int fd, shift;
	long double divider;
	char *map;
} palette;

__attribute__((noreturn))
void usage(const char *myself) {
	printf("Usage: %s [OPTION] samplefile palfile shift divider outfile\n\n", myself);
	puts("\t-l\tapply the natural logarithm to each sample before mapping");
	puts("\t\t(flatten more)");
	puts("\t-2\tapply the base-2 logarithm to each sample before mapping");
	puts("\t\t(flatten less)");
	puts("\t\tOnly one logarithm may be applied at a time (for now)");
	exit(1);
}

__attribute__((noreturn))
void fail (const char *msg) {   /* report function failures */
	perror(msg);
	exit(1);
}

long double nothing (long double x) { return x; }

int main (int argc, char **argv) {
	FILE *infile, *outfile;
	palette map;
	long double (*flatten)(long double) = &nothing;

	if (1 < argc) {
		if (!strcmp("-l", argv[1])) flatten = &logl;
		if (!strcmp("-2", argv[1])) flatten = &log2l;
		if (flatten != &nothing) { argc--; argv++; }
	}

	if (6 != argc) usage(argv[0]);

	if (NULL == (infile = fopen(argv[1], "r"))) fail(argv[1]);
	if (-1 == (map.fd = open(argv[2], O_RDONLY))) fail(argv[2]);
	{ /* magically allocate and free a temporary structure */
		struct stat info;
		fstat(map.fd, &info);
		map.size = (size_t)info.st_size;
	}
	map.map = mmap(NULL, map.size, PROT_READ, MAP_SHARED, map.fd, (size_t)0);
	map.shift = atoi(argv[3]);
	map.divider = atof(argv[4]);
	outfile = fopen(argv[5], "w");

	for (;;) { /* ever */
		static long double sample;
		const unsigned char black[] = { 0, 0, 0 };
		if (1 != fread(&sample, sizeof(long double), (size_t)1, infile)) {
			if (feof(infile)) break; /* clean EOF, all done */
			else fail(argv[1]); /* shit broke, throw a fit */
		} /* got data, do work */
		if (0 <= sample)
			fwrite(map.map + 3 * (unsigned int)((long double)map.size + flatten(sample) * (long double)map.size / map.divider + map.shift) % map.size, (size_t)1, (size_t)3, outfile);
		else
			fwrite(black, (size_t)1, (size_t)3, outfile);
	}
	fclose(infile);
	close(map.fd);
	fclose(outfile);

	return 0;
}

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

#define BUFSIZE 4096 / sizeof(long double)

typedef struct { char r, g, b; } rgb24;

typedef struct {
	size_t size;
	int fd, shift;
	long double divider;
	rgb24 *map;
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
	map.size /= 3;
	map.shift = atoi(argv[3]);
	map.divider = strtold(argv[4], NULL);
	outfile = fopen(argv[5], "w");

	size_t records;
	do {
		long double inbuffer[BUFSIZE];
		rgb24 outbuffer[BUFSIZE];
		rgb24 black = { 0, 0, 0 };

		records = fread(inbuffer, sizeof(long double), BUFSIZE, infile);

		// if short read, and not EOF infile, shit broke, throw a fit.
		if (BUFSIZE != records && !feof(infile)) fail(argv[1]);

		// convert samples to rgb24 colors
		for (size_t i = 0; i < records; i++)
			outbuffer[i] = inbuffer[i] < 0 ? black : map.map[(size_t)(
				map.size + fabsl(flatten(inbuffer[i])) *
				map.size / map.divider + map.shift
			) % map.size];

		fwrite(outbuffer, sizeof(rgb24), records, outfile);
	} while (BUFSIZE == records);
	fclose(infile);
	close(map.fd);
	fclose(outfile);

	return 0;
}

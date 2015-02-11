/* palette.c/v4.2 - palette generator for complexfractals
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 *
 *  Changelog:
 *  v4.0 - Decoupled channels
 *  v4.1 - Added verbose flag, suppressed noisy debug info
 *  v4.2 - Rewrote some, added comments, verified that the RGB directive works
 */

#include <stdio.h>      	/* perror(), puts(), printf(), fopen(), fgets(), feof(), sscanf(), fclose(), fwrite() */
#include <stdlib.h>     	/* exit(), realloc(), calloc(), free(), size_t */
#include <stdarg.h>     	/* va_list, va_start(), vprintf(), va_end() */
#ifdef _WIN32
#	include <stdint.h>  	/* uint8_t */
#	define PRIuSIZET "Iu"
#else
#	include <inttypes.h>	/* uint8_t */
#	define PRIuSIZET "zu"
#endif
#include <sys/types.h>  	/* off_t */
#include <string.h>     	/* strcmp() */
#include <math.h>       	/* cos(), fmod() */
#include "constants.h"

#define CHANNELS 3
#define RED 0
#define GRN 1
#define BLU 2
/* 128 bytes is more than we should ever need for a single directive.
 * We'll just croak if they exceed it, for now.
 */
#define LINELEN 128

const char *channel[] = {"RED", "GRN", "BLU"};

struct RGB24 {
	uint8_t y[CHANNELS];
};

struct GRADIENT {
	struct RGB24 *x;
	size_t length;
};

struct POINT {
	double x;
	double y;
};

struct CHANNEL {
	struct POINT *points;
	size_t length;
};

/* runtime debug-or-not */
int nothing (const char *f, ...) { return (long)f; }
int (*debug)(const char *f, ...) = &nothing;

/* report function failures */
__attribute__((noreturn))
void fail (const char *msg) {
	perror(msg);
	exit(1);
}

/* report errors */
__attribute__((noreturn))
void die (const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	puts(".");
	exit(1);
}

/* This function generates a cosine interpolated multi-channel cyclical
 * waveform for use as a palette. Chosen for simplicity (no, really),
 * precision, and actually quite nice results compared to other methods,
 * like linear interpolation (ugly, even modified), splines (complicated)
 * or curve fitting (in some cases, horrendously imprecise).
 */
struct GRADIENT *generate_palette (const struct CHANNEL *channels, struct GRADIENT *gradient) {
	uint8_t c;	/* only three channels, after all */
	size_t i;	/* current point in overall interpolation */
	size_t a;	/* starting point for this segment of interpolation */
	size_t b;	/* ending point for this segment of interpolation */

	for (c = 0; c < CHANNELS; c++) {
		/* point A = cyclically previous point */
		a = channels[c].points[0].x > 0 ? channels[c].length - 1 : 0;
		/* point B = cyclically next point */
		b = (a + 1) % channels[c].length;
		/* for each point "i" in the palette... */
		for (i = 0; i < gradient->length; i++) {
			/* y = vertical coordinate ("magnitude") of the point */
			double y;
			/* x = horizontal coordinate ("timestamp") of the point */
			double x = (double)i / (double)gradient->length;
			/* dx = distance between A and B on the X axis (delta t of [a, b]) */
			double dx = fmod(1 + channels[c].points[b].x - channels[c].points[a].x, 1.0);

			/* Move forward through the given channels until we
			 * find one we haven't surpassed yet. Any channels
			 * we skip entirely are lost in the noise of the
			 * waveform. Such is the nature of the beast.
			 */
			while (x > channels[c].points[b].x) {
				b = ((a = (a + 1) % channels[c].length) + 1) % channels[c].length;
				if (channels[c].points[b].x < channels[c].points[a].x) channels[c].points[b].x++;
			}

			x -= channels[c].points[a].x;	/* shift X */
			if (dx) /* no divide by zero please */
				x /= dx;	/* scale X */
			else if (channels[c].points[a].x == channels[c].points[b].x)
				puts("Warning: Two points specified on the same channel at the same time.");
			y = (cos(M_PI * x) + 1) / 2;	/* interpolate! */
			y *= channels[c].points[a].y - channels[c].points[b].y;	/* scale Y */
			y += channels[c].points[b].y;	/* shift Y */

			gradient->x[i].y[c] = (uint8_t)(y * 255 + .5);
		} /* for i < gradient->length */
	} /* for c < CHANNELS */

	return gradient;
}

__attribute__((noreturn))
void usage (const char *myself) {
	printf("Usage: %s infile outfile\n\n", myself);
	puts("\t-v\tshow debug output");
	exit(1);
}

/* allocate storage for, and store, specified point */
struct CHANNEL *add_point (struct CHANNEL *in, double x, double y) {
	struct POINT *new;
	new = realloc(in->points, (in->length + 1) * sizeof(*(in->points)));
	if (!new)
		fail("failed to allocate memory");
	in->points = new;
	in->points[in->length].x = x;
	in->points[in->length].y = y;
	in->length++;
	return in;
}

/* allocate storage for, and store, channels as
 * specified, for each channel, all at once
 */
void add_syncpoint (struct CHANNEL *channels, double x, char *line, off_t *off) {
	uint8_t c;
	double y;
	for (c = 0; c < CHANNELS; c++) {
		line += *off;
		sscanf(line, "%lf%jn", &y, off);
		debug(",%f", y);
		add_point(&(channels[c]), x, y);
	}
}

/* print the parsed palette directives */
void printparsed (struct CHANNEL *channels) {
	uint8_t c;
	size_t i;
	for (c = 0; c < CHANNELS; c++)
		for (i = 0; i < channels[c].length; i++)
			printf(
					"%s: %f/%f\n",
					channel[c],
					channels[c].points[i].x,
					channels[c].points[i].y
			);
}

/* print the final interpolated palette data */
void printoutput (struct GRADIENT *gradient) {
	size_t i;
	for (i = 0; i < gradient->length; i++)
		printf(
			"%lf: %d, %d, %d\n",
			(double)i / (double)gradient->length,
			gradient->x[i].y[RED],
			gradient->x[i].y[GRN],
			gradient->x[i].y[BLU]
		);
}

int main (int argc, char **argv) {
	uint8_t c;
	size_t i;
	struct CHANNEL channels[CHANNELS];
	struct GRADIENT gradient;
	FILE *infile;
	FILE *outfile;

	/* handle the verbose flag */
	if (argc > 1 && !strcmp("-v", argv[1])) {
		debug = &printf;
		argc--;
		argv[1] = argv[0];
		argv++;
	}

	if (argc < 2)
		usage(argv[0]);

	/* init all storage */
	gradient.length = 0;
	for (c = 0; c < CHANNELS; c++)
		channels[c].length = 0;
	for (c = 0; c < CHANNELS; c++)
		channels[c].points = NULL;

	/* read and parse the input file */
	if (NULL == (infile = fopen(argv[1], "r")))
		fail(argv[1]);
	for (;;) {
		/* x, y = coordinates of point */
		double x, y;
		/* process input one line at a time
		 * LINELEN bytes is more than we should ever need
		 */
		char line[LINELEN];
		/* off = character offset in current line
		 * used only in RGB directive
		 */
		off_t off = 0;

		/* read a line */
		if (NULL == fgets(line, LINELEN, infile)) {
			if (feof(infile))
				break;
			fail("reading input file");
		}
		/* skip whitespace */
		for (i = 0; ' ' == line[i] || '\t' == line[i]; i++);
		/* skip blank lines */
		if ('\n' == line[i])
			continue;
		/* skip comments */
		if ('#' == line[i]) {
			debug("%s", &line[i]);
			fflush(stdout);
			continue;
		}
		/* figure out how long our results will be */
		if (sscanf(line+i, "LEN%" PRIuSIZET, &gradient.length)) {
			debug("gradient length: %llu\n", gradient.length);
			/* could allocate storage here, but we'll wait until
			 * the entire file is processed successfully.
			 */
			continue;
		}

		/* process actual control point directives */
		if (sscanf(line+i, "RED%lf%lf", &x, &y)) {
			debug("read RED control point %lf/%lf\n", x, y);
			add_point(&channels[RED], x, y);
		} else if (sscanf(line+i, "GRN%lf%lf", &x, &y)) {
			debug("read GRN control point %lf/%lf\n", x, y);
			add_point(&channels[GRN], x, y);
		} else if (sscanf(line+i, "BLU%lf%lf", &x, &y)) {
			debug("read BLU control point %lf/%lf\n", x, y);
			add_point(&channels[BLU], x, y);
		} else if (sscanf(line+i, "RGB%lf%jn", &x, &off)) {
			/* Tested and fixed. Not sure it's the best way, though. */
			debug("read RGB control point %lf", x);
			add_syncpoint(channels, x, line+i, &off);
			if (&printf == debug) putchar('\n');
		} else
			printf("Warning: Unknown statement in config file: %s\n", line+i);
	}
	fclose(infile);

	/* debug? print the parsed directives */
	if (&printf == debug)
		printparsed(channels);
	/* make sure we got a gradient length */
	if (!gradient.length) die("Config must specify a gradient length (LEN)");
	/* this could be allocated above */
	gradient.x = calloc(gradient.length, sizeof(*gradient.x));

	/* actually generate the palette */
	generate_palette(channels, &gradient);
	/* debug? print the final palette data */
	if (&printf == debug)
		printoutput(&gradient);

	/* write the palette to outfile */
	outfile = fopen(argv[2], "w");
	fwrite(gradient.x, sizeof(*gradient.x), gradient.length, outfile);
	fclose(outfile);

	/* clean up */
	free(gradient.x);
	for (c = 0; c < CHANNELS; c++)
		free(channels[c].points);

	return 0;
}

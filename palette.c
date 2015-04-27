#include <stdio.h>      	/* perror(), puts(), printf(), fopen(), fgets(), feof(), sscanf(), fclose(), fwrite() */
#include <stdlib.h>     	/* exit(), realloc(), calloc(), free(), size_t */
#ifdef _WIN32
#	include <stdint.h>  	/* uint8_t */
#	define PRIuSIZET "Iu"
#else
#	include <inttypes.h>	/* uint8_t */
#	define PRIuSIZET "zu"
#endif
#include <stddef.h>     	/* ptrdiff_t */
#include <math.h>       	/* cos(), fmod() */

#include "constants.h"
#include "utils.h"

#define CHANNELS 3
#define RED 0
#define GRN 1
#define BLU 2
/* 128 bytes is more than we should ever need for a single directive.
 * We'll just croak if they exceed it, for now.
 */
#define LINELEN 128

const char *channel[] = { "RED", "GRN", "BLU" };

struct rgb24 {
	uint8_t y[CHANNELS];
};

struct gradient {
	struct rgb24 *x;
	size_t length;
};

struct point {
	double x;
	double y;
};

struct channel {
	struct point *points;
	size_t length;
};

/* This function generates a cosine interpolated multi-channel cyclical
 * waveform for use as a palette. Chosen for simplicity (no, really),
 * precision, and actually quite nice results compared to other methods,
 * like linear interpolation (ugly, even modified), splines (complicated)
 * or curve fitting (in some cases, horrendously imprecise).
 */
struct gradient *generate_palette (const struct channel *channels, struct gradient *gradient) {
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

			/* Move forward through the given points until we
			 * find one we haven't surpassed yet. Any points
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
	printf("Usage: %s [-v] infile outfile\n\n", myself);
	puts("\t-v\tshow debug output");
	exit(1);
}

/* allocate storage for, and store, specified point */
struct channel *add_point (struct channel *in, double x, double y) {
	struct point *new;
	new = realloc(in->points, (in->length + 1) * sizeof(*(in->points)));
	if (!new) fail("failed to allocate memory");
	in->points = new;
	in->points[in->length].x = x;
	in->points[in->length].y = y;
	in->length++;
	return in;
}

/* allocate storage for, and store, channels as
 * specified, for each channel, all at once
 */
void add_syncpoint (struct channel *channels, double x, char *line, ptrdiff_t *diff) {
	uint8_t c;
	double y;
	for (c = 0; c < CHANNELS; c++) {
		line += *diff;
		sscanf(line, "%lf%tn", &y, diff);
		debug(",%f", y);
		add_point(&(channels[c]), x, y);
	}
}

/* print the parsed palette directives */
void printparsed (struct channel *channels) {
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
void printoutput (struct gradient *gradient) {
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
	struct channel channels[CHANNELS];
	struct gradient gradient;
	FILE *infile;
	FILE *outfile;

	/* handle the verbose flag */
	if (1 < argc && *(uint16_t *)"-v" == *(uint16_t *)argv[1]) {
		enable_debug();
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
		/* syncdiff = character offset in current line
		 * used only in RGB directive
		 */
		ptrdiff_t syncdiff = 0;

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
		} else if (sscanf(line+i, "RGB%lf%jn", &x, &syncdiff)) {
			/* Tested and fixed. Not sure it's the best way, though. */
			debug("read RGB control point %lf", x);
			add_syncpoint(channels, x, line+i, &syncdiff);
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

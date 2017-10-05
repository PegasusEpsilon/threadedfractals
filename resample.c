/* resample.c, from threadedfractals
 * by "Pegasus Epsilon" <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 */

#include <inttypes.h>	/* uint8_t, uint32_t */
#include <stdio.h>   	/* printf() */
#include <stdlib.h>  	/* exit() */
#include <string.h>  	/* strcmp() */

#include "utils.h"

#define CHANNELS 3
#define RED 0
#define GRN 1
#define BLU 2

typedef struct {
	uint8_t c[CHANNELS];
} RGB24;

__attribute__((noreturn))
void usage (char *myself) {
	printf("Usage: %s [-v] infile width factor outfile\n", myself);
	exit(1);
}

RGB24 downsample (size_t px_width, RGB24 **samples) {
	struct {
		uint32_t c[CHANNELS];
	} mixer;
	RGB24 *row, result;
	size_t x, y, d = px_width * px_width;

	mixer.c[RED] = mixer.c[GRN] = mixer.c[BLU] = 0;
	for (y = 0; y < px_width; y++) {
		row = samples[y];
		for (x = 0; x < px_width; x++) {
			mixer.c[RED] += row[x].c[RED];
			mixer.c[GRN] += row[x].c[GRN];
			mixer.c[BLU] += row[x].c[BLU];
		}
	}

	result.c[RED] = (uint8_t)(mixer.c[RED] / d);
	result.c[GRN] = (uint8_t)(mixer.c[GRN] / d);
	result.c[BLU] = (uint8_t)(mixer.c[BLU] / d);

	return result;
}

int main (int argc, char **argv) {
	size_t ibuf_len, obuf_len;
	size_t px_x, px_y, spx_y;	/* pixel x, y, subpixel row */
	size_t px_width, px_size;	/* in subpixel samples */
	size_t img_width;	/* in pixels */
	FILE *ifile, *ofile;
	RGB24 *ibuf, *obuf, **samples;

	/* enable debug output, maybe */
	if (argc > 5 && *(uint16_t *)"-v" == *(uint16_t *)argv[1])
		{ enable_debug(); argv[1] = argv[0]; argc--; argv++; }

	/* make sure we have the right amount of arguments */
	if (5 != argc) usage(argv[0]);

	/* process args */
	if (NULL == (ifile = fopen(argv[1], "r"))) fail(argv[1]);
	img_width = (size_t)atoi(argv[2]);
	px_width = (size_t)atoi(argv[3]);
	if (NULL == (ofile = fopen(argv[4], "w"))) fail(argv[4]);

	/* setup runtime constants */
	px_size = px_width * px_width;
	obuf_len = sizeof(RGB24) * img_width;
	ibuf_len = obuf_len * px_size;

	/* allocate memory */
	ibuf = (RGB24 *)malloc(ibuf_len);
	obuf = (RGB24 *)malloc(obuf_len);
	samples = (RGB24 **)malloc(sizeof(RGB24 *) * px_size);

	/* process infile */
	for (px_y = 0; ; px_y++) { /* ever */
		/* read a full line of supersampled pixels */
		if (1 != fread(ibuf, ibuf_len, (size_t)1, ifile))
			{ if (feof(ifile)) break; else fail(argv[1]); }

		debug("\rresampling line %d...", px_y + 1);
		fflush(stdout);

		/* downsample the all the pixels */
		for (px_x = 0; px_x < img_width; px_x++) {
			/* set up offsets to the supersamples in this pixel */
			for (spx_y = 0; spx_y < px_width; spx_y++)
				samples[spx_y] = ibuf + (img_width * px_width * spx_y) + (px_width * px_x);

			/* downsample it */
			obuf[px_x] = downsample(px_width, samples);
		}

		/* write the downsampled line */
		if (1 != fwrite(obuf, obuf_len, (size_t)1, ofile))
			fail(argv[4]);
		fflush(ofile);
	}
	debug("Done.\n");

	return 0;
}

/* zlib-only PNG writer
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * (C) 2012-2017 Distribute Unmodified
 */

#include <stddef.h>   	/* size_t */
#include <stdio.h>    	/* fopen(), fread(), fwrite(), fclose() */
#include <stdlib.h>   	/* exit() */
#include <stdint.h>   	/* uint(8|16|32)_t */
#include <zlib.h>     	/* z_stream, deflateInit2(), deflate(), crc32() */
#include <arpa/inet.h>	/* htonl() - noop on network-order machines */

#include "utils.h"

/* unalienate crc32 -- native types please! */
#define crc32(x, y, z) (uint32_t)crc32((uLong)x, (Bytef *)y, (uInt)z)
#define Z_CHUNK (size_t)(32 << 10) /* 32kB */

const uint8_t png_magic[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const uint8_t zlib_header[] = { 0x78, 0xda };

struct png_chunk {
	uint32_t size;
	uint32_t type;
} __attribute__((packed));

struct png_ihdr {
	struct png_chunk header;
	uint32_t width;
	uint32_t height;
	uint8_t bit_depth;
	uint8_t color_type;
	uint8_t compression_method;
	uint8_t filter_method;
	uint8_t interlace_method;
} __attribute__((packed));

struct png_iend {
	struct png_chunk header;
} __attribute__((packed));

__attribute__((cold, noreturn))
void usage (const char *restrict const myself) {
	printf("Usage: %s [-v] [-32] infile width height outfile\n", myself);
	exit(1);
}

__attribute__((hot, always_inline)) static inline
void crc_write (const void *data, size_t size, FILE *stream, uint32_t *crc) {
	*crc = crc32(*crc, data, size);
	if (1 != fwrite(data, size, 1, stream) || ferror(stream)) fail("crc_write");
}

__attribute__((hot, always_inline)) static inline
void fwrite_chunk (
	struct png_chunk *restrict const chunk, FILE *restrict const file
) {
	/* save this for later */
	size_t size = chunk->size;
	/* CRC does not include the chunk size field */
	uint32_t crc = htonl(crc32(0, &chunk->type, (size - sizeof(chunk->size))));
	/* chunk size does not count header */
	chunk->size = htonl(size - sizeof(struct png_chunk));
	/* write chunk */
	fwrite(chunk, size, 1, file);
	/* write CRC */
	fwrite(&crc, sizeof(uint32_t), 1, file);
}

__attribute__((hot, always_inline)) static inline
int output (
	z_stream *restrict const stream, const int flush,
	FILE *restrict const file, uint32_t *restrict const crc
) {
	static uint8_t buffer[Z_CHUNK];
	size_t ready;

	do {
		stream->next_out = buffer;
		stream->avail_out = Z_CHUNK;
		if (Z_STREAM_ERROR == deflate(stream, flush)) die("zlib killed itself");
		if ((ready = Z_CHUNK - stream->avail_out))
			crc_write(buffer, ready, file, crc);
	} while (0 == stream->avail_out);

	return !stream->avail_out;
}

int main (int argc, char **argv) {
	FILE *ifile, *ofile;
	long idat_start, idat_size;
	uint32_t crc = 0, height, width, scanline;
	uint8_t channels = 3;

	z_stream stream = {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
		.next_in = Z_NULL,
	};

	struct png_ihdr ihdr = {
		.header = {
			.size = sizeof(struct png_ihdr),
			.type = *(uint32_t *)"IHDR"
		},
		/* we can easily encode other pixel formats, but we only care about
		 * 24bpp truecolor images at the moment.
		 */
		.bit_depth = 8,	/* 8 bits per channel */
		.color_type = 2,	/* RGB (24bpp, PNG24) */
		/* the PNG spec allows only one compression method. despite the spec's
		 * statements, the stream must be a full zlib stream, with zlib header,
		 * adler32 footer (for the decompressed data), and an extra additional
		 * CRC32 checksum (for compressed data) per the PNG chunk spec proper.
		 */
		.compression_method = 0,	/* zlib/32767 */
		/* we don't bother with any filtering, just shoving the raw pixel data
		 * to zlib. We *may* get better results with filtering, but exploring
		 * that problem space takes more time than we're willing to spend on
		 * this simple application of the PNG spec. Besides, we beat every
		 * PNG compressor I've found, (except optipng, which we generally tie),
		 * already.
		 */
		.filter_method = 0,	/* adaptive */
		/* interlacing brings a lot of code complexity,
		 * with very limited (if any) benefit. don't bother.
		 */
		.interlace_method = 0	/* progressive scan */
		/* NOTE: Despite the radical simplification of the potential outputs by
		 * these self-imposed restraints on filtering and interlacing, we still
		 * beat ImageMagick's PNG writer by a decent margin.
		 */
	};

	struct png_iend iend = { .header = {
		.size = sizeof(struct png_iend),
		.type = *(uint32_t *)"IEND"
	} };

	/* Deal with optional command-line arguments */
	while (argv[1] && '-' == *argv[1]) {
		if (*(uint32_t *)"-32" == *(uint32_t *)argv[1]) {
			/* RGBA (32bpp, PNG32) */
			debug("Enabled PNG32...");
			ihdr.color_type = 6;
			channels = 4;
		} else if (*(uint16_t *)"-v" == *(uint16_t *)argv[1])
			enable_debug();
		else printf("Unknown flag %s ignored.\n", argv[1]);
		argv[1] = argv[0];
		argc--;
		argv++;
	}

	/* Make sure we've still got our args */
	if (4 > argc) usage(argv[0]);

	/* Open our files */
	if (NULL == (ifile = fopen(argv[1], "r"))) fail(argv[1]);
	if (NULL == (ofile = fopen(argv[4], "w"))) fail(argv[4]);

	/* Process width and height arguments */
	width = atol(argv[2]);
	height = atol(argv[3]);
	scanline = width * channels;

	/* Write the PNG magic */
	fwrite(png_magic, (size_t)8, (size_t)1, ofile);

	/* Write the IHDR */
	ihdr.width = htonl(width);
	ihdr.height = htonl(height);
	fwrite_chunk((struct png_chunk *)&ihdr, ofile);

	debug("writing %s, %sx%s...\n", argv[4], argv[2], argv[3]);

	/* Build a single IDAT chunk as a stream.
	 * Read, compress, write, for maximum compression efficiency.
	 * First, the size, which we don't yet know, because we haven't
	 * compressed anything yet -- so we save the offset here...
	 */
	idat_size = ftell(ofile);
	/* ...skip it, and come back to fill it in after compression is complete.
	 * If your platform doesn't support seek past end of file,
	 * just write (uint32_t)0 instead, overwriting it later as normal.
	 */
	fseek(ofile, sizeof(uint32_t), SEEK_CUR);

	/* write "IDAT" */
	crc_write("IDAT", 4, ofile, &crc);

	/* Start counting IDAT size from here */
	idat_start = ftell(ofile);

	/* Start up zlib stream compressor */
	if (Z_OK != deflateInit2(&stream, 9, Z_DEFLATED, 15, 9, Z_DEFAULT_STRATEGY))
		die("failed to initialize zlib");

	/* Compress the stream */
	uint8_t input[Z_CHUNK] = { 0 };
	for (uint32_t i = 0; i < height; i++) {
		input[0] = 0; // row filter type zero = no filter
		stream.next_in = input;
		stream.avail_in = (uInt)1;
		output(&stream, Z_NO_FLUSH, ofile, &crc);

		uint32_t chunk;
		for (uint32_t j = 0; j != scanline; j += chunk) {
			uint32_t left = scanline - j;
			chunk = left > Z_CHUNK ? Z_CHUNK : left;
			stream.next_in = input;
			stream.avail_in = fread(input, 1, chunk, ifile);
			if (ferror(ifile)) fail(argv[1]);
			output(&stream, Z_NO_FLUSH, ofile, &crc);
		}
	}
	/* Finish up */
	output(&stream, Z_FINISH, ofile, &crc);
	fclose(ifile);
	deflateEnd(&stream);

	/* calculate IDAT size */
	idat_start = ftell(ofile) - idat_start;
	debug("IDAT size: %lu (%08x) bytes\n", idat_start, idat_start);
	idat_start = htonl(idat_start);

	/* Write the CRC, in network byte order */
	crc = htonl(crc);
	fwrite(&crc, sizeof(crc), 1, ofile);

	/* Write the IEND chunk */
	fwrite_chunk((struct png_chunk *)&iend, ofile);

	/* go back and fill in the IDAT size */
	fseek(ofile, idat_size, SEEK_SET);
	fwrite(&idat_start, sizeof(uint32_t), 1, ofile);

	/* Finish up */
	fclose(ofile);
	debug("done.\n");

	return 0;
}

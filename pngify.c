/* zlib-only PNG writer
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * (C) 2012 Distribute Unmodified
 */

#include <stddef.h>   	/* size_t */
#include <stdio.h>    	/* fopen(), fread(), fwrite(), fclose() */
#include <stdlib.h>   	/* exit() */
#include <stdarg.h>   	/* va_list, va_start(), vprintf(), va_end() */
#include <stdint.h>   	/* uint(8|16|32)_t */
#include <zlib.h>     	/* z_stream, deflateInit2(), deflate(), crc32() */
#include <arpa/inet.h>	/* htonl() - noop on network-order machines */

/* unalienate crc32 -- native types please! */
#define crc32(x, y, z) (uint32_t)crc32((uLong)x, (Bytef *)y, (uInt)z)
#define Z_CHUNK (size_t)(32 << 10) /* 32kB */

const uint8_t png_magic[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const uint8_t zlib_header[] = { 0x78, 0xda };

struct png_chunk {
	uint32_t size;
	uint32_t typecode;
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

__attribute__((pure cold noreturn))
void usage (const char *restrict const myself) {
	printf("Usage: %s infile width height outfile\n", myself);
	exit(1);
}

/* Print errno error message and exit with errorlevel */
__attribute__((cold noreturn)) /* can it be pure when it calls perror? */
void fail (const char *restrict const msg) {
	perror(msg);
	exit(1);
}

/* Print custom message and exit with errorlevel */
__attribute__((pure cold noreturn))
void die (const char *restrict const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	puts(".");
	exit(1);
}

__attribute__((hot pure always_inline))
static inline void fwrite_chunk (
	struct png_chunk *restrict const chunk,
	FILE *restrict const file
) {
	/* save this for later */
	size_t size = chunk->size;
	/* CRC does not include the chunk size field */
	uint32_t crc = htonl(crc32(0, &chunk->typecode, (size - sizeof(chunk->size))));
	/* chunk size does not count header */
	chunk->size = htonl(size - sizeof(struct png_chunk));
	/* write chunk */
	fwrite(chunk, size, 1, file);
	/* write CRC */
	fwrite(&crc, sizeof(uint32_t), 1, file);
}

__attribute__((hot always_inline))
static inline int output (
	z_stream *restrict const stream,
	const int flush,
	FILE *restrict const file,
	uint32_t *restrict const crc
) {
	static uint8_t buffer[Z_CHUNK];
	size_t ready;

	stream->next_out = buffer;
	stream->avail_out = Z_CHUNK;

	if (Z_STREAM_ERROR == deflate(stream, flush)) die("zlib killed itself");
	if ((ready = Z_CHUNK - stream->avail_out)) {
		*crc = crc32(*crc, buffer, ready);
		if (1 != fwrite(buffer, ready, 1, file)) fail("write");
	}
	return !stream->avail_out;
}

int debug_off (const char *restrict const fmt, ...) { return (long)fmt; }
int debug_on  (const char *restrict const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vprintf(fmt, args);
	va_end(args);
	fflush(stdout);
	return ret;
}

int main (int argc, char **argv) {
	int (*debug)(const char *, ...) = &debug_off;
	FILE *ifile, *ofile;
	long idat_start, idat_end, idat_size;
	uint32_t crc, height, width;

	z_stream stream = {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
		.next_in = Z_NULL,
	};

	struct png_ihdr ihdr = {
		.header = {
			.size = sizeof(struct png_ihdr),
			.typecode = *(uint32_t *)"IHDR"
		},
		.bit_depth = 8,	/* 24bpp */
		.color_type = 2,	/* color */
		.compression_method = 0,	/* zlib/32767 */
		.filter_method = 0,	/* adaptive */
		.interlace_method = 0	/* progressive scan */
	};

	struct png_iend iend = { .header = {
		.size = sizeof(struct png_iend),
		.typecode = *(uint32_t *)"IEND"
	} };

	/* Turn on verbose, maybe */
	if (1 < argc) {
		if (*(uint16_t *)argv[1] == *(uint16_t *)"-v") debug = &debug_on;
		if (debug != &debug_off) { argv[1] = argv[0]; argc--; argv++; }
	}

	/* Make sure we've got our args */
	if (4 > argc) usage(argv[0]);

	/* Open our files */
	if (NULL == (ifile = fopen(argv[1], "r"))) fail(argv[1]);
	if (NULL == (ofile = fopen(argv[4], "w"))) fail(argv[4]);

	/* Process width and height arguments */
	width = atol(argv[2]);
	height = atol(argv[3]);
	if (3 * width > Z_CHUNK)
		die("Encoding images wider than %d pixels is not implemented", Z_CHUNK / 3);

	/* Write the PNG magic */
	fwrite(png_magic, (size_t)8, (size_t)1, ofile);

	/* Write the IHDR */
	ihdr.width = htonl(width);
	ihdr.height = htonl(height);
	fwrite_chunk((struct png_chunk *)&ihdr, ofile);

	debug("writing %s, %sx%s...", argv[4], argv[2], argv[3]);

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

	/* Start up CRC calculation */
	crc = crc32(0, "IDAT", 4);

	/* Write the IDAT typecode */
	if (1 != fwrite("IDAT", 4, 1, ofile) || ferror(ofile)) fail(argv[4]);

	/* Start counting IDAT size from here */
	idat_start = ftell(ofile);

	/* Start up zlib stream compressor */
	if (Z_OK != deflateInit2(&stream, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY))
		die("failed to initialize zlib");

	/* Remember to CRC every single byte */
	crc = crc32(crc, zlib_header, 2);
	fwrite(zlib_header, 2, 1, ofile);

	/* Compress the stream */
	uint8_t input[Z_CHUNK] = { 0 };
	for (uint32_t i = 0; i < height; i++) {
		stream.next_in = input;
		stream.avail_in = (uInt)1 + 3 * fread(&input[1], 3, width, ifile);
		if (ferror(ifile)) fail(argv[1]);
		while (output(&stream, Z_NO_FLUSH, ofile, &crc));
	}
	/* Finish up
	 * NOTE: I don't know why this final single null is actually required.
	 * It's not in the PNG spec, as far as I can see, but leaving it out,
	 * and just flushing the pending compressed data to disk instead,
	 * actually breaks the output file.
	 */
	stream.next_in = input;
	stream.avail_in = 1;
	output(&stream, Z_FINISH, ofile, &crc);
	fclose(ifile);
	deflateEnd(&stream);

	/* Go back and write the IDAT size,
	 * which we left blank before,
	 * now that we know what it is.
	 */
	idat_end = ftell(ofile);
	idat_start = htonl(idat_end - idat_start);
	fseek(ofile, idat_size, SEEK_SET);
	fwrite(&idat_start, sizeof(uint32_t), 1, ofile);
	fseek(ofile, idat_end, SEEK_SET);

	/* Write the CRC, in network byte order */
	crc = htonl(crc);
	fwrite(&crc, sizeof(crc), 1, ofile);

	/* Write the IEND chunk */
	fwrite_chunk((struct png_chunk *)&iend, ofile);

	/* Finish up */
	fclose(ofile);
	debug("done.\n");

	return 0;
}

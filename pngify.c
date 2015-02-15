/* zlib-only PNG writer
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * (C) 2012 Distribute Unmodified
 *
 */

#include <stddef.h>   	/* size_t */
#include <stdio.h>    	/* fopen(), fread(), fwrite(), fclose() */
#include <stdlib.h>   	/* exit() */
#include <stdarg.h>   	/* va_list, va_start(), vprintf(), va_end() */
#include <stdint.h>   	/* uint(8|16|32)_t */
#include <zlib.h>     	/* z_stream, deflateInit2(), deflate(), crc32() */
#include <arpa/inet.h>	/* htonl() */

/* unalienate crc32 -- native types please! */
#define crc32(x, y, z) (uint32_t)crc32((uLong)x, (Bytef *)y, (uInt)z)
#define Z_CHUNK (size_t)(32 << 10) /* 32kB */

const uint8_t PNG_header[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
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

__attribute__((noreturn))
void usage (const char *myself) {
	printf("Usage: %s infile width height outfile\n", myself);
	exit(1);
}

__attribute__((noreturn))
void fail (const char *msg) {	/* report function failures */
	perror(msg);
	exit(1);
}

__attribute__((noreturn))
void die (const char *fmt, ...) {	/* report errors */
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	puts(".");
	exit(1);
}

void fwrite_chunk (void *chunk, FILE *ofile) {
	/* finally checksumming properly. */
	uint32_t *csz, crc;
	size_t size;
	void *ctc;
	csz = chunk;	/* cast chunk as uint32_t -> chunk size */
	size = *csz;	/* save length */
	ctc = &((struct png_chunk *)chunk)->typecode;	/* CRC starts here */
	crc = htonl(crc32(0, ctc, (size - sizeof(uint32_t))));	/* do CRC */
	*csz = htonl(*csz - sizeof(PNG_header));	/* fix length */
	fwrite(chunk, size, 1, ofile);	/* write chunk */
	fwrite(&crc, sizeof(uint32_t), 1, ofile);	/* write CRC */
}

static inline int output (z_stream *stream, int flush, FILE *file, uint32_t *crc) {
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

int debug_off (const char *fmt, ...) { return (long)fmt; }
int debug_on  (const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vprintf(fmt, args);
	va_end(args);
	fflush(stdout);
	return ret;
}

int main (int argc, char **argv) {
	z_stream stream;
	FILE *ifile, *ofile;
	struct png_ihdr ihdr = { .header = {
		.size = sizeof(struct png_ihdr),
		.typecode = *(uint32_t *)"IHDR"
	} };
	struct png_iend iend = { .header = {
		.size = sizeof(struct png_iend),
		.typecode = *(uint32_t *)"IEND"
	} };
	long idat_start, idat_end, idat_size;
	uint32_t crc;
	size_t scanline;
	int (*debug)(const char *, ...) = &debug_off;

	if (1 < argc) {	/* turn on verbose, maybe */
		if (*(uint16_t *)argv[1] == *(uint16_t *)"-v") debug = &debug_on;
		if (debug != &debug_off) { argv[1] = argv[0]; argc--; argv++; }
	}

	/* make sure we've got our args */
	if (4 > argc) usage(argv[0]);

	/* open our files */
	if (NULL == (ifile = fopen(argv[1], "r"))) fail(argv[1]);
	if (NULL == (ofile = fopen(argv[4], "w"))) fail(argv[4]);

	/* write the PNG magic right away */
	fwrite(PNG_header, (size_t)8, (size_t)1, ofile);

	/* process width and height arguments */
	scanline = atol(argv[2]);
	ihdr.width = htonl(scanline);
	scanline *= 3;
	if (scanline > Z_CHUNK)
		die("Encoding images wider than %d pixels is not implemented", Z_CHUNK / 3);
	uint32_t height = atol(argv[3]);
	ihdr.height = htonl(height);
	debug("writing %s, %sx%s...", argv[4], argv[2], argv[3]);
	fflush(stdout);

	/* finish building the IHDR */
	ihdr.bit_depth = 8;	/* 24bpp */
	ihdr.color_type = 2;	/* color */
	ihdr.compression_method = 0;	/* zlib/32767 */
	ihdr.filter_method = 0;	/* adaptive */
	ihdr.interlace_method = 0;	/* progressive scan */

	/* write the IHDR */
	fwrite_chunk(&ihdr, ofile);

	/* save the IDAT location */
	idat_size = ftell(ofile);
	/* skip the size field until after compression */
	fseek(ofile, sizeof(uint32_t), SEEK_CUR);

	/* write the IDAT typecode */
	crc = crc32(0, "IDAT", 4);
	if (1 != fwrite("IDAT", 4, 1, ofile) || ferror(ofile)) fail(argv[4]);

	idat_start = ftell(ofile);
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = Z_NULL;

	if (Z_OK != deflateInit2(&stream, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY))
		die("failed to initialize zlib");

	crc = crc32(crc, zlib_header, 2);
	fwrite(zlib_header, 2, 1, ofile);

	/* actually compress */
	uint8_t input[Z_CHUNK] = { 0 };
	for (uint32_t i = 0; i < height; i++) {
		stream.next_in = input;
		stream.avail_in = (uInt)1 + fread(&input[1], 1, scanline, ifile);
		if (ferror(ifile)) fail(argv[1]);
		while (output(&stream, Z_NO_FLUSH, ofile, &crc));
	}
	/* finish up */
	stream.next_in = input;
	stream.avail_in = 1;
	output(&stream, Z_FINISH, ofile, &crc);
	fclose(ifile);
	deflateEnd(&stream);

	/* write the IDAT size */
	idat_end = ftell(ofile);
	idat_start = htonl(idat_end - idat_start);
	fseek(ofile, idat_size, SEEK_SET);
	fwrite(&idat_start, sizeof(uint32_t), 1, ofile);
	fseek(ofile, idat_end, SEEK_SET);

	crc = htonl(crc);
	fwrite(&crc, sizeof(crc), 1, ofile);

	fwrite_chunk(&iend, ofile);

	fclose(ofile);

	debug("done.\n");

	return 0;
}

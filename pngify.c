/* zlib-only PNG writer
 * by Pegasus Epsilon <pegasus@pimpninjas.org>
 * (C) 2012 Distribute Unmodified
 *
 */

#define _POSIX_C_SOURCE 200112L	/* fileno() */

#include <sys/types.h>	/* fstat(), struct stat */
#include <sys/stat.h> 	/* fstat(), struct stat */
#include <unistd.h>   	/* fstat(), struct stat */
#include <stddef.h>   	/* size_t */
#include <stdio.h>    	/* fopen(), fread(), fwrite(), fclose(), fileno() */
#include <stdlib.h>   	/* exit() */
#include <stdarg.h>   	/* va_list, va_start(), vprintf(), va_end() */
#include <stdint.h>   	/* uint8_t */
#include <string.h>   	/* memset() */
#include <zlib.h>     	/* z_stream, deflateInit2(), deflate(), crc32() */
#include <arpa/inet.h>	/* htonl() -- platform dependant byte-order reversal */

/* unalienate crc32 -- native types please! */
#define crc32(x, y, z) (uint32_t)crc32((uLong)x, (Bytef *)y, (uInt)z)

#define Z_CHUNK (size_t)(32 << 10) /* 32kB */

const uint8_t PNG_header[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const uint8_t IHDR_typecode[] = { 0x49, 0x48, 0x44, 0x52 };	/* "IHDR" */
const uint8_t IDAT_typecode[] = { 0x49, 0x44, 0x41, 0x54 };	/* "IDAT" */
const uint8_t zlib_header[] = { 0x78, 0xda };
const uint8_t IEND_typecode[] = { 0x49, 0x45, 0x4e, 0x44 };	/* "IEND" */

typedef struct {
	uint32_t size;
	uint32_t typecode;
} __attribute__((packed)) PNG_chunk;

typedef struct {
	PNG_chunk header;
	uint32_t width;
	uint32_t height;
	uint8_t bit_depth;
	uint8_t color_type;
	uint8_t compression_method;
	uint8_t filter_method;
	uint8_t interlace_method;
} __attribute__((packed)) IHDR;

typedef struct {
	PNG_chunk header;
} __attribute__((packed)) IEND;

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
void die (const char *fmt, ...) {       /* report errors */
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
	ctc = &((PNG_chunk *)chunk)->typecode;	/* CRC starts here */
	crc = htonl(crc32(0, ctc, (size - sizeof(uint32_t))));	/* do CRC */
	*csz = htonl(*csz - (uint32_t)sizeof(PNG_header));	/* fix length */
	fwrite(chunk, size, (size_t)1, ofile);	/* write chunk */
	fwrite(&crc, sizeof(uint32_t), (size_t)1, ofile);	/* write CRC */
}

int nothing (const char *f, ...) { return (long)f; }

int main (int argc, char **argv) {
	z_stream stream;
	FILE *ifile, *ofile;
	IHDR ihdr;
	IEND iend;
	long IDAT_start, IDAT_end, IDAT_size;
	uint32_t crc;
	size_t written = 0, scanline;
	int (*debug)(const char *, ...) = &nothing;

	if (1 < argc) {	/* turn on verbose, maybe */
		if (!strcmp("-v", argv[1])) debug = &printf;
		if (debug != &nothing) { argv[1] = argv[0]; argc--; argv++; }
	}

	/* make sure we've got our args */
	if (4 > argc) usage(argv[0]);

	/* open our files */
	if (NULL == (ifile = fopen(argv[1], "r"))) fail(argv[1]);
	if (NULL == (ofile = fopen(argv[4], "w"))) fail(argv[4]);

	/* write the PNG magic right away */
	fwrite(PNG_header, (size_t)8, (size_t)1, ofile);

	/* build the IHDR */
	ihdr.header.size = sizeof(ihdr);
	memcpy(&ihdr.header.typecode, IHDR_typecode, (size_t)4);

	/* process width and height arguments */
	ihdr.width = htonl((uint32_t)(scanline = (size_t)atol(argv[2])));
	scanline *= 3;
	if (scanline > Z_CHUNK)
		die("Encoding images wider than %d pixels is not implemented", Z_CHUNK / 3);
	ihdr.height = htonl((uint32_t)atol(argv[3]));
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
	IDAT_size = ftell(ofile);
	/* skip the size field until after compression */
	fseek(ofile, sizeof(uint32_t), SEEK_CUR);

	/* write the IDAT typecode */
	crc = crc32(0, IDAT_typecode, 4);
	if (
		1 != fwrite(IDAT_typecode, (size_t)4, (size_t)1, ofile)
		&& ferror(ofile)
	) fail(argv[4]);

	IDAT_start = ftell(ofile);
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = Z_NULL;
#pragma GCC diagnostic push
	/* Fuck zlib. Seriously. */
#pragma GCC diagnostic ignored "-Wtraditional-conversion"
	if (Z_OK != deflateInit2(&stream, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY))
		die("failed to initialize zlib");
#pragma GCC diagnostic pop

	crc = crc32(crc, zlib_header, 2);
	fwrite(zlib_header, (size_t)2, (size_t)1, ofile);

	do {
		uint8_t output[Z_CHUNK], input[Z_CHUNK];
		stream.next_in = input;
		stream.avail_in = (uInt)fread(input + 1, (size_t)1, scanline, ifile) + 1;
		*input = 0;	/* no filter */
		if (ferror(ifile)) fail(argv[1]);
		do {
			size_t have;
			stream.next_out = output;
			stream.avail_out = Z_CHUNK;
			if (Z_STREAM_ERROR == deflate(&stream, feof(ifile) ? Z_FINISH : Z_NO_FLUSH))
				die("zlib killed itself");
			if ((have = Z_CHUNK - stream.avail_out)) {
				/* FOUR TABS */
				crc = crc32(crc, output, have);
				if (1 != fwrite(output, have, (size_t)1, ofile)) fail(argv[4]);
				fflush(ofile);
			}
			written += have;
		} while (!stream.avail_out);
	} while (!feof(ifile));
	fclose(ifile);
	deflateEnd(&stream);

	/* write the IDAT size */
	IDAT_end = ftell(ofile);
	IDAT_start = IDAT_end - IDAT_start;
	fseek(ofile, IDAT_size, SEEK_SET);
	IDAT_start = (long)htonl((uint32_t)IDAT_start);
	fwrite(&IDAT_start, sizeof(uint32_t), (size_t)1, ofile);
	fseek(ofile, IDAT_end, SEEK_SET);

	crc = htonl(crc);
	fwrite(&crc, sizeof(crc), (size_t)1, ofile);

	iend.header.size = sizeof(iend);
	memcpy(&iend.header.typecode, IEND_typecode, (size_t)4);
	fwrite_chunk(&iend, ofile);

	fclose(ofile);

	debug("done.\n");

	return 0;
}

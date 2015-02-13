#define _GNU_SOURCE	/* asprintf(), execvpe() */
#include <sys/types.h>	/* stat(), mkfifo(), wait() */
#include <sys/stat.h>	/* stat(), mkfifo() */
#include <sys/wait.h>	/* wait() */
#include <unistd.h> 	/* stat(), fork(), execvpe() */
#include <signal.h> 	/* signal(), SIGCHLD, SIG_IGN */
#include <stdlib.h> 	/* malloc(), exit() */
#include <stdio.h>  	/* printf(), puts() */
#include <string.h> 	/* strlen() */

#ifdef _WIN32
#       define PRIuSIZET "Iu"
#       define PRIuOFFT "Id"
#else
#       define PRIuSIZET "zu"
#       define PRIuOFFT "zd"
#endif

#define NUMBERS_FMT "-%" PRIuSIZET "-%" PRIuSIZET
#define FIFO_EXT ".rgb"
#define OUTPUT_EXT ".png"

__attribute__((noreturn))
static inline void usage (char *myself) {
	printf("Usage: %s INFILE WIDTH HEIGHT VTILES HTILES OUTFILE OUTPROG ARGS\n\n", myself);
	puts("	INFILE	Input filename");
	puts("	WIDTH	Width of input file");
	puts("	HEIGHT	Height of input file");
	puts("	ROWS	Number of rows to tile");
	puts("	LINES	Number of lines to tile");
	puts("		Example, 10 rows and 10 lines will make 100 tiles,");
	puts("		10x10, out of the input image");
	puts("	OUTFILE	Output filename");
	puts("		Note: this will have -Y-X appended to it,");
	puts("		changing output.png to output-0-0.png, output-0-1.png, etc");
	puts("	OUTPROG	Full path to output compressor");
	puts("	ARGS	Any arguments that must be passed to the compressor");
	puts("		Note: Various tokens may be used here to be replaced when calling the");
	puts("		compressor. Available tokens are {infile} {outfile} {height} and {width}");
	puts("\nReport bugs to Pegasus Epsilon <pegasus@pimpninjas.org>");
	exit(1);
}

static inline char *split_filename_ext (char *in) {
	unsigned i = strlen(in);
	while (i--) if ('.' == in[i]) {
		in[i] = 0;
		return in + i + 1;
	}
	return NULL;
}

void reaper (int which) { (void)which; wait(NULL); }

/* spawn a compressor and attach to it through a named pipe,
 * the return the FILE* that corresponds to that connection
 */
FILE *spawn_compressor (
	char **argv, char **envp,
	char *outfile, size_t width, size_t height, size_t y, size_t x
) {
	static char *extension = "";

	const char *tokens[] = { "{infile}", "{outfile}", "{width}", "{height}", NULL };
	static union {
		char **byindex[4];
		struct {
			char **fifo;
			char **file;
			char **width;
			char **height;
		};
	} ptr = {{ 0 }};

	if (!*extension) {
		// find our tokens
		for (int i = 1; argv[i]; i++)
			for (int j = 0; tokens[j]; j++)
				if (!strcmp(tokens[j], argv[i])) {
					ptr.byindex[j] = &argv[i];
					break;
				}

		// figure out the outfile extension and strip it
		extension = split_filename_ext(outfile);
	}

	int i;
	if (ptr.fifo) i = asprintf(ptr.fifo, "%s" NUMBERS_FMT FIFO_EXT, outfile, y, x);
	if (ptr.file) i = asprintf(ptr.file, "%s" NUMBERS_FMT ".%s", outfile, y, x, extension);
	if (ptr.width) i = asprintf(ptr.width, "%" PRIuSIZET, width);
	if (ptr.height) i = asprintf(ptr.height, "%" PRIuSIZET, height);
	(void)i;

	if (mkfifo(*ptr.fifo, 0600)) perror(*ptr.fifo);

	if (!fork()) {
		for (int i = 0; tokens[i]; i++) if (ptr.byindex[i])
			*ptr.byindex[i] = strdup(*ptr.byindex[i]);
		execvpe(*argv, argv, envp);
	}

	FILE *ret = fopen(*ptr.fifo, "w");
	for (int i = 0; tokens[i]; i++) free(*ptr.byindex[i]);
	return ret;
}

int main (int argc, char **argv, char **envp) {
	if (8 > argc) usage(argv[0]);

	char *input_filename = argv[1];
	size_t total_width = atoi(argv[2]);
	size_t total_height = atoi(argv[3]);
	size_t vertical_tiles = atoi(argv[4]);
	size_t horizontal_tiles = atoi(argv[5]);
	char *output_filename = argv[6];
	FILE *input_file = fopen(input_filename, "r");
	FILE **fifos = malloc(horizontal_tiles * sizeof(FILE *));

	signal(SIGCHLD, &reaper);
	for (size_t ty = 0; ty < vertical_tiles; ty++) {
		size_t height = (size_t)((1 + ty) * total_height / vertical_tiles)
		              - (size_t)(     ty  * total_height / vertical_tiles);
		size_t *widths = malloc(horizontal_tiles * sizeof(size_t *));

		for (size_t tx = 0; tx < horizontal_tiles; tx++) {
			widths[tx] = (size_t)((1 + tx) * total_width / horizontal_tiles)
			           - (size_t)(     tx  * total_width / horizontal_tiles);
			fifos[tx] = spawn_compressor(&argv[7], envp, output_filename, widths[tx], height, ty, tx);
		}

		for (size_t y = 0; y < height; y++)
			for (size_t tx = 0; tx < horizontal_tiles; tx++) {
				char *buffer = calloc(widths[tx], 3);
				if (widths[tx] != fread(buffer, 3, widths[tx], input_file))
					puts("short read");
				fwrite(buffer, 3, widths[tx], fifos[tx]);
				free(buffer);
			}

		for (size_t tx = 0; tx < horizontal_tiles; tx++) {
			fclose(fifos[tx]);
			char *fifo;
			int i = asprintf(&fifo, "%s" NUMBERS_FMT FIFO_EXT, output_filename, ty, tx);
			(void)i; unlink(fifo); free(fifo);
		}

		free(widths);
	}

	free(fifos);
	fclose(input_file);

	return 0;
}

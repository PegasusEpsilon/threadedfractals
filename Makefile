CC=cc
CFLAGS=-Ofast -Wall -Wextra -Werror -ansi -pedantic -std=c99 -fmax-errors=3

default:	pngify resample render palette threaded threadless modules

pngify:	pngify.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lz

resample:	resample.c utils.o
	$(CC) $(CFLAGS) $^ -o $@

render:	render.c
	$(CC) $(CFLAGS) $^ -o $@ -lm

palette:	palette.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

threaded:	threaded.c loader.h loader.o mapper.h mapper.o utils.h utils.o types.h modules/sampler.h
	$(CC) $(CFLAGS) $^ -o $@ -lm -lpthread -ldl

threadless:	threadless.c loader.h loader.o mapper.h mapper.o utils.h utils.o types.h modules/sampler.h
	$(CC) $(CFLAGS) $^ -o $@ -lm -ldl

.PHONY:	modules

modules:
	$(MAKE) -C modules

clean:
	for spec in \
		julia.png threaded.png threadless.png palette.png \
		julia.rgb threaded.rgb threadless.rgb \
		julia.msaa threaded.msaa threadless.msaa palette.bin \
		julia.map threaded.map threadless.map \
		*.o threaded threadless palette render resample tiler pngify \
	; do test -e $$spec && rm $$spec \
	; done || true
	$(MAKE) clean -C modules

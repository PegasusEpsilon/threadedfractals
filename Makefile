CC=cc
CFLAGS=-Ofast -Wall -Wextra -Wshadow -Werror -ansi -pedantic -std=c99
HEADERS=circularlist.h loader.h mapper.h utils.h types.h modules/sampler.h
OBJECTS=circularlist.o loader.o mapper.o utils.o
DEPS=$(HEADERS) $(OBJECTS)

ifneq ($(shell uname),Darwin)
	# clang no likey this flag
	CFLAGS=$(CFLAGS) -fmax-errors=3
endif

default:	pngify resample render palette threaded threadless modules

pngify:	pngify.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lz

resample:	resample.c utils.o
	$(CC) $(CFLAGS) $^ -o $@

render:	render.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

palette:	palette.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

threaded:	threaded.c $(DEPS)
	$(CC) $(CFLAGS) $< $(OBJECTS) -o $@ -lm -lpthread -ldl

threadless:	threadless.c $(DEPS)
	$(CC) $(CFLAGS) $< $(OBJECTS) -o $@ -lm -ldl

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

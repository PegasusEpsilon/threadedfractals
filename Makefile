CFLAGS = -Ofast -Wall -Wextra -Wshadow -Wconversion -Werror -ansi -pedantic -std=c99
HEADERS = config.h circularlist.h loader.h mapper.h utils.h types.h modules/sampler.h
OBJECTS = circularlist.o loader.o mapper.o utils.o
DEPS = $(HEADERS) $(OBJECTS)

ifeq ($(shell $(CC) --version | awk 'NR == 1 { print $$1 }'),clang)
	CFLAGS += -fsanitize=shift
else # assume GCC
	CFLAGS += -fmax-errors=3
endif

default:	pngify resample render palette threaded threadless modules

resample:	resample.c utils.o
	$(CC) $(CFLAGS) $^ -o $@

render:	render.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -lquadmath

palette:	palette.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

threaded:	threaded.c $(DEPS)
	$(CC) $(CFLAGS) $< $(OBJECTS) -o $@ -lm -lpthread -ldl

threadless:	threadless.c $(DEPS)
	$(CC) $(CFLAGS) $< $(OBJECTS) -o $@ -lm -ldl

.PHONY:	modules pngify

modules:
	$(MAKE) -C modules

pngify:
	$(MAKE) -C pngify

clean:
	for spec in \
		julia.png threaded.png threadless.png palette.png \
		julia.rgb threaded.rgb threadless.rgb \
		julia.msaa threaded.msaa threadless.msaa palette.bin \
		julia.map threaded.map threadless.map \
		*.o threaded threadless palette render resample tiler \
	; do test -e $$spec && rm $$spec \
	; done || :
	$(MAKE) clean -C modules
	$(MAKE) clean -C pngify

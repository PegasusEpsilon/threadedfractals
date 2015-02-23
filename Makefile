CC=cc
CFLAGS=-Ofast -Wall -Wextra -Werror -ansi -pedantic -std=c99 -fmax-errors=3

pngify:	pngify.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lz

resample:	resample.c utils.o
	$(CC) $(CFLAGS) $^ -o $@

render:	render.c
	$(CC) $(CFLAGS) $^ -o $@ -lm

palette:	palette.c utils.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

threaded:	threaded.c loader.h loader.o mapper.h mapper.o utils.h utils.o types.h sample.h
	$(CC) $(CFLAGS) $^ -o $@ -lm -lpthread -ldl

threadless:	threadless.c loader.h loader.o mapper.h mapper.o utils.h utils.o types.h sample.h
	$(CC) $(CFLAGS) $^ -o $@ -lm -ldl

%.so: %.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

rotate.so:	rotate.c loader.c utils.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

translate.so:	translate.c loader.c utils.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

scale.so:	scale.c loader.c utils.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

renormalized.so:	renormalized.c loader.c utils.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

mandelbrot.so:	mandelbrot.c loader.c utils.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

julia.so:	julia.c loader.c utils.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

modules:
	make julia.so mandelbrot.so crosstrap.so pointtrap.so renormalized.so \
		escape_count.so rotate.so translate.so scale.so

clean:
	rm *.o \
		threaded.png threadless.png palette.png \
		threaded.msaa threadless.msaa palette.bin \
		threaded.rgb threadless.rgb \
		threaded.map threadless.map \
		threaded threadless \
		renormalized.so pointtrap.so crosstrap.so \
		mandelbrot.so julia.so escape_count.so \
		rotate.so translate.so scale.so \
		palette render resample tiler pngify || \
	true

CC=cc
CFLAGS=-g -Wall -Wextra -Werror -ansi -pedantic -std=c99 -fmax-errors=3
LIBS=-lm
DIVIDER=1
THREADS=2
MSAA=1
SIZE_REAL=1920
SIZE_IMAG=1080
MSAA_REAL=$$(($(SIZE_REAL)*$(MSAA)))
MSAA_IMAG=$$(($(SIZE_IMAG)*$(MSAA)))
CENTER_REAL=-0.7766729
CENTER_IMAG=-0.13661091
RADIUS_REAL=0.00016
RADIUS_IMAG=$(shell echo "scale=40;$(RADIUS_REAL)*$(SIZE_IMAG)/$(SIZE_REAL)"|bc|sed -e 's/0*$$//')
THETA=0
ARGS=$(MSAA_REAL) $(MSAA_IMAG) $(CENTER_REAL) $(CENTER_IMAG) $(RADIUS_REAL) $(RADIUS_IMAG) $(THETA)

threaded:	threaded.c sample.h sample.o mapper.h mapper.o types.h
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) -lpthread

threadless:	threadless.c sample.h sample.o mapper.h mapper.o types.h
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

render:	render.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

palette:	palette.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

palette.bin:	palette palette.txt
	./palette palette.txt palette.bin

pngify:	pngify.c
	$(CC) $(CFLAGS) $^ -o $@ -lz

threaded.map:	threaded
	bash -c 'time ./$^ $(THREADS) $(ARGS) $@'

threaded.msaa:	render threaded.map palette.bin
	./render -l threaded.map palette.bin 0 $(DIVIDER) $@

threaded.rgb:	resample threaded.msaa
	./$^ $(SIZE_REAL) $(MSAA) $@

threaded.png:	pngify threaded.rgb
	./$^ $(SIZE_REAL) $(SIZE_IMAG) $@

threadless.map:	threadless
	bash -c 'time ./$^ $(ARGS) $@'

threadless.msaa:	render threadless.map palette.bin
	./render -l threadless.map palette.bin 0 $(DIVIDER) $@

threadless.rgb:	resample threadless.msaa
	./$^ $(SIZE_REAL) $(MSAA) $@

threadless.png:	pngify threadless.rgb
	./$^ $(SIZE_REAL) $(SIZE_IMAG) $@

map:
	rm sample.map || true

clean: map
	rm *.o render resample pngify \
		threaded threadless palette \
		threaded.png threadless.png palette.png \
		threaded.rgb threadless.rgb palette.bin \
		threaded.msaa threadless.msaa \
		threaded.map threadless.map || \
	true

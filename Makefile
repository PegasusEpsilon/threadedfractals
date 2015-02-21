CC=cc
CFLAGS=-Ofast -Wall -Wextra -Werror -ansi -pedantic -std=c99 -fmax-errors=3
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

threaded.png:	pngify threaded.rgb
	./$^ $(SIZE_REAL) $(SIZE_IMAG) $@

threaded.rgb:	resample threaded.msaa
ifneq ($(MSAA), 1)
	./$^ $(SIZE_REAL) $(MSAA) $@
else
	ln -sf threaded.msaa threaded.rgb
endif

threaded.msaa:	render threaded.map palette.bin
	./render -l threaded.map palette.bin 0 $(DIVIDER) $@

threaded.map:	threaded renormalized.so
	bash -c 'time ./$^ $(THREADS) $(ARGS) $@'

threadless.png:	pngify threadless.rgb
	./$^ $(SIZE_REAL) $(SIZE_IMAG) $@

threadless.rgb:	resample threadless.msaa
ifneq ($(MSAA), 1)
	./$^ $(SIZE_REAL) $(MSAA) $@
else
	ln -sf threadless.msaa threadless.rgb
endif

threadless.msaa:	render threadless.map palette.bin
	./render -l threadless.map palette.bin 0 $(DIVIDER) $@

threadless.map:	threadless renormalized.so
	bash -c 'time ./$^ $(ARGS) $@'

palette.bin:	palette palette.txt
	./palette palette.txt palette.bin

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

map:
	rm sample.map || true

renormalized.so:	renormalized.c
	$(CC) -fPIC -shared $^ -o $@

clean: map
	rm *.o \
		threaded.png threadless.png palette.png \
		threaded.msaa threadless.msaa palette.bin \
		threaded.rgb threadless.rgb \
		threaded.map threadless.map \
		threaded threadless \
		palette render resample tiler pngify || \
	true

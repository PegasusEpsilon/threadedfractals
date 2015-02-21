CC=cc
CFLAGS=-Ofast -Wall -Wextra -Werror -ansi -pedantic -std=c99 -fmax-errors=3
THREADS=2
MSAA=1
SIZE_REAL=1920
SIZE_IMAG=1080
MSAA_REAL=$$(($(SIZE_REAL)*$(MSAA)))
MSAA_IMAG=$$(($(SIZE_IMAG)*$(MSAA)))

# nautilus
CENTER_REAL=-0.7766729
CENTER_IMAG=-0.13661091
RADIUS_REAL=0.00016

# full set
CENTER_REAL=-0.75
CENTER_IMAG=0
RADIUS_REAL=2

# bird of paradise
#CENTER_REAL=0.3750001200618655
#CENTER_IMAG=0.2166393884377127i
#RADIUS_REAL=0.000000000002

RADIUS_IMAG=$(shell echo "scale=40;$(RADIUS_REAL)*$(SIZE_IMAG)/$(SIZE_REAL)"|bc|sed -e 's/0*$$//')
THETA=0
SHIFT=0

# renormalized sampler
#SAMPLER=renormalized.so
#SAMPLER_ARGS=
#DIVIDER=1
#FLATTEN=-l

# cross trap sampler
SAMPLER=crosstrap.so
# crosstrap.so range start angle
SAMPLER_ARGS=0.5 1 0
DIVIDER=1.5
FLATTEN=

# point trap sampler
#SAMPLER=pointtrap.so
# pointtrap.so range start
#SAMPLER_ARGS=0.005 1
#DIVIDER=10
#FLATTEN=

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
	./render $(FLATTEN) threaded.map palette.bin $(SHIFT) $(DIVIDER) $@

threaded.map:	threaded $(SAMPLER)
	bash -c 'time ./$< $(THREADS) $(ARGS) $@ $(SAMPLER) $(SAMPLER_ARGS)'

threadless.png:	pngify threadless.rgb
	./$^ $(SIZE_REAL) $(SIZE_IMAG) $@

threadless.rgb:	resample threadless.msaa
ifneq ($(MSAA), 1)
	./$^ $(SIZE_REAL) $(MSAA) $@
else
	ln -sf threadless.msaa threadless.rgb
endif

threadless.msaa:	render threadless.map palette.bin
	./render $(FLATTEN) threadless.map palette.bin $(SHIFT) $(DIVIDER) $@

threadless.map:	threadless $(SAMPLER)
	bash -c 'time ./$< $(ARGS) $@ $(SAMPLER) $(SAMPLER_ARGS)'

palette.png:	pngify palette.bin
	./$^ 45 34 $@

palette.bin:	palette blueglow.txt
	./$^ palette.bin

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

%.so: %.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

clean: map
	rm *.o \
		threaded.png threadless.png palette.png \
		threaded.msaa threadless.msaa palette.bin \
		threaded.rgb threadless.rgb \
		threaded.map threadless.map \
		threaded threadless \
		renormalized.so pointtrap.so crosstrap.so \
		palette render resample tiler pngify || \
	true

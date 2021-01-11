CFLAGS=-Ofast -I.. -Wall -Wextra -Wshadow -Wconversion -Winline -Werror -ansi -pedantic -std=c99
DEPS=../utils.c

ifeq ($(shell $(CC) --version | awk 'NR == 1 { print $$1 }'),clang)
	CFLAGS += -fsanitize=shift
else # assume GCC
	CFLAGS += -fmax-errors=3
endif

modules:
	make julia.so mandelbrot.so crosstrap.so pointtrap.so renormalized.so \
	escape_count.so rotate.so translate.so scale.so dragon.so levy.so \
	mirror.so flip.so

# nothing special
%.so: %.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

# no libs, just deps
translate.so:	translate.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

mirror.so:	mirror.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

flip.so:	flip.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

scale.so:	scale.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

julia.so:	julia.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

# deps and libs
rotate.so:	rotate.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

renormalized.so:	renormalized.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm -lquadmath

mandelbrot.so:	mandelbrot.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

escape_count.so:	escape_count.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lquadmath

crosstrap.so:	crosstrap.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm -lquadmath

pointtrap.so:	pointtrap.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm -lquadmath

# no deps, just libs
dragon.so:	dragon.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

levy.so:	levy.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

clean:
	for spec in \
		renormalized.so pointtrap.so crosstrap.so flip.so \
		mandelbrot.so julia.so escape_count.so mirror.so \
		rotate.so translate.so scale.so dragon.so levy.so\
	; do test -e $$spec && rm $$spec \
	; done || :

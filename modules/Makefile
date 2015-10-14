CFLAGS=-Ofast -I.. -Wall -Wextra -Wshadow -Werror -ansi -pedantic -std=c99
DEPS=../loader.c ../utils.c

ifneq ($(shell uname),Darwin)
	# clang no likey this flag
	CFLAGS=$(CFLAGS) -fmax-errors=3
endif

modules:
	make julia.so mandelbrot.so crosstrap.so pointtrap.so renormalized.so \
	escape_count.so rotate.so translate.so scale.so dragon.so levy.so

# nothing special
%.so: %.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

# only deps
translate.so:	translate.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

scale.so:	scale.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

julia.so:	julia.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

# deps and math library
rotate.so:	rotate.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

renormalized.so:	renormalized.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

mandelbrot.so:	mandelbrot.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

# no deps, just math
crosstrap.so:	crosstrap.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

dragon.so:	dragon.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

levy.so:	levy.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

clean:
	for spec in \
		renormalized.so pointtrap.so crosstrap.so \
		mandelbrot.so julia.so escape_count.so \
		rotate.so translate.so scale.so dragon.so levy.so\
	; do test -e $$spec && rm $$spec \
	; done || true

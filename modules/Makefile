FLAGS=-Ofast -I.. -Wall -Wextra -Wshadow -Werror -ansi -pedantic -std=c99
DEPS=../utils.c

ifeq ($(shell uname),Darwin)
	CFLAGS=$(FLAGS)
else
	# clang no likey this flag
	CFLAGS=$(FLAGS) -fmax-errors=3
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
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

mandelbrot.so:	mandelbrot.c ../loader.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

# no deps, just libs
crosstrap.so:	crosstrap.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -lm

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

CC=cc
CFLAGS=-g -Ofast -I.. -Wall -Wextra -Werror -ansi -pedantic -std=c99 -fmax-errors=3
DEPS=../loader.c ../utils.c

modules:
	make julia.so mandelbrot.so crosstrap.so pointtrap.so renormalized.so \
	escape_count.so rotate.so translate.so scale.so dragon.so

%.so: %.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

rotate.so:	rotate.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

translate.so:	translate.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

scale.so:	scale.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

renormalized.so:	renormalized.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

mandelbrot.so:	mandelbrot.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

julia.so:	julia.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

dragon.so:	dragon.c
	$(CC) $(CFLAGS) -fPIC -shared $^ -lm -o $@

clean:
	for spec in \
		renormalized.so pointtrap.so crosstrap.so \
		mandelbrot.so julia.so escape_count.so \
		rotate.so translate.so scale.so dragon.so \
	; do test -e $$spec && rm $$spec \
	; done || true

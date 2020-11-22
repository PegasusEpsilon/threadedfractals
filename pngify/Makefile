CC=cc
FLAGS=-Ofast -Wall -Wextra -Wshadow -Werror -ansi -pedantic -std=c99 -g

ifeq ($(shell uname),Darwin)
	CFLAGS=$(FLAGS)
else
	# clang no likey this flag
	CFLAGS=$(FLAGS) -fmax-errors=3
endif

default:	pngify

smush:	smush.c ../utils.o
	$(CC) -I.. $(CFLAGS) $^ -o $@ -lz

pngify:	pngify.c ../utils.o
	$(CC) -I.. $(CFLAGS) $^ -o $@ -lz

pngify32:	pngify32.c ../utils.o
	$(CC) -I.. $(CFLAGS) $^ -o $@ -lz

clean:
	rm pngify || :

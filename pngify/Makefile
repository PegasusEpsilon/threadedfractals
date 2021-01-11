CFLAGS = -Ofast -Wall -Wextra -Wshadow -Wconversion -Winline -Werror -ansi -pedantic -std=c99 -g

ifeq ($(shell $(CC) --version | awk 'NR == 1 { print $$1 }'),clang)
	CFLAGS += -fsanitize=shift
else # assume GCC
	CFLAGS += -fmax-errors=3
endif

default:	pngify

pngify:	pngify.c ../utils.o
	$(CC) -I.. $(CFLAGS) $^ -o $@ -lz

clean:
	rm pngify || :

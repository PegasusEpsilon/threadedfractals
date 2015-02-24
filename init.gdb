set args 1 192 108 test.map translate -0.75 0 mandelbrot renormalized escape_count 16
break main
run
skip file atoi.c
skip file malloc.c
skip file printf.c
skip file random.c
skip file rand.c
skip file ioputs.c
skip file iofflush.c
skip file strtod.c
skip file iofopen.c
skip file pthread_attr_init.c
skip file pthread_attr_setdetachstate.c
skip file pthread_create.c
skip file ../nptl/pthread_mutex_lock.c
skip file pthread_mutex_unlock.c
skip file cabsl.c
skip file w_logl.c
skip file w_log2l.c
skip file iofwrite.c

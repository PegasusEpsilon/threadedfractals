#ifndef UTILS_H
#define UTILS_H
#include <stdlib.h>   	/* exit() */
#include <stdarg.h>   	/* va_list, va_start(), vprintf(), va_end() */
#include <stdio.h>    	/* perror() puts() */

/* Print errno error message and exit with errorlevel */
__attribute__((cold noreturn)) /* can it be pure when it calls perror? */
void fail (const char *restrict const msg);

/* Print custom message and exit with errorlevel */
__attribute__((pure cold noreturn))
void die (const char *restrict const fmt, ...);

/* debug output system */
extern int (*debug)(const char *, ...);
void enable_debug (void);
#endif /* UTILS_H */

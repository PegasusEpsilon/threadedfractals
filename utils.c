/* utils.c, from threadedfractals
 * by "Pegasus Epsilon" <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 */

#include <stdlib.h> 	/* exit() */
#include <stdarg.h> 	/* va_list, va_start(), vprintf(), va_end() */
#include <stdio.h>  	/* perror() puts() */
#include <stdbool.h>	/* bool */
#include <errno.h>  	/* errno */
#include <limits.h> 	/* UINT_MAX */

unsigned strtoui (const char *ptr, char **endptr, int base) {
	long unsigned val = strtoul(ptr, endptr, base);
	errno = errno ? errno : val > UINT_MAX ? ERANGE : errno;
	return (unsigned)val;
}

#define SAFE_BASE(x, y) x safe_ ## y (\
	const char *const ptr, char **endptr, int base, \
	const char *const name, void (*errcb)(void)\
) {\
	x ret;\
	errno = 0;\
	ret = y(ptr, endptr, base);\
	if (errno) {\
		printf("invalid %s (too large or negative)\n\n", name);\
		errcb();\
	}\
	return ret;\
}

#define SAFE_NOBASE(x, y) x safe_ ## y (\
	const char *const ptr, char **endptr, \
	const char *const name, void (*errcb)(void)\
) {\
	x ret;\
	errno = 0;\
	ret = y(ptr, endptr);\
	if (errno) {\
		printf("invalid %s (out of range)\n\n", name);\
		errcb();\
	}\
	return ret;\
}

SAFE_BASE(unsigned, strtoui)
SAFE_BASE(long long unsigned, strtoull)
SAFE_NOBASE(long double, strtold)

/* Print errno error message and exit with errorlevel */
__attribute__((cold, noreturn))
void fail (const char *restrict const msg) {
	perror(msg);
	exit(1);
}

/* Print custom message and exit with errorlevel */
__attribute__((cold, noreturn))
void die (const char *restrict const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	puts(".");
	exit(1);
}

/* debug print system */
int debug_off (const char *restrict const fmt, ...) { return *(int *)fmt; }
int debug_on  (const char *restrict const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vprintf(fmt, args);
	va_end(args);
	fflush(stdout);
	return ret;
}
int (*debug)(const char *, ...) = &debug_off;
void enable_debug (void) { debug = &debug_on; }
bool debug_enabled (void) { return debug == &debug_on; }

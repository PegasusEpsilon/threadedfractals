#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

/* safe_* functions operate as their non-safe counterparts, except instead of
 * setting errno, print an error and call their callback function.
 */
unsigned safe_strtoui (
	char *ptr, char **endptr, int base, char *name, void (*error_cb)(void)
);
long long unsigned safe_strtoull (char *, char **, int, char *, void (*)(void));
long double safe_strtold (char *ptr, char **endptr, char *name, void(*err_cb)(void));

/* Print errno error message and exit with errorlevel */
__attribute__((cold, noreturn))
void fail (const char *restrict const msg);

/* Print custom message and exit with errorlevel */
__attribute__((cold, noreturn))
void die (const char *restrict const fmt, ...);

/* debug output system */
extern int (*debug)(const char *, ...);
void enable_debug (void);
bool debug_enabled (void);

#endif /* UTILS_H */

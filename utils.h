#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

/* Print errno error message and exit with errorlevel */
__attribute__((cold))
__attribute__((noreturn))
void fail (const char *restrict const msg);

/* Print custom message and exit with errorlevel */
__attribute__((cold))
__attribute__((noreturn))
void die (const char *restrict const fmt, ...);

/* debug output system */
extern int (*debug)(const char *, ...);
void enable_debug (void);
bool debug_enabled (void);

#endif /* UTILS_H */

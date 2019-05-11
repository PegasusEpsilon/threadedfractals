#ifndef LOADER_H
#define LOADER_H

#include <complex.h>
#include "config.h"

void *sampler_handle;

__attribute__((cold))
#ifdef COMPLEX
FLOAT (*get_sampler (char **))(FLOAT complex *, FLOAT complex *);
#else
FLOAT (*get_sampler (char **))(FLOAT complex *);
#endif

#endif /* LOADER_H */

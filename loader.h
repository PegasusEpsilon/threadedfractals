#ifndef LOADER_H
#define LOADER_H

#include <complex.h>
#include "config.h"

void *sampler_handle;

__attribute__((cold))
#ifdef COMPLEX_SAMPLER
FLOAT (*get_sampler (char **))(COMPLEX *, COMPLEX *);
#else
FLOAT (*get_sampler (char **))(COMPLEX *);
#endif

#endif /* LOADER_H */

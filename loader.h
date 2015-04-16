#ifndef LOADER_H
#define LOADER_H

#include <complex.h>
#include "config.h"

void *sampler_handle;

__attribute__((cold))
FLOAT (*get_sampler (char **))(FLOAT complex *);

#endif /* LOADER_H */

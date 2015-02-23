#ifndef LOADER_H
#define LOADER_H

#include <complex.h>

void *sampler_handle;

__attribute__((cold))
long double (*get_sampler (char **))(long double complex *);

#endif /* LOADER_H */

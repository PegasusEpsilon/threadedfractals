#ifndef LOADER_H
#define LOADER_H

#include "types.h"

void *sampler_handle;

__attribute__((cold))
long double (*get_sampler (char *lib_name, char **))(struct coordinates_4d *);

#endif /* LOADER_H */

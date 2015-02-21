#ifndef LOADER_H
#define LOADER_H

#include "types.h"

void *sampler_handle;

__attribute__((cold)) long double (*get_sampler (char *lib_name))(struct coordinates_4d *);
#endif /* LOADER_H */

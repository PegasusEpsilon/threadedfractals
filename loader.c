#define _GNU_SOURCE 	/* asprintf() */
#include <stdio.h>  	/* asprintf() */
#include <stdlib.h> 	/* atexit() */
#include <dlfcn.h>  	/* dlopen(), dlsym(), dlclose() */
#include <complex.h>	/* complex */

#include "utils.h"
#include "config.h"

// each module has its own copy of this compilation unit, so this variable is
// not shared between any of them, despite it only being created once in the
// codebase
static void *sampler_handle = NULL;
__attribute__((cold)) static
void dispose_sampler (void) { dlclose(sampler_handle); }

__attribute__((cold))
FLOAT (*get_sampler (char **argv))(complex FLOAT *) {
	FLOAT (*fn)(complex FLOAT *);
	void (*init)(char **);

	if (sampler_handle)
		die("Calling get_sampler twice will result in leaks. Exiting.");

	char *tmp;
	if (-1 == asprintf(&tmp, "modules/%s.so", argv[0]))
		die("asprintf() threw an error. Giving up.");
	if (!(sampler_handle = dlopen(argv[0], RTLD_LAZY))) {
		free(tmp);
		if (-1 == asprintf(&tmp, "./modules/%s.so", argv[0]))
			die("asprintf() threw an error. Giving up.");
		/* valgrind/memcheck reports a leak of memory allocated by dlopen().
		 * there is no way to force libdl to release this memory.
		 * it's not a bug in this code.
		 */
		sampler_handle = dlopen(tmp, RTLD_LAZY);
		if (!sampler_handle) die(dlerror());
	}
	free(tmp);

	/* clean up when we exit */
	atexit(dispose_sampler);

/* We have officially left the realm of portability.
 * To port this to Windows will now require DLL shenanigans and some #ifdefs.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	if (!(fn = (FLOAT (*)(complex FLOAT *))dlsym(sampler_handle, "sample"))) {
		printf("loader error: ");
		tmp = dlerror();
		die(tmp ? tmp : "NULL sampler not allowed");
	}

	if ((init = (void (*)(char **))dlsym(sampler_handle, "init")))
		init(argv);
#pragma GCC diagnostic pop

	return fn;
}

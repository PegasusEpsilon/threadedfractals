#define _GNU_SOURCE 	/* asprintf() */

#include <stdlib.h> 	/* atexit() */
#include <dlfcn.h>  	/* dlopen(), dlsym(), dlclose() */

#include "types.h"
#include "utils.h"

void *sampler_handle = NULL;
__attribute__((cold)) static
void dispose_sampler (void) { dlclose(sampler_handle); }

__attribute__((cold))
long double (*get_sampler (char **argv))(struct coordinates_4d *) {
	long double (*fn)(struct coordinates_4d *);
	void (*init)(char **);

	if (sampler_handle)
		die("Calling get_sampler twice will result in leaks. Exiting.");

	if (!(sampler_handle = dlopen(argv[0], RTLD_LAZY))) {
		char *tmp;
		if (-1 == asprintf(&tmp, "./%s", argv[0]))
			die("asprintf() threw an error. Giving up.");
		sampler_handle = dlopen(tmp, RTLD_LAZY);
		free(tmp);
		if (!sampler_handle) die(dlerror());
	}

	/* clean up when we exit */
	atexit(dispose_sampler);

/* We have officially left the realm of portability.
 * To port this to Windows will now require DLL shenanigans and some #ifdefs.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
	if (!(fn = (long double (*)(struct coordinates_4d *))dlsym(sampler_handle, "sample"))) {
		char *tmp = dlerror();
		die(tmp ? tmp : "NULL sampler not allowed");
	}

	if ((init = (void (*)(char **))dlsym(sampler_handle, "init")))
		init(argv);
#pragma GCC diagnostic pop

	return fn;
}

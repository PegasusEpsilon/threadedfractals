#ifndef SAMPLER_H
#define SAMPLER_H

#ifdef COMPLEX_SAMPLER
	#define sampler(x) FLOAT (*x) (COMPLEX *const, COMPLEX *const)
#else
	#define sampler(x) FLOAT (*x) (COMPLEX *const)
#endif

#endif /* SAMPLER_H */

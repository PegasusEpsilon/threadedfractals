#ifndef SAMPLER_H
#define SAMPLER_H

#ifdef COMPLEX
	#define sampler(x) FLOAT (*x) (complex FLOAT *const, complex FLOAT *const)
#else
	#define sampler(x) FLOAT (*x) (FLOAT complex *const)
#endif

#endif /* SAMPLER_H */

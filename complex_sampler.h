#ifndef COMPLEX_SAMPLER_H
#define COMPLEX_SAMPLER_H

#define sampler(x) long double (*x) (long double complex *const, long double complex *const)

static sampler(complex_sample);
#endif /* COMPLEX_SAMPLER_H */

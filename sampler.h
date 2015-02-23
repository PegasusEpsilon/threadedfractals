#ifndef SAMPLER_H
#define SAMPLER_H

#define sampler(x) long double (*x) (long double complex *const)

static sampler(real_sample);
#endif /* SAMPLER_H */

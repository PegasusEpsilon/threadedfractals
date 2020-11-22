#ifndef CONFIG_H
#define CONFIG_H

/* the width of the float types may or may not matter on your machine.
 *
 * on my laptop (core 2 duo) double is fastest. long double obviously
 * gives more accurate results (needed when rendering a deep zoom).
 *
 * on my desktop (ryzen 3 2200G) longdouble is just as fast as double.
 *
 * software floating point is always slower, but if you need software long
 * doubles (or better) anyway, you might get the same performance out of
 * software quad doubles. if you do, you might as well use quad doubles.
 */

// If you actually need singles for performance, you are on a very old system.
//#define SINGLE
// If you have an FPU at all, doubles are probably the same speed as singles.
//#define DOUBLE
// Long doubles may or may not be software, and thus may or may not be slow.
#define LONGDOUBLE
// I have never seen a machine that does quad doubles in hardware.
//#define SOFTQUAD

#ifdef SINGLE
	#define FLOAT float
	#define LOG logf
	#define LOG2 log2f
	#define SIN(x) sinf(x)
	#define COS(x) cosf(x)
	#define FABS(x) fabsf(x)
	#define SQRT(x) sqrtf(x)
	#define CABS(z) cabsf(z)
	#define CREAL(z) crealf(z)
	#define CIMAG(z) cimagf(z)
	#define FMT "f"
#endif // SINGLE

#ifdef DOUBLE
	#define FLOAT double
	#define LOG log
	#define LOG2 log2
	#define SIN(x) sin(x)
	#define COS(x) cos(x)
	#define FABS(x) fabs(x)
	#define SQRT(x) sqrt(x)
	#define CABS(z) cabs(z)
	#define CREAL(z) creal(z)
	#define CIMAG(z) cimag(z)
	#define FMT "f"
#endif // DOUBLE

#ifdef LONGDOUBLE
	#define FLOAT long double
	#define LOG logl
	#define LOG2 log2l
	#define SIN(x) sinl(x)
	#define COS(x) cosl(x)
	#define FABS(x) fabsl(x)
	#define SQRT(x) sqrtl(x)
	#define CABS(z) cabsl(z)
	#define CREAL(z) creall(z)
	#define CIMAG(z) cimagl(z)
	#define FMT "Lf"
#endif // LONGDOUBLE

#ifdef SOFTQUAD
	#include <quadmath.h>
	#define FLOAT __float128
	#define COMPLEX __complex128
	#define LOG logq
	#define LOG2 log2q
	#define SIN(x) sinq(x)
	#define COS(x) cosq(x)
	#define FABS(x) fabsq(x)
	#define SQRT(x) sqrtq(x)
	#define CABS(z) cabsq(z)
	#define CREAL(z) crealq(z)
	#define CIMAG(z) cimagq(z)
	#define FMT "Qf"
#else
	#define COMPLEX complex FLOAT
#endif // SOFTQUAD

#if defined(SINGLE) && defined(DOUBLE) || \
	defined(SINGLE) && defined(LONGDOUBLE) || \
	defined(SINGLE) && defined(SOFTQUAD) || \
	defined(DOUBLE) && defined(LONGDOUBLE) || \
	defined(DOUBLE) && defined(SOFTQUAD) || \
	defined(LONGDOUBLE) && defined(SOFTQUAD)
#warn Exactly one of SINGLE/DOUBLE/LONGDOUBLE/SOFTQUAD should be defined!
#endif // multiple float sizes defined

#endif // CONFIG_H

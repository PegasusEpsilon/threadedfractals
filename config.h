#ifndef CONFIG_H
#define CONFIG_H

/* the width of the float types may or may not matter on your machine
 * on my laptop (core 2 duo) double is fastest. long double obviously
 * gives more accurate results (needed when rendering a deep zoom).
 */

//#define SINGLE
#define LONGDOUBLE

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
#endif

#ifndef SINGLE
#ifndef LONGDOUBLE
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
#endif
#endif

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
#endif

#ifdef SINGLE
#ifdef LONGDOUBLE
#warn Only one (or none!) of SINGLE or LONGDOUBLE should be defined!
#endif
#endif

#endif // CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H

/* the width of the float types may or may not matter on your machine
 * on my laptop (core 2 duo) double is fastest. long double obviously
 * gives more accurate results (needed when rendering a deep zoom).
 */

/*
#define FLOAT float
#define LOG logf
#define LOG2 log2f
#define SIN sinf
#define COS cosf
#define CABS cabsl
#define FABS fabsl
#define SQRT sqrtl
#define CREAL creall
#define CIMAG cimagl
*/

#define FLOAT double
#define LOG log
#define LOG2 log2
#define SIN sin
#define COS cos
#define CABS cabs
#define FABS fabs
#define SQRT sqrt
#define CREAL creal
#define CIMAG cimag

/*
#define FLOAT long double
#define LOG logl
#define LOG2 log2l
#define SIN sinl
#define COS cosl
#define CABS cabsl
#define FABS fabsl
#define SQRT sqrtl
#define CREAL creall
#define CIMAG cimagl
*/

#endif // CONFIG_H

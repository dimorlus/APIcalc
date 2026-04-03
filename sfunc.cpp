#include "pch.h"
#ifdef __BORLANDC__
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <limits>

#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"

#define M_PI_2l 1.5707963267948966192313216916398L
#define nullptr NULL
#pragma warn -8066
#pragma warn -8017
#pragma warn -8008

#else
#include <cstdint>
#include <ctime>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"
#endif

#ifdef _float128_
#define _long_double_
typedef __float128 float__t;
#define sinl   sinq
#define cosl   cosq
#define tanl   tanq
#define asinl  asinq
#define acosl  acosq
#define atanl  atanq
#define atan2l atan2q
#define sinhl  sinhq
#define coshl  coshq
#define tanhl  tanhq
#define sqrtl  sqrtq
#define cbrtl  cbrtq
#define expl   expq
#define logl   logq
#define log10l log10q
#define powl   powq
#define fabsl  fabsq
#define floorl floorq
#define ceill  ceilq
#define roundl roundq
#define hypotl hypotq
#define fmodl  fmodq
#define isnanl isnanq
#define isinfl isinfq
#endif // _float128_

#ifndef __GNUC__
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#endif // __GNUC__


#ifdef _float128_
#define M_PI    3.14159265358979323846264338327950288Q
#define M_2PI   6.283185307179586476925286766559005768394Q
#define M_PI_2l 1.57079632679489661923132169163975144Q
#define M_E     2.71828182845904523536028747135266250Q
#define PHI     1.61803398874989484820458683436563812Q //(1+sqrt(5))/2 golden ratio
#define M_PId   3.14159265358979323846264338327950288
#define M_PI_2d 1.57079632679489661923132169163975144
#define M_Ed    2.71828182845904523536028747135266250
#define PHId    1.61803398874989484820458683436563812 //(1+sqrt(5))/2 golden ratio
#else
#define M_PI    3.1415926535897932384626433832795L
#define M_2PI   6.283185307179586476925286766559005768394L
#define M_PId   3.1415926535897932384626433832795L
#define M_PI_2l 1.5707963267948966192313216916398L
#define M_E     2.7182818284590452353602874713527L
#define M_Ed    2.7182818284590452353602874713527L
#define PHI     1.6180339887498948482045868343656L //(1+sqrt(5))/2 golden ratio
#define PHId    1.6180339887498948482045868343656L
#endif /*_float128_*/

int_t To_int (int_t val)
{
 return val;
}

float__t To_float (float__t val)
{
 return val;
}

// Check if a number is prime and return the next prime number greater than or equal to n
int_t Prime (int_t n)
{
 if (n <= 3) return n;
 n |= 1;
 while (true)
  {
   int_t m = (int_t)sqrt ((float__t)n) + 1;
   int_t k = 3;
   while (true)
    {
     if (k > m) return n;
     if (n % k == 0) break;
     k += 2;
    }
   n += 2;
  }
}

// int -> float (reinterpret bits)
float__t floatf (uint64_t i) // float: take the lower 32 bits
{
 uint32_t ii = (uint32_t)(0xffffffff & i); // truncate to 32
 float f;
 memcpy (&f, &ii, 4);
 return (float__t)f;
}

float__t floatd (uint64_t i) // double / float__t (64 бит)
{
 double d;
 memcpy (&d, &i, 8);
 return (float__t)d;
}

// float -> int (reinterpret bits)
uint64_t binf (float__t x) // float -> 32-bit int
{
 float f = (float)x;
 uint32_t i;
 memcpy (&i, &f, 4);
 return (uint64_t)i;
}

uint64_t bindbl (float__t x) // double -> 64-bit int
{
 double d = (double)x;
 uint64_t i;
 memcpy (&i, &d, 8);
 return (int64_t)i; // signed, for proper display
}

// Convert integer to binary string with spaces every 8 bits
char *to_bin (int nval, char *pbuf, int nbufsize)
{
 int ncnt;
 int bcnt;
 int nlen = sizeof (int) * 8 + sizeof (int);

 if (pbuf != nullptr && nbufsize > nlen)
  {
   pbuf[nlen]     = '\0';
   pbuf[nlen - 1] = 'b';
   for (ncnt = 0, bcnt = nlen - 2; ncnt < nlen; ncnt++, bcnt--)
    {
     if (ncnt > 0 && (ncnt % 8) == 0)
      {
       pbuf[bcnt] = '.';
       bcnt--;
      }
     pbuf[bcnt] = (nval & (1 << ncnt)) ? '1' : '0';
    }
  }
 return pbuf;
}

// Euclidean Algorithm for GCD
int_t Gcd (int_t x, int_t y)
{
 while (x)
  {
   int_t r = y % x;
   y       = x;
   x       = r;
  }
 return y;
}

// Extended Euclidean Algorithm for finding modular inverse
int_t Invmod (int_t x, int_t y)
{
 int_t m = y;
 int_t u = 1, v = 0;
 int_t s = 0, t = 1;
 while (x)
  {
   int_t q = y / x;
   int_t r = y % x;
   int_t a = s - q * u;
   int_t b = t - q * v;
   y       = x;
   s       = u;
   t       = v;
   x       = r;
   u       = a;
   v       = b;
  }
 if (y != 1) return 0;
 while (s < 0) s += m;
 return s;
}

int_t Not (int_t n)
{
 return n ^ -1L;
}

// Return the current time in seconds since the epoch plus n hours
int_t Now (int_t n)
{
 return (int_t)time (nullptr) + n * 60 * 60;
}

// Error function approximation using Abramowitz and Stegun formula 7.1.26
float__t Erf (float__t x)
{
 // constants
 float__t a1 = (float__t)0.254829592L;
 float__t a2 = (float__t)-0.284496736L;
 float__t a3 = (float__t)1.421413741L;
 float__t a4 = (float__t)-1.453152027L;
 float__t a5 = (float__t)1.061405429L;
 float__t p  = (float__t)0.3275911L;
 // Save the sign of x
 int sign = (x < (float__t)0.0L) ? -1 : 1;
 x        = fabsl (x);

 // A&S formula 7.1.26
 float__t t = (float__t)1.0L / ((float__t)1.0L + p * x);
 float__t y = (float__t)1.0L - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * expl (-x * x);

 return sign * y;
}

// Complementary error function: erfc(x) = 1 - erf(x)
float__t Erfc (float__t x)
{
 return (float__t)1.0L - Erf (x);
}

// Generate a random float in the range [0, x)
float__t Random (float__t x)
{
 return (float__t)(rand () * x / (float__t)RAND_MAX);
}

// Absolute value: Abs(x)
float__t Abs (float__t x)
{
#ifdef _long_double_
 return fabsl (x);
#else
 return fabs (x);
#endif
}

// fmod function: Fmod(x, y) = x - n*y where n is the quotient of x/y rounded towards zero
float__t Fmod (float__t x, float__t y)
{
#ifdef _long_double_
 return fmodl (x, y);
#else
 return fmod (x, y);
#endif
}


// Arctangent: atan(x)
float__t Atan (float__t x)
{
#ifdef _long_double_
 return atanl (x);
#else
 return atan (x);
#endif
}

// Arccotangent: acot(x) = arctan(1/x)
float__t Acot (float__t x)
{
#ifdef _long_double_
 if (x == (float__t)0.0L) return M_PI_2l; // pi/2
 return atanl ((float__t)1.0L / x);
#else
 if (x == (float__t)0.0L) return M_PI_2; // pi/2
 return atan (1.0 / x);
#endif
}

// Arctangent of y/x: atan2(y, x)
float__t Atan2l (float__t x, float__t y)
{
 if (x != (float__t)0.0L && y != (float__t)0.0L )
  {
#ifdef _long_double_
   return atan2l (x, y);
#else
   return atan2 (x, y);
#endif
  }
 else
  return qnan; 
}

// Trigonometric functions
float__t Cos (float__t x)
{
#ifdef _long_double_
 return cosl (x);
#else
 return cos (x);
#endif
}

// Hyperbolic cosine
float__t Cosh (float__t x)
{
#ifdef _long_double_
 return coshl (x);
#else
 return cosh (x);
#endif
}

// Exponential function
float__t Exp (float__t x)
{
#ifdef _long_double_
 return expl (x);
#else
 return exp (x);
#endif
}

// Sine function
float__t Sin (float__t x)
{
#ifdef _long_double_
 return sinl (x);
#else
 return sin (x);
#endif
}

// Hyperbolic sine
float__t Sinh (float__t x)
{
#ifdef _long_double_
 return sinhl (x);
#else
 return sinh (x);
#endif
}

// Tangent function
float__t Tan (float__t x)
{
#ifdef _long_double_
 return tanl (x);
#else
 return tan (x);
#endif
}

// Hyperbolic tangent
float__t Tanh (float__t x)
{
#ifdef _long_double_
 return tanhl (x);
#else
 return tanh (x);
#endif
}

// Power function: x^y
float__t Pow (float__t x, float__t y)
{
 if ((x > (float__t)0.0L) || ((x == (float__t)0.0L) && 
     (y > (float__t)0.0L)) || ((x < (float__t)0.0L) && 
     (y - floor (y) == (float__t)0.0L)))
  {
#ifdef _long_double_
   return powl (x, y); // x^y
#else
   return pow (x, y);
#endif
  }
 else
  return qnan; 
}

// Square root function
float__t Sqrt (float__t x)
{
// return sqrtl(x);
#ifdef _long_double_
 if (x >= (float__t)0.0L)
  return sqrtl (x);
 else
  return qnan; 
#else
 if (x >= (float__t)0.0L)
  return sqrt (x);
 else
  return qnan; 
#endif
}

// Hypotenuse function: sqrt(x^2 + y^2)
float__t Hypot (float__t x, float__t y)
{
#ifdef _long_double_
 return hypotl (x, y);
#else
 return hypot (x, y);
#endif
 //return Sqrt (x * x + y * y);
}

// Arccosine function
float__t Acos (float__t x)
{
#ifdef _long_double_
 if ((x <= (float__t)1.0L) && (x >= (float__t)-1.0L))
  return acosl (x);
 else
  return qnan; 
#else
 if ((x <= (float__t)1.0L) && (x >= (float__t)-1.0L))
  return acos (x);
 else
  return qnan; 
#endif
}

// Arcsine function
float__t Asin (float__t x)
{
#ifdef _long_double_
 if ((x <= (float__t)1.0L) && (x >= (float__t)-1.0L))
  return asinl (x);
 else
  return qnan; 
#else
 if ((x <= (float__t)1.0L) && (x >= (float__t)-1.0L))
  return asin (x);
 else
  return qnan; 
#endif
}

// Natural logarithm function
float__t Log (float__t x)
{
#ifdef _long_double_
 if (x > (float__t)0.0L)
  return logl (x);
 else
  return qnan; 
#else
 if (x > (float__t)0.0L)
  return log (x);
 else
  return qnan; 
#endif
}

// Base-10 logarithm function
float__t Lg (float__t x)
{
#ifdef _long_double_
 if (x > (float__t)0.0L)
  return log10l (x);
 else
  return qnan; 
#else
 if (x > (float__t)0.0L)
  return log10 (x);
 else
  return qnan; 
#endif
}

// Convert a linear value to decibels (20 * log10(x))
float__t NP (float__t x)
{
#ifdef _long_double_
 if (x > (float__t)0.0L)
  return (float__t)20.0L * log10l (x);
 else
  return qnan; 
#else
 if (x > (float__t)0.0L)
  return (float__t)20.0L * log10 (x);
 else
  return qnan; 
#endif
}

// Convert a linear value to decibels (10 * log10(x))
float__t DB (float__t x)
{
#ifdef _long_double_
 if (x > (float__t)0.0L)
  return (float__t)10.0L * log10l (x);
 else
  return qnan; 
#else
 if (x > (float__t)0.0L)
  return (float__t)10.0L * log10 (x);
 else
  return qnan; 
#endif
}

// Hyperbolic cotangent function
float__t Ctanh (float__t x) // cth
{
#ifdef _long_double_
 if (x > 0)
  return (expl ((float__t)2.0L * x) + 1) / (expl ((float__t)2.0L * x) - 1);
 else
  return qnan; 
#else                                                 /*_long_double_*/
 if (x > 0)
  return (exp ((float__t)2.0L * x) + 1) / (exp ((float__t)2.0L * x) - 1);
 else
  return qnan; 
#endif                                                /*_long_double_*/
}

// Inverse hyperbolic functions
float__t Arsh (float__t x)
{
#ifdef _long_double_
 return logl (x + sqrt (x * x + (float__t)1.0L));
#else  /*_long_double_*/
 return log (x + sqrt (x * x + (float__t)1.0L));
#endif /*_long_double_*/
}

// Inverse hyperbolic cosine function
float__t Arch (float__t x)
{
#ifdef _long_double_
 return logl (x + sqrtl (x * x - (float__t)1.0L));
#else  /*_long_double_*/
 return log (x + sqrt (x * x - (float__t)1.0L));
#endif /*_long_double_*/
}

// Inverse hyperbolic tangent function
float__t Arth (float__t x)
{
 if ((x == (float__t)1.0L) || (x == (float__t)-1.0L))
  return qnan; 
#ifdef _long_double_
 return logl ((1 + x) / (1 - x)) / (float__t)2.0L;
#else  /*_long_double_*/
 return log ((1 + x) / (1 - x)) / (float__t)2.0L;
#endif /*_long_double_*/
}

// Inverse hyperbolic cotangent function
float__t Arcth (float__t x)
{
 if ((x == (float__t)1.0L) || (x == (float__t)-1.0L))
  return qnan; 
#ifdef _long_double_
 return logl ((x + (float__t)1.0L) / (x - (float__t)1.0L)) / (float__t)2.0L;
#else  /*_long_double_*/
 return log ((x + (float__t)1.0L) / (x - (float__t)1.0L)) / (float__t)2.0L;
#endif /*_long_double_*/
}

//  Convert decibels to linear scale (10^(x/20))
float__t Exp10 (float__t x)
{
#ifdef _long_double_
 return expl (x * logl ((float__t)10.0L));
#else  /*_long_double_*/
 return exp (x * log ((float__t)10.0L));
#endif /*_long_double_*/
}

// Convert decibels to linear scale (10^(x/20))
float__t ANP (float__t x)
{
 return Exp10 (x / (float__t)20.0L);
}

// Convert decibels to linear scale (10^(x/10))
float__t ADB (float__t x)
{
 return Exp10 (x / (float__t)10.0L);
}

// Trigonometric functions with degree input
float__t Sing (float__t x)
{
#ifdef _long_double_
 return sinl (M_PI * x / (float__t)180.0L);
#else  /*_long_double_*/
 return sin (M_PI * x / (float__t)180.0L);
#endif /*_long_double_*/
}

// Cosine function with degree input
float__t Cosg (float__t x)
{
#ifdef _long_double_
 return cosl (M_PI * x / (float__t)180.0L);
#else  /*_long_double_*/
 return cos (M_PI * x / (float__t)180.0L);
#endif /*_long_double_*/
}

// Tangent function with degree input
float__t Tgg (float__t x)
{
#ifdef _long_double_
 return tanl (M_PI * x / (float__t)180.0L);
#else  /*_long_double_*/
 return tan (M_PI * x / (float__t)180.0L);
#endif /*_long_double_*/
}

// Cotangent function with degree input
float__t Ctgg (float__t x)
{
 if (x == (float__t)0.0L) return qnan; 
#ifdef _long_double_
 return 1 / tanl (M_PI * x / (float__t)180.0L);
#else  /*_long_double_*/
 return 1 / tan (M_PI * x / (float__t)180.0L);
#endif /*_long_double_*/
}

// Cotangent function
float__t Ctg (float__t x)
{
 if (x == (float__t)0.0L) return qnan; 
#ifdef _long_double_
 return 1 / tanl (x);
#else  /*_long_double_*/
 return 1 / tan (x);
#endif /*_long_double_*/
}

// Round a float to the nearest integer
float__t Round (float__t x)
{
 return (float__t)((int)(x + (float__t)0.5L));
}

// Round a float to the nearest integer towards positive infinity
float__t Ceil (float__t x)
{
 return (float__t)((int)(x + (float__t)1.0L));
}

// Round a float to the nearest integer towards negative infinity
float__t Floor (float__t x)
{
 return (float__t)((int)(x));
}

// Get the fractional part of a float
float__t Frac (float__t x)
{
 float__t d;
#ifdef _long_double_
#ifdef _float128_
 return modfq (x, &d);
#else
 return modfl (x, &d);
#endif /*_float128_*/
#else
 return modf (x, &d);
#endif
}

// Get the integer part of a float
float__t Int (float__t x)
{
 return (float__t)((int_t)x);
}

// Identity function for floats
float__t Float (float__t x)
{
 return x;
}


// Get the minimum of two floats
float__t Min (float__t x, float__t y)
{
 return (x < y) ? x : y;
}

// Get the maximum of two floats
float__t Max (float__t x, float__t y)
{
 return (x > y) ? x : y;
}

// Get the base-2 logarithm of a float
float__t Log2 (float__t x)
{
 if (x <= (float__t)0.0L) return qnan; 
#ifdef _long_double_
 return logl (x) / logl ((float__t)2.0L);
#else  /*_long_double_*/
 return log (x) / log ((float__t)2.0L);
#endif /*_long_double_*/
}

// Get the logarithm of y to the base x
float__t Logn (float__t x, float__t y)
{
 if (x <= (float__t)0.0L) return qnan; 
 if (y <= (float__t)0.0L) return qnan; 
#ifdef _long_double_
 return logl (y) / logl (x);
#else  /*_long_double_*/
 return log (y) / log (x);
#endif /*_long_double_*/
}

// Get the cube root of a float
float__t Root3 (float__t x)
{
#ifdef _long_double_
#ifdef __BORLANDC__
 return powl (x, (float__t)1.0L / (float__t)3.0L);
#else
 return cbrtl (x);
#endif
 #else  /*_long_double_*/
#ifdef __BORLANDC__
 return pow (x, (float__t)1.0L / (float__t)3.0L);
#else
 return cbrt (x);
#endif
#endif /*_long_double_*/
}

// Get the y-th root of x
float__t Rootn (float__t x, float__t y)
{
#ifdef _long_double_
 return powl (x, (float__t)1.0L / y);
#else  /*_long_double_*/
 return pow (x, (float__t)1.0L / y);
#endif /*_long_double_*/
}

/*SWG Table https://en.wikipedia.org/wiki/Standard_wire_gauge*/
double tswg[] = { 12.700, 11.786, 10.973, 10.160, 9.449,  8.839,  8.230,  7.620,  7.010,  6.401,
                  5.893,  5.385,  4.877,  4.470,  4.064,  3.658,  3.251,  2.946,  2.642,  2.337,
                  2.032,  1.829,  1.626,  1.422,  1.219,  1.016,  0.914,  0.813,  0.711,  0.610,
                  0.559,  0.5080, 0.4572, 0.4166, 0.3759, 0.3454, 0.3150, 0.2946, 0.2743, 0.2540,
                  0.2337, 0.2134, 0.1930, 0.1727, 0.1524, 0.1321, 0.1219, 0.1118, 0.1016, 0.0914,
                  0.0813, 0.0711, 0.0610, 0.0508, 0.0406, 0.0305, 0.0254 };

// Convert SWG to mm
float__t Swg (float__t x) // SWG to mm
{
 int n = (int)x;
 if ((n >= -6) && (n <= 50))
  return (float__t)tswg[n + 6];
 else
  return (float__t)0.0L;
}

// Convert SWG to mm^2
float__t SSwg (float__t x) // SWG to mm^2
{
#ifdef _long_double_
 return powl (Swg (x), (float__t)2.0L) * M_PI / (float__t)4.0L;
#else  /*_long_double_*/
 return pow (Swg (x), (float__t)2.0L) * M_PI / (float__t)4.0L;
#endif /*_long_double_*/
}

// Convert mm to SWG
float__t Aswg (float__t x) // mm to SWG
{
 if ((x < 0.0254) || (x > 12.7))
  return qnan; 
 int n = 0;
 while (tswg[n] >= (double)x) n++;
 if (n)
  {
   if (fabs (tswg[n] - (double)x) < fabs (tswg[n - 1] - (double)x))
    return (float__t)(n - 6);
   else
    return (float__t)(n - 7);
  }
 else
  return (float__t)(-6);
}


/* https://en.wikipedia.org/wiki/American_wire_gauge */
// AWG to mm: d = 0.127 * 92^((36-n)/39) = exp(2.1104 - 0.11594 * n)
float__t Awg (float__t x) // AWG to mm
{
#ifdef _long_double_
 return expl ((float__t)2.1104L - (float__t)0.11594L * x);
#else  /*_long_double_*/
 return exp ((float__t)2.1104L - (float__t)0.11594L * x);
#endif /*_long_double_*/
}

// AWG to mm^2: A = pi * (d/2)^2 = pi * (exp(2.1104 - 0.11594 * n)/2)^2
float__t SAwg (float__t x) // AWG to mm^2
{
#ifdef _long_double_
 return powl (exp ((float__t)2.1104L - (float__t)0.11594L * x), (float__t)2.0L) * M_PI / (float__t)4.0L;
#else  /*_long_double_*/
 return pow (exp ((float__t)2.1104L - (float__t)0.11594L * x), (float__t)2.0L) * M_PI / (float__t)4.0L;
#endif /*_long_double_*/
}

// Convert diameter to mm^2: A = pi * (d/2)^2 = pi * d^2 / 4
float__t Cs (float__t x) // diameter to mm^2
{
#ifdef _long_double_
 return powl (x, (float__t)2.0L) * M_PI / ((float__t)4.0L);
#else  /*_long_double_*/
 return pow (x, (float__t)2.0L) * M_PI / (float__t)4.0L;
#endif /*_long_double_*/
}
// Convert mm^2 to diameter: d = sqrt(4 * A / pi)
float__t Acs (float__t x) // mm^2 to diameter
{
#ifdef _long_double_
 return sqrtl ((float__t)4.0L * x / M_PI);
#else  /*_long_double_*/
 return sqrt ((float__t)4.0L * x / M_PI);
#endif /*_long_double_*/
}

// Convert mm to AWG: n = 36 - 39 * log(d/0.127) / log(92) = (2.1104 - log(x)) / 0.11594
float__t Aawg (float__t x) // mm to AWG
{
#ifdef _long_double_
 return (float__t)(((float__t)2.1104L - logl (x)) / (float__t)0.11594L);
#else  /*_long_double_*/
 return (float__t)(((float__t)2.1104L - log (x)) / (float__t)0.11594L);
#endif /*_long_double_*/
}

/* https://en.wikipedia.org/wiki/E_series_of_preferred_numbers */
double E3[]  = { 1.0, 2.2, 4.7, 10 };
double E6[]  = { 1.0, 1.5, 2.2, 3.3, 4.7, 6.8, 10 };
double E12[] = { 1.0, 1.2, 1.5, 1.8, 2.2, 2.7, 3.3, 3.9, 4.7, 5.6, 6.8, 8.2, 10 };
double E24[] = { 1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0, 2.2, 2.4, 2.7, 3.0, 3.3,
                 3.6, 3.9, 4.3, 4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1, 10 };
double E48[] = { 1.00, 1.05, 1.10, 1.15, 1.21, 1.27, 1.33, 1.40, 1.47, 1.54, 1.62, 1.69, 1.78,
                 1.87, 1.96, 2.05, 2.15, 2.26, 2.37, 2.49, 2.61, 2.74, 2.87, 3.01, 3.16, 3.32,
                 3.48, 3.65, 3.83, 4.02, 4.22, 4.42, 4.64, 4.87, 5.11, 5.36, 5.62, 5.90, 6.19,
                 6.49, 6.81, 7.15, 7.50, 7.87, 8.25, 8.66, 9.09, 9.53, 10,   10 };
double E96[] = { 1.00, 1.02, 1.05, 1.07, 1.10, 1.13, 1.15, 1.18, 1.21, 1.24, 1.27, 1.30, 1.33, 1.37,
                 1.40, 1.43, 1.47, 1.50, 1.54, 1.58, 1.62, 1.65, 1.69, 1.74, 1.78, 1.82, 1.87, 1.91,
                 1.96, 2.00, 2.05, 2.10, 2.15, 2.21, 2.26, 2.32, 2.37, 2.43, 2.49, 2.55, 2.61, 2.67,
                 2.74, 2.80, 2.87, 2.94, 3.01, 3.09, 3.16, 3.24, 3.32, 3.40, 3.48, 3.57, 3.65, 3.74,
                 3.83, 3.92, 4.02, 4.12, 4.22, 4.32, 4.42, 4.53, 4.64, 4.75, 4.87, 4.99, 5.11, 5.23,
                 5.36, 5.49, 5.62, 5.76, 5.90, 6.04, 6.19, 6.34, 6.49, 6.65, 6.81, 6.98, 7.15, 7.32,
                 7.50, 7.68, 7.87, 8.06, 8.25, 8.45, 8.66, 8.87, 9.09, 9.31, 9.53, 9.76, 10 };
double E192[]
    = { 1.00, 1.01, 1.02, 1.04, 1.05, 1.06, 1.07, 1.09, 1.10, 1.11, 1.13, 1.14, 1.15, 1.17, 1.18,
        1.20, 1.21, 1.23, 1.24, 1.26, 1.27, 1.29, 1.30, 1.32, 1.33, 1.35, 1.37, 1.38, 1.40, 1.42,
        1.43, 1.45, 1.47, 1.49, 1.50, 1.52, 1.54, 1.56, 1.58, 1.60, 1.62, 1.64, 1.65, 1.67, 1.69,
        1.72, 1.74, 1.76, 1.78, 1.80, 1.82, 1.84, 1.87, 1.89, 1.91, 1.93, 1.96, 1.98, 2.00, 2.03,
        2.05, 2.08, 2.10, 2.13, 2.15, 2.18, 2.21, 2.23, 2.26, 2.29, 2.32, 2.34, 2.37, 2.40, 2.43,
        2.46, 2.49, 2.52, 2.55, 2.58, 2.61, 2.64, 2.67, 2.71, 2.74, 2.77, 2.80, 2.84, 2.87, 2.91,
        2.94, 2.98, 3.01, 3.05, 3.09, 3.12, 3.16, 3.20, 3.24, 3.28, 3.32, 3.36, 3.40, 3.44, 3.48,
        3.52, 3.57, 3.61, 3.65, 3.70, 3.74, 3.79, 3.83, 3.88, 3.92, 3.97, 4.02, 4.07, 4.12, 4.17,
        4.22, 4.27, 4.32, 4.37, 4.42, 4.48, 4.53, 4.59, 4.64, 4.70, 4.75, 4.81, 4.87, 4.93, 4.99,
        5.05, 5.11, 5.17, 5.23, 5.30, 5.36, 5.42, 5.49, 5.56, 5.62, 5.69, 5.76, 5.83, 5.90, 5.97,
        6.04, 6.12, 6.19, 6.26, 6.34, 6.42, 6.49, 6.57, 6.65, 6.73, 6.81, 6.90, 6.98, 7.06, 7.15,
        7.23, 7.32, 7.41, 7.50, 7.59, 7.68, 7.77, 7.87, 7.96, 8.06, 8.16, 8.25, 8.35, 8.45, 8.56,
        8.66, 8.76, 8.87, 8.98, 9.09, 9.20, 9.31, 9.42, 9.53, 9.65, 9.76, 9.88, 10 };
// Find the nearest standard value from E-series
float__t Ee (float__t x, float__t y) // find standard value
{
 if (x >= (float__t)0.0L && y >= (float__t)0.0L)
  {
   unsigned int n;
   unsigned int N = (unsigned int)x;
   double delta_n = 10;
   double delta_n1;
   double V = fabs ((double)y);

   /* normalize */
   int ex = 0;
   while (V >= 1)
    {
     V /= 10;
     ex++;
    }
   while (V < 1)
    {
     V *= 10;
     ex--;
    }

   if (N <= 192)
    {
     unsigned int nn = log10 (V) * N + 0.5;
     if (nn > N) nn = N;
     switch (N)
      {
      case 3:
       if ((nn >= 0) && (nn < sizeof (E3)/sizeof(E3[0])))
        V = E3[nn];
       else
        V = qnan; 
       break;
      case 6:
       if ((nn >= 0) && (nn < sizeof (E6)/sizeof(E6[0])))
        V = E6[nn];
       else
        V = qnan; 
       break;
      case 12:
       if ((nn >= 0) && (nn < sizeof (E12)/sizeof(E12[0])))
        V = E12[nn];
       else
        V = qnan; 
       break;
      case 24:
       if ((nn >= 0) && (nn < sizeof (E24)/sizeof(E24[0])))
        V = E24[nn];
       else
        V = qnan; 
       break;
      case 48:
       if ((nn >= 0) && (nn < sizeof (E48)/sizeof(E48[0])))
        V = E48[nn];
       else
        V = qnan; 
       break;
      case 96:
       if ((nn >= 0) && (nn < sizeof (E96)/sizeof(E96[0])))
        V = E96[nn];
       else
        V = qnan; 
       break;
      case 192:
       if ((nn >= 0) && (nn < sizeof (E192)/sizeof(E192[0])))
        V = E192[nn];
       else
        V = qnan; 
       break;
      default:
       V = qnan; 
       break;
      }
    }
   else
    {
     for (n = 0; n < N; n++)
      {
       double v = exp (log (10.0) * n / N);
       delta_n1 = fabs (V - v);
       if (delta_n1 > delta_n)
        {
         break;
        }
       delta_n = delta_n1;
      }
     V = (floor ((0.5 + exp (log (10.0) * (n) / N) * 100))) / 100;
    }
   /* denormalize */
   while (ex)
    if (ex > 0)
     {
      V *= 10.0;
      ex--;
     }
    else
     {
      V /= 10.0;
      ex++;
     }

   if (y < 0)
    return (float__t)(-V);
   else
    return (float__t)V;
  }
 else
  return (float__t)0.0L;
}

// Calculate the factorial of a number
float__t Factorial (float__t x)
{
 if ((x >= 0) && (x - Floor (x) == 0.0))
  {
   int n        = (int)x;
   float__t res = (float__t)1.0L;
   for (; n > 1; n--) res = res * (float__t)n;
   return res;
  }
 else 
  {
   float__t re, im;
   FactorialC (x, 0, re, im);
   return re;
  }
}

// Convert Celsius to Farenheit
float__t Farenheit (float__t x)
{
 return (float__t)((x - (float__t)32.0L) * (float__t)5.0L / (float__t)9.0L);
}

// Compare two floats with a given precision
float__t Cmp (float__t x, float__t y, float__t prec)
{
#ifdef _long_double_
 float__t rel_diff = fabsl (x - y) / (x > y ? x : y);
#else
 float__t rel_diff = fabs (x - y) / (x > y ? x : y);
#endif
 if (rel_diff < prec)
  return (float__t)0.0L;
 else if (x < y)
  return (float__t)-1.0L;
 else
  return (float__t)1.0L;
}

// Vref=Vout*Rl/(Rh+Rl)
// Vout=Vref*(Rh+Rl)/Rl
// Calculate the output voltage of a voltage divider
float__t Vout (float__t Vref, float__t Rh, float__t Rl)
{
 return Vref * (Rh + Rl) / Rl;
}

#ifdef __BORLANDC__

extern "C" {
    int _matherr(struct _exception *e) { return 1; }
    int _matherrl(struct _exceptionl *e) { return 1; } // long double
}

//Check if a double value is NaN (Not a Number)
bool IsNaN (const double fVal)
{
 return (((*(__int64 *)(&fVal) & 0x7FF0000000000000ull) == 0x7FF0000000000000ull)
         && ((*(__int64 *)(&fVal) & 0x000FFFFFFFFFFFFFull) != 0x0000000000000000ull));
}

// Check if a float__t value is NaN (Not a Number)
bool IsNaNL (const float__t ldVal)
{
 return IsNaN ((double)ldVal);
}

bool IsInf (const double fVal)
{
 return (((*(__int64 *)(&fVal) & 0x7FF0000000000000ull) == 0x7FF0000000000000ull)
         && ((*(__int64 *)(&fVal) & 0x000FFFFFFFFFFFFFull) == 0x0000000000000000ull));
}

bool IsInfL (const float__t ldVal)
{
 return IsInf ((double)ldVal);
}

const unsigned short neg_inf_bits[5] = {0, 0, 0, 0x8000, 0xFFFF};
const long double neg_inf = *(long double*)neg_inf_bits;
#endif


// Extract the format string from a printf-style format and copy it to the destination buffer
int fmtc (char *dst, char *fmt)
{
 char c;
 int i = 0;
 do
  {
   c = fmt[i];
   if (c == '%') break;
   dst[i++] = c;
  }
 while (c && (i < STRBUF));
 dst[i] = '\0';
 return i;
}

// prn("%1.30LG",(x:=1.84467440737095536e19;(x-1)/x))
// Format a string with the given format and arguments, and copy it to the destination buffer
// Fixes:
//   - param[] and p are now reset for each format specifier
//   - tSci, tFract, tNrm, tDeg now use float__t instead of double
//   - added dest size limit (STRBUF) to prevent buffer overrun
//   - clarified sfmt-- comment at end of inner loop

int_t fprn (char *dest, char *sfmt, int args, char ic, value *v_stack)
{
 char pfmt[STRBUF];
 enum ftypes
 {
  tNone,
  tBin,
  tComp,
  tChar,
  tSpc,
  tSci,
  tNrm,
  tTime,
  tInt,
  tFloat,
  tFract,
  tDeg,
  tString,
  tPtr
 } fmt;
 char c, cc;
 char param[16];
 int p;
 int i;
 int n = 0;
 bool flag;
 char *dst     = dest;
 char *dst_end = dest + 128; // leave room 
 if (!sfmt) return 0;
 do
  {
   c    = '\0';
   i    = 0;
   flag = false;
   fmt  = tNone;
   // reset per-specifier state
   memset (param, 0, sizeof (param));
   p = 0;
   do
    {
     cc = c;
     c = pfmt[i++] = *sfmt++;
     pfmt[i]       = '\0';
     if (flag)
      {
       if (c == '*')
        {
         i--;      // step back to overwrite '*' with next character in format string
         continue; // skip '*' width/precision specifier; we don't support it, but we also don't
                   // want to treat it as part of the format type
        }
       if (c == '%')
        {
         flag = false;
         continue;
        }
       else if ((c == 'f') || (c == 'e') || (c == 'E') || (c == 'g') || (c == 'G'))
        {
         fmt = tFloat;
         break;
        }
       else if ((c == 'd') || (c == 'i') || (c == 'u') || (c == 'x') || (c == 'X') || (c == 'o'))
        {
         fmt = tInt;
         break;
        }
       else if (c == 'c')
        {
         fmt = tChar;
         break;
        }
       else if (c == 'C')
        {
         fmt = tSpc;
         break;
        }
       else if (c == 'D')
        {
         fmt = tDeg;
         break;
        }
       else if (c == 'S')
        {
         fmt = tSci;
         break;
        }
       else if (c == 'F')
        {
         fmt = tFract;
         break;
        }
       else if (c == 'N')
        {
         fmt = tNrm;
         break;
        }
       else if (c == 't')
        {
         fmt = tTime;
         break;
        }
       else if (c == 'b')
        {
         fmt = tBin;
         break;
        }
       else if (c == 'B')
        {
         fmt = tComp;
         break;
        }
       else if (c == 's')
        {
         fmt = tString;
         break;
        }
       else if ((c == 'n') || (c == 'p'))
        {
         fmt = tPtr;
         break;
        }
       else
        {
         if (p < (int)sizeof (param) - 1) param[p++] = c;
         param[p] = '\0';
         continue;
        }
      }
     else if (c == '%')
      {
       flag = true;
       continue;
      }
    }
   while (c && (i < STRBUF - 2));

   if (!c) sfmt--; // sfmt stepped past '\0'; step back so outer while (*sfmt) terminates

   if (n < args)
    {
     switch (fmt)
      {
      case tNone:
       if (dst + i < dst_end) dst += sprintf (dst, pfmt);
       break;
      case tPtr:
       if (dst + i < dst_end)
        {
         strcpy (dst, pfmt);
         dst += strlen (pfmt);
        }
       break;
      case tComp:
       {
        double cd = (double)v_stack[n].get ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += b2scistr (dst, cd);
       }
       break;
      case tBin:
       {
        __int64 bi = v_stack[n].get_int ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += b2str (dst, pfmt, bi);
       }
       break;
      case tChar:
       {
        if (cc == 'l')
         {
          int ii = v_stack[n].get_int ();
          if (dst < dst_end) dst += fmtc (dst, pfmt);
          if (dst < dst_end) dst += wchr2str (dst, ii);
         }
        else
         {
          char ci = (char)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, ci);
         }
       }
       break;
      case tSpc:
       {
        char ci = (char)v_stack[n].get_int ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += chr2str (dst, ci);
       }
       break;
      case tSci:
       {
        double dd = (double)v_stack[n].get ();
        double di = (double)v_stack[n].imval;
        normz (dd, di);
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += d2scistr (dst, dd);
        if (di != 0.0)
         {
          *dst++ = (di > 0.0) ? '+' : '-';
          di = fabs (di);
          if (dst < dst_end) dst += d2scistr (dst, di);
          if (dst < dst_end) *dst++ = ic;
         }
       }
       break;
      case tFract:
       {
        int en      = 0;
        double dd = (double)v_stack[n].get (); 
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (param[0]) en = atoi (param);
        if (dst < dst_end) dst += d2frcstr (dst, dd, en);
       }
       break;
      case tNrm:
       {
        double dd = (double)v_stack[n].get (); // use double, not float__t
        double di = (double)v_stack[n].imval;
        normz (dd, di);
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += d2nrmstr (dst, dd);
        if (di != 0.0)
         {
          *dst++ = (di > 0.0) ? '+' : '-'; 
          di = fabs (di);
          if (dst < dst_end) dst += d2nrmstr (dst, di);
          if (dst < dst_end) *dst++ = ic;
         }
       }
       break;
      case tTime:
       {
        __int64 bi = v_stack[n].get_int ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += t2str (dst, bi);
       }
       break;
      case tInt:
       {
        if (cc == 'l')
         {
          long li = (long)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, li);
         }
        else if (cc == 'L')
         {
          __int64 Li = v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, Li);
         }
        else if (cc == 'h')
         {
          short hi = (short)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, hi);
         }
        else
         {
          int ii = (int)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, ii);
         }
       }
       break;
      case tFloat:
       {
        char sstr[2048];
        char *sptr = sstr;
        char *sp = sstr;
        char *sstr_end = sstr + sizeof (sstr) - 16; // leave room for '\0' and potential extra chars
        if (cc == 'L')
         {
#ifdef __GNUC__
          __float80 Ld = (__float80)v_stack[n].get ();
          __float80 Li = (__float80)v_stack[n].imval;
          sptr += snprintf (sptr, sstr_end - sptr, pfmt, Ld);
          if (Li != 0.0L)
           {
            *sptr++ = (Li > 0.0L) ? '+' : '-';
            Li      = (Li > 0.0L) ? Li  : -Li;
            sptr += snprintf (sptr, sstr_end - sptr, pfmt, Li);
            *sptr++ = ic;
           }
          while (dst < dst_end && sp < sptr) *dst++ = *sp++;
#else
          long double Ld = (long double)v_stack[n].get ();
          long double Li = (long double)v_stack[n].imval;
          sptr += snprintf (sptr, sstr_end - sptr, pfmt, Ld);
          if (Li != 0.0L)
           {
            *sptr++ = (Li > 0.0L) ? '+' : '-';
            Li      = (Li > 0.0L) ? Li : -Li;
            sptr += snprintf (sptr, sstr_end - sptr, pfmt, Li);
            *sptr++ = ic;
           }
          while (dst < dst_end && sp < sptr) *dst++ = *sp++;
#endif
         }
        else
         {
          double dd = (double)v_stack[n].get ();
          double di = (double)v_stack[n].imval;
          sptr += snprintf (sptr, sstr_end - sptr, pfmt, dd);
          if (di != 0.0)
           {
            *sptr++  = (di > 0.0) ? '+' : '-';
            di = fabs (di);
            sptr += snprintf (sptr,sstr_end - sptr, pfmt, di);
            *sptr++ = ic;
           }
          while (dst < dst_end && sp < sptr) *dst++ = *sp++;
         }
       }
       break;
      case tDeg:
       {
        double dd = (double)v_stack[n].get ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += dgr2str (dst, dd);
       }
       break;
      case tString:
       if (dst < dst_end) dst += sprintf (dst, pfmt, v_stack[n].sval);
       break;
      }
     n++;
    }
   else
    {
     // no more arguments - output remaining literal text from format string
     if (dst < dst_end) dst += sprintf (dst, pfmt);
    }
  }
 while (*sfmt && (i < STRBUF) && (n <= args));

 *dst = '\0';
 return dst - dest;
}

#ifdef __BORLANDC__
int get_timezone(void) //return seconds
{
    return -_timezone;
}
#else
// Get the timezone offset in seconds
int get_timezone (void) // return seconds
{
 TIME_ZONE_INFORMATION tzi;
 DWORD tzResult    = GetTimeZoneInformation (&tzi);
 long timezoneBias = tzi.Bias; // in minutes
 int tzHours       = timezoneBias * 60;
 return tzHours;
}
#endif
// Convert a date-time string to a timestamp (seconds since epoch)
int_t datatime (char *tstr)
{
 time_t result = 0;
 int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;

 if (tstr && sscanf (tstr, "%4d.%2d.%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec))
  {
   struct tm breakdown = { 0 };
   breakdown.tm_year   = year - 1900; /* years since 1900 */
   breakdown.tm_mon    = month - 1;
   breakdown.tm_mday   = day;
   breakdown.tm_hour   = hour;
   breakdown.tm_min    = min;
   breakdown.tm_sec    = sec;
   result              = mktime (&breakdown) - get_timezone ();
  }
 return (int_t)result;
}

// Get information about the type of electromagnetic wave based on its wavelength in meters
const char *wavelength_info (float__t wavelength)
{
 // wavelength_m - wavelength in meters
 double wavelength_m = (double)wavelength; // use double for more precision in comparisons
 if (wavelength_m > 10000)
  {
   return "VLF radio waves (3-30 kHz, 10-100 km, mid: 31.6 km)";
  }
 else if (wavelength_m > 1000)
  {
   return "LF radio waves (30-300 kHz, 1-10 km, mid: 3.16 km)";
  }
 else if (wavelength_m > 100)
  {
   return "MF radio waves (300-3000 kHz, 100-1000 m, AM, mid: 316 m)";
  }
 else if (wavelength_m > 10)
  {
   return "HF radio waves (3-30 MHz, 10-100 m, shortwave, mid: 31.6 m)";
  }
 else if (wavelength_m > 1)
  {
   return "VHF radio waves (30-300 MHz, 1-10 m, FM/TV, mid: 3.16 m)";
  }
 else if (wavelength_m > 0.1)
  {
   return "UHF radio waves (300-3000 MHz, 0.1-1 m, mid: 31.6 cm)";
  }
 else if (wavelength_m > 0.03)
  {
   return "L-band microwaves (1-2 GHz, 15-30 cm, GPS, mid: 20 cm)";
  }
 else if (wavelength_m > 0.015)
  {
   return "S-band (2-4 GHz, 7.5-15 cm, WiFi/radar, mid: 10 cm)";
  }
 else if (wavelength_m > 0.0075)
  {
   return "C-band microwaves (4-8 GHz, 3.75-7.5 cm, sat, mid: 5 cm)";
  }
 else if (wavelength_m > 0.00375)
  {
   return "X-band microwaves (8-12 GHz, 2.5-3.75 cm, mid: 3 cm)";
  }
 else if (wavelength_m > 0.0025)
  {
   return "Ku-band (12-18 GHz, 1.67-2.5 cm, sat TV, mid: 2 cm)";
  }
 else if (wavelength_m > 0.00167)
  {
   return "K-band (18-27 GHz, 1.11-1.67 cm, radar, mid: 1.33 cm)";
  }
 else if (wavelength_m > 0.001)
  {
   return "Ka-band (27-40 GHz, 7.5-11.1 mm, mid: 8.96 mm)";
  }
 else if (wavelength_m > 1e-4)
  {
   return "Far infrared (FIR, 100-1000 um, mid: 316 um)";
  }
 else if (wavelength_m > 1e-5)
  {
   return "EHF/mmWave (30-300 GHz, 10-100 um, 5G, mid: 31.6 um)";
  }
 else if (wavelength_m > 8e-6)
  {
   return "Long-wave IR (LWIR, 8-15 um, thermal, mid: 11.5 um)";
  }
 else if (wavelength_m > 3e-6)
  {
   return "Mid-wave IR (MWIR, 3-8 um, night vision, mid: 5.5 um)";
  }
 else if (wavelength_m > 1.4e-6)
  {
   return "Short-wave IR (SWIR, 1.4-3 um, fiber, mid: 2.2 um)";
  }
 else if (wavelength_m > 7.8e-7)
  {
   return "Near IR (NIR, 0.78-1.4 um, remote, mid: 1.09 um)";
  }
 else if (wavelength_m > 6.2e-7)
  {
   return "Red visible light (620-780 nm, mid: 700 nm)";
  }
 else if (wavelength_m > 5.97e-7)
  {
   return "Orange visible light (597-620 nm, mid: 608.5 nm)";
  }
 else if (wavelength_m > 5.77e-7)
  {
   return "Yellow visible light (577-597 nm, mid: 587 nm)";
  }
 else if (wavelength_m > 4.92e-7)
  {
   return "Green visible light (492-577 nm, mid: 534.5 nm)";
  }
 else if (wavelength_m > 4.55e-7)
  {
   return "Blue visible light (455-492 nm, mid: 473.5 nm)";
  }
 else if (wavelength_m > 3.8e-7)
  {
   return "Violet visible light (380-455 nm, mid: 417.5 nm)";
  }
 else if (wavelength_m > 3.15e-7)
  {
   return "UVA ultraviolet (315-380 nm, tanning, mid: 347.5 nm)";
  }
 else if (wavelength_m > 2.8e-7)
  {
   return "UVB ultraviolet (280-315 nm, sunburn, mid: 297.5 nm)";
  }
 else if (wavelength_m > 1e-7)
  {
   return "UVC ultraviolet (100-280 nm, germicidal, mid: 190 nm)";
  }
 else if (wavelength_m > 1e-11)
  {
   return "X-ray radiation (0.01-10 nm, imaging, mid: 1 nm)";
  }
 else
  {
   return "Gamma ray radiation (<0.01 nm, nuclear)";
  }
}

// Function:
//
// Accepts wavelength in meters
// Returns RGB color in 0xRRGGBB (uint32_t) format
// Outside the visible spectrum (380 - 780 nm) returns black (0x000000)
// Uses spectral color approximation
// Applies intensity correction at the edges (the eye is less sensitive to violet and deep red)
// Applies gamma correction for more realistic display
//
// Usage examples:
//
// 650 nm (red): wavelength_to_rgb(650e-9) → about 0xFF0000
// 550 nm (green): wavelength_to_rgb(550e-9) → about 0x00FF00
// 450 nm (blue): wavelength_to_rgb(450e-9) → about 0x0000FF

uint32_t wavelength_to_rgb (float__t wavelength_m)
{
 // Convert to nanometers for convenience
 double wavelength_nm = (double)(wavelength_m * 1e9);

 // Visible range: approximately 380-780 nm
 if (wavelength_nm < 380 || wavelength_nm > 780)
  {
   return 0x000000; // Black outside the visible spectrum
  }

 double red = 0, green = 0, blue = 0;

 // Spectral color approximation
 if (wavelength_nm >= 380 && wavelength_nm < 440)
  {
   // Violet -> Blue
   red   = -(wavelength_nm - 440) / (440 - 380);
   green = 0.0;
   blue  = 1.0;
  }
 else if (wavelength_nm >= 440 && wavelength_nm < 490)
  {
   // Blue -> Cyan
   red   = 0.0;
   green = (wavelength_nm - 440) / (490 - 440);
   blue  = 1.0;
  }
 else if (wavelength_nm >= 490 && wavelength_nm < 510)
  {
   // Cyan -> Green
   red   = 0.0;
   green = 1.0;
   blue  = -(wavelength_nm - 510) / (510 - 490);
  }
 else if (wavelength_nm >= 510 && wavelength_nm < 580)
  {
   // Green -> Yellow
   red   = (wavelength_nm - 510) / (580 - 510);
   green = 1.0;
   blue  = 0.0;
  }
 else if (wavelength_nm >= 580 && wavelength_nm < 645)
  {
   // Yellow -> Orange -> Red
   red   = 1.0;
   green = -(wavelength_nm - 645) / (645 - 580);
   blue  = 0.0;
  }
 else if (wavelength_nm >= 645 && wavelength_nm <= 780)
  {
   // Red
   red   = 1.0;
   green = 0.0;
   blue  = 0.0;
  }

 // Intensity correction at the spectrum edges (eye is less sensitive)
 double factor = 1.0;
 if (wavelength_nm >= 380 && wavelength_nm < 420)
  {
   factor = 0.3 + 0.7 * (wavelength_nm - 380) / (420 - 380);
  }
 else if (wavelength_nm >= 700 && wavelength_nm <= 780)
  {
   factor = 0.3 + 0.7 * (780 - wavelength_nm) / (780 - 700);
  }

 red *= factor;
 green *= factor;
 blue *= factor;

 // Gamma correction (closer to eye perception)
 double gamma = 0.8;
 red          = pow (red, gamma);
 green        = pow (green, gamma);
 blue         = pow (blue, gamma);

 // Convert to 8-bit components and pack into uint32_t
 /*uint8_t r = (uint8_t)(red * 255);
 uint8_t g = (uint8_t)(green * 255);
 uint8_t b = (uint8_t)(blue * 255);*/

 uint32_t result
     = (uint32_t)(blue * 255) + 256 * (uint32_t)(green * 255) + 65536 * (uint32_t)(red * 255);
 return result;
}

// The function uses the black body radiation approximation algorithm according to Planck's law:
//
// Accepts temperature in Kelvin
// Returns RGB color in 0xRRGGBB format
// For temperatures below 1000K returns black
//
// Example temperatures:
//
// 1000K: Red-orange glow (lava)
// 1850K: Candle (~0xFF8E13)
// 2700K: Incandescent lamp (warm yellow)
// 3400K: Halogen lamp
// 5500K: Daylight (white)
// 6500K: Cool white (studio light)
// 9000K: Bluish tint
// 15000-40000K: Blue sky
//
// The formulas are based on the empirical approximation by Tanner Helland for converting black body color temperature to RGB.

uint32_t temperature_to_rgb (float__t temp_kelvin)
{
 // Temperature should be within reasonable limits (800-40000 K)
 // Draper point is ~798K (visible glow starts)
 if (temp_kelvin < 780)
  {
   return 0x000000; // Black for too low temperatures
  }

 double temp = temp_kelvin / 100.0;
 double red, green, blue;

 // Red channel
 if (temp <= 66)
  {
   red = 255;
  }
 else
  {
   red = temp - 60;
   red = 329.698727446 * pow (red, -0.1332047592);
   if (red < 0) red = 0;
   if (red > 255) red = 255;
  }

 // Green channel
 if (temp <= 66)
  {
   green = temp;
   green = 99.4708025861 * log (green) - 161.1195681661;
   if (green < 0) green = 0;
   if (green > 255) green = 255;
  }
 else
  {
   green = temp - 60;
   green = 288.1221695283 * pow (green, -0.0755148492);
   if (green < 0) green = 0;
   if (green > 255) green = 255;
  }

 // Blue channel
 if (temp >= 66)
  {
   blue = 255;
  }
 else if (temp <= 19)
  {
   blue = 0;
  }
 else
  {
   blue = temp - 10;
   blue = 138.5177312231 * log (blue) - 305.0447927307;
   if (blue < 0) blue = 0;
   if (blue > 255) blue = 255;
  }

 // Dimming for the 800K - 1000K range, so the color appears smoothly
 if (temp_kelvin < 1000)
  {
   double dimming = (temp_kelvin - 800.0) / 200.0;
   if (dimming < 0) dimming = 0;
   if (dimming > 1) dimming = 1;

   red *= dimming;
   green *= dimming;
   blue *= dimming;
  }

 return (uint32_t)(blue) + 256 * (uint32_t)(green) + 65536 * (uint32_t)(red);
}

// Convert decibels to a linear scale for complex numbers: adb(z) = 10^(z/10)
void AdbC (float__t re_in, float__t im_in, float__t &re_out, float__t &im_out)
{
 // adb(z) = 10^(z/10)
 float__t tmp_re = re_in / (float__t)10.0L;
 float__t tmp_im = im_in / (float__t)10.0L;

 Exp10C (tmp_re, tmp_im, re_out, im_out);
}

// Convert nepers to a linear scale for complex numbers: anp(z) = exp(z)
void AnpC (float__t re_in, float__t im_in, float__t &re_out, float__t &im_out)
{
 float__t tmp_re = re_in / (float__t)20.0L;
 float__t tmp_im = im_in / (float__t)20.0L;

 Exp10C (tmp_re, tmp_im, re_out, im_out);
}


// Sine of a complex number: sin(z) = sin(x + iy) = sin(x) * cosh(y) + i * cos(x) * sinh(y)
void SinC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 re = sinl (x) * coshl (y);
 im = cosl (x) * sinhl (y);
#else
 re = sin (x) * cosh (y);
 im = cos (x) * sinh (y);
#endif
}

// Cosine of a complex number: cos(z) = cos(x) * cosh(y) - i * sin(x) * sinh(y)
void CosC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 re = cosl (x) * coshl (y);
 im = -sinl (x) * sinhl (y);
#else
 re = cos (x) * cosh (y);
 im = -sin (x) * sinh (y);
#endif
}

// Exponential of a complex number: exp(z) = exp(x) * (cos(y) + i * sin(y))
void ExpC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 float__t ex = expl (x);
 re = ex * cosl (y);
 im = ex * sinl (y);
#else
 double ex = exp (x);
 re = ex * cos (y);
 im = ex * sin (y);
#endif
}

// Exponential of a complex number with base 10: exp10(z) = 10^z = exp(z * ln(10))
void Exp10C(float__t x, float__t y, float__t& re, float__t& im)
{
 #ifdef _long_double_
 float__t ex = powl ((float__t)10.0L, x);
 re = ex * cosl (y);
 im = ex * sinl (y);
#else
 double ex = pow (10.0, x);
 re = ex * cos (y);
 im = ex * sin (y);
#endif
}


// Absolute value (magnitude) of a complex number: abs(z) = sqrt(x^2 + y^2)
float__t AbsC (float__t x, float__t y)
{
#ifdef _long_double_
 return hypotl (x, y);
#else
 return hypot (x, y);
#endif
}

// Round the real and imaginary parts of a complex number to the nearest integer
void RoundC(float__t x, float__t y, float__t& re, float__t& im)
{
 re = Round (x);
 im = Round (y);
}
// Round the real and imaginary parts of a complex number up to the nearest integer
void CeilC(float__t x, float__t y, float__t& re, float__t& im)
{
 re = Ceil (x);
 im = Ceil (y);
}
// Round the real and imaginary parts of a complex number down to the nearest integer
void FloorC(float__t x, float__t y, float__t& re, float__t& im)
{
 re = Floor (x);
 im = Floor (y);
}
// Get the fractional part of the real and imaginary parts of a complex number
void FracC(float__t x, float__t y, float__t& re, float__t& im)
{
 re = Frac (x);
 im = Frac (y);
}
// Get the integer part of the real and imaginary parts of a complex number
void IntC (float__t x, float__t y, float__t &re, float__t &im)
{
 re = Int (x);
 im = Int (y);
}
// Get the real and imaginary parts of a complex number as floats (essentially a no-op, but included
// for completeness)
void FloatC (float__t x, float__t y, float__t &re, float__t &im)
{
 re = Float (x);
 im = Float (y);
}


// Tangent of a complex number: tan(z) = sin(z) / cos(z)
void TanC (float__t x, float__t y, float__t &re, float__t &im)
{
 float__t sin_re, sin_im, cos_re, cos_im;
 SinC (x, y, sin_re, sin_im);
 CosC (x, y, cos_re, cos_im);
 // (a+bi)/(c+di) = [(ac+bd) + i(bc-ad)] / (c^2 + d^2)
 float__t denom = cos_re * cos_re + cos_im * cos_im;
 if (denom == (float__t)0.0L)
  {
   re = qnan; 
   im = qnan; 
   return;
  }
 re = (sin_re * cos_re + sin_im * cos_im) / denom;
 im = (sin_im * cos_re - sin_re * cos_im) / denom;
}

// Cotangent of a complex number: cot(z) = 1 / tan(z)
void CotC (float__t x, float__t y, float__t &re, float__t &im)
{
 float__t tan_re, tan_im;
 TanC (x, y, tan_re, tan_im);
 float__t denom = tan_re * tan_re + tan_im * tan_im;
 if (denom == (float__t)0.0L)
  {
   re = qnan; 
   im = qnan; 
   return;
  }
 re = tan_re / denom;
 im = -tan_im / denom;
}

// Arcsine of a complex number: arcsin(z) = -i * ln(iz + sqrt(1 - z^2))
void AsinC (float__t x, float__t y, float__t &re, float__t &im)
{
 // iz = -y + ix
 float__t a = (float__t)1.0L - (x * x - y * y);
 float__t b = -(float__t)2.0L * x * y;
 float__t sqrt_re, sqrt_im;
 // sqrt(1 - z^2)
 SqrtC (a, b, sqrt_re, sqrt_im);
 // iz + sqrt(...)
 float__t s_re = -y + sqrt_re;
 float__t s_im = x + sqrt_im;
 // ln(iz + sqrt(...))
 float__t ln_re, ln_im;
 LnC (s_re, s_im, ln_re, ln_im);
 re = ln_im;
 im = -ln_re;
}

// Arccosine of a complex number: arccos(z) = -i * ln(z + sqrt(z^2 - 1))
void AcosC (float__t x, float__t y, float__t &re, float__t &im)
{
 // z^2 - 1
 float__t a = x * x - y * y - (float__t)1.0L;
 float__t b = (float__t)2.0L * x * y;
 float__t sqrt_re, sqrt_im;
 SqrtC (a, b, sqrt_re, sqrt_im);
 // z + sqrt(...)
 float__t s_re = x + sqrt_re;
 float__t s_im = y + sqrt_im;
 // ln(z + sqrt(...))
 float__t ln_re, ln_im;
 LnC (s_re, s_im, ln_re, ln_im);
 re = ln_im;
 im = -ln_re;
}

// Arctangent of a complex number: arctan(z) = (i/2) * [ln(1 - iz) - ln(1 + iz)]
void AtanC (float__t x, float__t y, float__t &re, float__t &im)
{
 // 1 - iz = 1 + y - ix
 float__t a1 = 1 + y;
 float__t b1 = -x;
 float__t ln1_re, ln1_im;
 LnC (a1, b1, ln1_re, ln1_im);

 // 1 + iz = 1 - y + ix
 float__t a2 = 1 - y;
 float__t b2 = x;
 float__t ln2_re, ln2_im;
 LnC (a2, b2, ln2_re, ln2_im);

 re = (float__t)0.5L * (ln1_im - ln2_im);
 im = (float__t)0.5L * (ln2_re - ln1_re);
}

// Hyperbolic sine of a complex number: sinh(z) = sinh(x) * cos(y) + i * cosh(x) * sin(y)
void SinhC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 re = sinhl (x) * cosl (y);
 im = coshl (x) * sinl (y);
#else
 re = sinh (x) * cos (y);
 im = cosh (x) * sin (y);
#endif
}

// Hyperbolic cosine of a complex number: cosh(z) = cosh(x) * cos(y) + i * sinh(x) * sin(y)
void CoshC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 re = coshl (x) * cosl (y);
 im = sinhl (x) * sinl (y);
#else
 re = cosh (x) * cos (y);
 im = sinh (x) * sin (y);
#endif
}

// Hyperbolic tangent of a complex number: tanh(z) = sinh(z) / cosh(z)
void TanhC (float__t x, float__t y, float__t &re, float__t &im)
{
 float__t sinh_re, sinh_im, cosh_re, cosh_im;
 SinhC (x, y, sinh_re, sinh_im);
 CoshC (x, y, cosh_re, cosh_im);
 float__t denom = cosh_re * cosh_re + cosh_im * cosh_im;
 if (denom == (float__t)0.0L)
  {
   re = qnan; 
   im = qnan; 
   return;
  }
 re = (sinh_re * cosh_re + sinh_im * cosh_im) / denom;
 im = (sinh_im * cosh_re - sinh_re * cosh_im) / denom;
}

// Natural logarithm of a complex number: ln(z) = ln|z| + i*arg(z)
void LnC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef __BORLANDC__
 if ((x == (float__t)0.0L) && (y == (float__t)0.0L))
 {
  re = (float__t)(-1.0L/0.0L);
  im = 0;
  return;
 }
#else
 if ((x == (float__t)0.0L) && (y == (float__t)0.0L))
  {
   re = -inf;
   im = 0;
   return;
  }
 #endif
#ifdef _long_double_
 re = (float__t)0.5L * logl (x * x + y * y);
 //im = atan2l (y, x);
 im = (y == (float__t)0.0L && x < (float__t)0.0L) ? M_PI : atan2l (y, x);
#else
 re = 0.5 * log (x * x + y * y);
 im = (y == 0.0 && x < 0.0) ? M_PI : atan2 (y, x);
#endif
}

// Logarithm base 10 of a complex number: log10(z) = ln(z) / ln(10)
void Log10C(float__t x, float__t y, float__t& re, float__t& im)
{
    float__t ln10 = (float__t)2.3025850929940456840179914546844L; // ln(10)
    float__t ln_re, ln_im;
    LnC(x, y, ln_re, ln_im);
    re = ln_re / ln10;
    im = ln_im / ln10;
}

// Logarithm base 2 of a complex number: log2(z) = ln(z) / ln(2)
void Log2C (float__t x, float__t y, float__t &re, float__t &im)
{
    float__t ln2 = (float__t)0.69314718055994530941723212145818L; // ln(2)
    float__t ln_re, ln_im;
    LnC(x, y, ln_re, ln_im);
    re = ln_re / ln2;
    im = ln_im / ln2;
}


// Square root of a complex number: sqrt(z) = sqrt(r) * [cos(phi/2) + i*sin(phi/2)]
void SqrtC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 float__t r   = hypotl (x, y);
 //float__t phi = atan2l (y, x);
 float__t phi = (y == (float__t)0.0L && x < (float__t)0.0L) ? M_PI : atan2l (y, x);
 r = sqrtl (r);
 re = r * cosl (phi / (float__t)2.0L);
 im = r * sinl (phi / (float__t)2.0L);
#else
 double r = hypot (x, y);
 double phi = atan2 (y, x);
 r = sqrt (r);
 re = r * cos (phi / (float__t)2.0L);
 im = r * sin (phi / (float__t)2.0L);
#endif
}

// Cube root of a complex number: cbrt(z) = cbrt(r) * [cos(phi/3) + i*sin(phi/3)]
void Root3C(float__t x, float__t y, float__t& re, float__t& im)
{
#ifdef _long_double_
    float__t r = hypotl(x, y);
    float__t phi = (y == (float__t)0.0L && x < (float__t)0.0L) ? M_PI : atan2l(y, x);
   #ifdef __BORLANDC__
    r = Root3(r);
   #else
    r = cbrtl(r);
   #endif
    re = r * cosl(phi / (float__t)3.0L);
    im = r * sinl (phi / (float__t)3.0L);
#else
    double r = hypot(x, y);
    double phi = (y == 0.0 && x < 0.0) ? M_PI : atan2(y, x);
   #ifdef __BORLANDC__
    r = Root3(r);
   #else
    r = cbrt(r);
   #endif
    re = r * cos(phi / (float__t)3.0L);
    im = r * sin (phi / (float__t)3.0L);
#endif
}

// Arccotangent of a complex number: acot(z) = arctan(1/z)
void AcotC (float__t x, float__t y, float__t &re, float__t &im)
{
 // 1/z = (x - iy) / (x^2 + y^2)
 float__t denom = x * x + y * y;
 float__t zx = x / denom;
 float__t zy = -y / denom;
 AtanC (zx, zy, re, im);
}

// Hyperbolic cotangent of a complex number: coth(z) = 1 / tanh(z)
void CothC (float__t x, float__t y, float__t &re, float__t &im)
{
 float__t tanh_re, tanh_im;
 TanhC (x, y, tanh_re, tanh_im);
 float__t denom = tanh_re * tanh_re + tanh_im * tanh_im;
 if (denom == (float__t)0.0L)
  {
   re = qnan; 
   im = qnan; 
   return;
  }
 re = tanh_re / denom;
 im = -tanh_im / denom;
}

// Hyperbolic arcsine of a complex number: asinh(z) = ln(z + sqrt(z^2 + 1))
void AsinhC (float__t x, float__t y, float__t &re, float__t &im)
{
 // z^2 + 1
 float__t a = x * x - y * y + (float__t)1.0L;
 float__t b = (float__t)2.0L * x * y;
 float__t sqrt_re, sqrt_im;
 SqrtC (a, b, sqrt_re, sqrt_im);
 // z + sqrt(...)
 float__t s_re = x + sqrt_re;
 float__t s_im = y + sqrt_im;
 LnC (s_re, s_im, re, im);
}

// Hyperbolic arccosine of a complex number: acosh(z) = ln(z + sqrt(z + 1) * sqrt(z - 1))
void AcoshC (float__t x, float__t y, float__t &re, float__t &im)
{
 // sqrt(z + 1)
 float__t sqrt1_re, sqrt1_im;
 SqrtC (x + (float__t)1.0L, y, sqrt1_re, sqrt1_im);
 // sqrt(z - 1)
 float__t sqrt2_re, sqrt2_im;
 SqrtC (x - (float__t)1.0L, y, sqrt2_re, sqrt2_im);
 // sqrt(z+1) * sqrt(z-1)
 float__t mul_re = sqrt1_re * sqrt2_re - sqrt1_im * sqrt2_im;
 float__t mul_im = sqrt1_re * sqrt2_im + sqrt1_im * sqrt2_re;
 // z + mul
 float__t s_re = x + mul_re;
 float__t s_im = y + mul_im;
 LnC (s_re, s_im, re, im);
}

// Hyperbolic arctangent of a complex number: atanh(z) = 0.5 * [ln(1 + z) - ln(1 - z)]
void AtanhC (float__t x, float__t y, float__t &re, float__t &im)
{
 // 1 + z
 float__t a1 = (float__t)1.0L + x;
 float__t b1 = y;
 float__t ln1_re, ln1_im;
 LnC (a1, b1, ln1_re, ln1_im);

 // 1 - z
 float__t a2 = (float__t)1.0L - x;
 float__t b2 = -(float__t)1.0L * y;
 float__t ln2_re, ln2_im;
 LnC (a2, b2, ln2_re, ln2_im);

 re = (float__t)0.5L * (ln1_re - ln2_re);
 im = (float__t)0.5L * (ln1_im - ln2_im);
}

// Hyperbolic arccotangent of a complex number: acoth(z) = 0.5 * [ln(1 + 1/z) - ln(1 - 1/z)]
void AcothC (float__t x, float__t y, float__t &re, float__t &im)
{
 // 1/z = (x - iy) / (x^2 + y^2)
 float__t denom  = x * x + y * y;
 float__t inv_re = x / denom;
 float__t inv_im = -y / denom;

 // 1 + 1/z
 float__t a1 = (float__t)1.0L + inv_re;
 float__t b1 = inv_im;
 float__t ln1_re, ln1_im;
 LnC (a1, b1, ln1_re, ln1_im);

 // 1 - 1/z
 float__t a2 = (float__t)1.0L - inv_re;
 float__t b2 = -(float__t)1.0L * inv_im;
 float__t ln2_re, ln2_im;
 LnC (a2, b2, ln2_re, ln2_im);

 re = (float__t)0.5L * (ln1_re - ln2_re);
 im = (float__t)0.5L * (ln1_im - ln2_im);
}

// Complex exponentiation of a complex number: PowC(z1, z2) = exp(z2 * ln(z1))
void PowC (float__t x1, float__t y1, float__t x2, float__t y2, float__t &re, float__t &im)
{
 // ln(z1)
 float__t ln_re, ln_im;
 LnC (x1, y1, ln_re, ln_im);

 // z2 * ln(z1)
 float__t a = x2 * ln_re - y2 * ln_im;
 float__t b = x2 * ln_im + y2 * ln_re;

 // exp(z2 * ln(z1))
 ExpC (a, b, re, im);
}

// Extraction of the y-th root of a complex number: RootNC(xr, xi, yr, yi, re, im) = (xr + i*xi)^(1/(yr + i*yi))
// RootNC(xr, xi, yr, yi, re, im) = (xr + i*xi)^(1/(yr + i*yi))
void RootNC (float__t xr, float__t xi, float__t yr, float__t yi, float__t &re, float__t &im)
{
 // 1/(yr + i*yi)
 float__t denom = yr * yr + yi * yi;
 if (denom == (float__t)0.0L)
  {
   re = qnan; 
   im = qnan; 
   return;
  }
 float__t pow_re = yr / denom;
 float__t pow_im = -yi / denom;

 // (xr + i*xi)^(1/(yr + i*yi)) = PowC(xr, xi, pow_re, pow_im)
 PowC (xr, xi, pow_re, pow_im, re, im);
}

// Complex logarithm with arbitrary base: LognC(x, y, u, v) = ln(y + iv) / ln(x + iu)
void LognC (float__t x, float__t y, float__t u, float__t v, float__t &re, float__t &im)
{
 // ln(y + iv)
 float__t ln_num_re, ln_num_im;
 LnC (u, v, ln_num_re, ln_num_im);

 // ln(x + iy)
 float__t ln_den_re, ln_den_im;
 LnC (x, y, ln_den_re, ln_den_im);

 // (a + bi) / (c + di) = [(ac + bd) + i(bc - ad)] / (c^2 + d^2)
 float__t denom = ln_den_re * ln_den_re + ln_den_im * ln_den_im;
 if (denom == (float__t)0.0L)
  {
   re = qnan; 
   im = qnan; 
   return;
  }
 re = (ln_num_re * ln_den_re + ln_num_im * ln_den_im) / denom;
 im = (ln_num_im * ln_den_re - ln_num_re * ln_den_im) / denom;
}

// Coefficients of the Lanczos approximation for g=15, N=15 (precision for 128-bit)
// Note: in GCC, use the Q suffix for __float128 (e.g., 1.0Q)

#ifdef _float128_
#define A_VAL   24.0Q // Параметр точности для 128 бит
#define N_COUNT 24
#define SQRT_2PI 2.50662827463100050241576528481104525Q
#else
#define A_VAL   12.0L // Для 80-bit BCB
#define N_COUNT 12
#define SQRT_2PI 2.50662827463100050241576528481104525L
#endif

void GammaC (float__t zr, float__t zi, float__t &re, float__t &im)
{
 // 1. Reflection formula for Re(z) < 0.5: Gamma(z) = PI / (sin(PI*z) * Gamma(1-z))
 if (zr < 0.5L)
  {
   float__t szr, szi, gzr, gzi;
   SinC (M_PI * zr, M_PI * zi, szr, szi);
   GammaC (1.0L - zr, -zi, gzr, gzi);

   float__t denr   = szr * gzr - szi * gzi;
   float__t deni   = szr * gzi + szi * gzr;
   float__t d_abs2 = denr * denr + deni * deni;

   re = (M_PI * denr) / d_abs2;
   im = (-M_PI * deni) / d_abs2;
   return;
  }

 // 2. Spouge series
 float__t z = zr - (float__t)1.0L;
 // Constant c0 in Spouge's method is sqrt(2*PI)
 float__t x_re = SQRT_2PI;
 float__t x_im = (float__t)0.0L;

 for (int k = 1; k < N_COUNT; k++)
  {
   float__t k_f = (float__t)k;

   // Coefficient ck = ( (A-k)^(k-0.5) * exp(A-k) ) / (k-1)!
   // The sign alternates: c1(+), c2(-), c3(+)...
   float__t ck = Pow (A_VAL - k_f, k_f - 0.5L) * Exp (A_VAL - k_f);

   if (k > 1)
    {
     // Using your Factorial function for integers
     ck /= Factorial (k - 1);
    }

   if (k % 2 == 0) ck = -ck;

   float__t dzr = z + k_f;
   float__t den = dzr * dzr + zi * zi;

   x_re += ck * dzr / den;
   x_im -= ck * zi / den;
  }

 // 3. Main part of the formula: (z+A)^(z+0.5) * exp(-(z+A))
 float__t tr = z + A_VAL;
 float__t ppr, ppi, epr, epi;

 // Using your complex PowC and ExpC
 PowC (tr, zi, z + 0.5L, zi, ppr, ppi);
 ExpC (-tr, -zi, epr, epi);

 // Final multiplication (ppr+ppi*i) * (epr+epi*i) * (x_re+x_im*i)
 float__t tr1 = ppr * epr - ppi * epi;
 float__t ti1 = ppr * epi + ppi * epr;

 re = tr1 * x_re - ti1 * x_im;
 im = tr1 * x_im + ti1 * x_re;
}

// Factorial n! = Gamma(n + 1)
void FactorialC (float__t nr, float__t ni, float__t &re, float__t &im)
{
 if (ni == (float__t)0.0L && nr < (float__t)0.0L && nr == Floor (nr))
  {
   re = qnan;
   im = (float__t)0.0L;
   return; // Factorial is not defined for negative integers
  }
 GammaC (nr + (float__t)1.0L, ni, re, im);
}


void PolarC (float__t re1, float__t im1, float__t re2, float__t im2, float__t &out_re,
             float__t &out_im)
{
 // 1. Calculate the argument for the exponent: i * y
 // i * (re2 + i*im2) = -im2 + i*re2
 float__t exp_arg_re = -im2;
 float__t exp_arg_im = re2;

 // 2. Calculate the complex exponent e^(i*y)
 float__t e_re, e_im;
 ExpC (exp_arg_re, exp_arg_im, e_re, e_im);

 // 3. Multiply the result by the complex "radius" x: (re1 + i*im1) * (e_re + i*e_im)
 out_re = re1 * e_re - im1 * e_im;
 out_im = re1 * e_im + im1 * e_re;
}

// Square root of the sum of squares of two complex numbers: sqrt(z1^2 + z2^2)
void HypotC (float__t re1, float__t im1, float__t re2, float__t im2, float__t &out_re,
             float__t &out_im)
{
 // 1. Calculate z1^2 = (re1 + i*im1)^2
 float__t z1sq_re = re1 * re1 - im1 * im1;
 float__t z1sq_im = (float__t)2.0L * re1 * im1;

 // 2. Calculate z2^2 = (re2 + i*im2)^2
 float__t z2sq_re = re2 * re2 - im2 * im2;
 float__t z2sq_im = (float__t)2.0L * re2 * im2;
 // 3. Calculate the sum: z1^2 + z2^2
 float__t sum_re = z1sq_re + z2sq_re;
 float__t sum_im = z1sq_im + z2sq_im;

 // 4. Calculate the square root: sqrt(z1^2 + z2^2)
 SqrtC (sum_re, sum_im, out_re, out_im);
}

void Atan2C (float__t re1, float__t im1, float__t re2, float__t im2, float__t &out_re,
             float__t &out_im)
{
 // atan2(z1, z2) = atan(z1/z2)
 float__t denom_re = re2 * re2 + im2 * im2;

 if (denom_re == (float__t)0.0L)
  {
   // If both numbers are zero — this is the classic NaN for atan2
   if (re1 == 0.0L && im1 == 0.0L)
    {
     out_re = qnan;
     out_im = qnan;
    }
   else if (re1 > (float__t)0.0L)
    {
     out_re = M_PI_2l; //M_PI / (float__t)2.0L; // PI / 2
     out_im = (float__t)0.0L;
    }
   else
    {
     out_re = -M_PI_2l;//-(M_PI/(float__t)2.0L); // -PI / 2
     out_im = (float__t)0.0L;
    }
   return;
  }

 // complex division (z1 / z2)
 float__t z_re = (re1 * re2 + im1 * im2) / denom_re;
 float__t z_im = (im1 * re2 - re1 * im2) / denom_re;

 // Calculate the base arctangent
 AtanC (z_re, z_im, out_re, out_im);

 // WARNING: Quadrant correction based on real parts!
 // This behavior mimics the classic real atan2
 if (re2 < (float__t)0.0L) // x < 0
  {
   if (re1 >= (float__t)0.0L)
    {
     out_re += M_PI; // Shift up for the 2nd quadrant
    }
   else
    {
     out_re -= M_PI; // Shift down for the 3rd quadrant
    }
  }
}

void Atan2C1(float__t re1, float__t im1, float__t re2, float__t im2, float__t& out_re,
    float__t& out_im)
{
 // atan2(z1, z2) = atan(z1/z2)
 float__t denom_re = re2 * re2 + im2 * im2;
 if (denom_re == (float__t)0.0L)
  {
   out_re = qnan; 
   out_im = qnan; 
   return;
  }
 float__t z_re = (re1 * re2 + im1 * im2) / denom_re;
 float__t z_im = (im1 * re2 - re1 * im2) / denom_re;
 AtanC (z_re, z_im, out_re, out_im);
}

// Complex modulus (remainder) of two complex numbers: z1 mod z2 = z1 - n * z2, where n = trunc(z1 /
// z2)
void FmodC (float__t x1, float__t y1, float__t x2, float__t y2, float__t &re, float__t &im)
{
 // z1 = x1 + i*y1, z2 = x2 + i*y2
 // Find the quotient n = z1 / z2
 float__t denom = x2 * x2 + y2 * y2;

 if (denom == 0.0L)
  {
   re = qnan;
   im = qnan;
   return;
  }

 // Real and imaginary parts of the exact quotient
 float__t quot_re = (x1 * x2 + y1 * y2) / denom;
 float__t quot_im = (y1 * x2 - x1 * y2) / denom;

 // Round the parts of the quotient towards zero (analog of trunc() for fmod)
 // You can use your int() or floor() with sign check
 float__t n_re = (quot_re >= 0) ? floorl (quot_re) : ceill (quot_re);
 float__t n_im = (quot_im >= 0) ? floorl (quot_im) : ceill (quot_im);

 // Remainder r = z1 - n * z2
 // n * z2:
 float__t prod_re = n_re * x2 - n_im * y2;
 float__t prod_im = n_re * y2 + n_im * x2;

 // Final result
 re = x1 - prod_re;
 im = y1 - prod_im;
}

// Complex division: z1 / z2 = (z1 * conj(z2)) / |z2|^2
void FdivC (float__t re1, float__t im1, float__t re2, float__t im2, float__t &out_re, float__t &out_im)
{
 float__t denom = re2 * re2 + im2 * im2;
 if (denom == (float__t)0.0L)
  {
   out_re = qnan; 
   out_im = qnan; 
   return;
  }
 out_re = (re1 * re2 + im1 * im2) / denom;
 out_im = (im1 * re2 - re1 * im2) / denom;
}

// Complex multiplication: (re1 + i*im1) * (re2 + i*im2) = (re1*re2 - im1*im2) + i*(re1*im2 +
// im1*re2)
void FmulC (float__t re1, float__t im1, float__t re2, float__t im2, float__t &out_re,
            float__t &out_im)
{
 out_re = re1 * re2 - im1 * im2;
 out_im = re1 * im2 + im1 * re2;
}

bool is_complex2 (value *arg1, value *arg2, int idx)
{
 if (((arg1->tag == tvCOMPLEX) 
     || (arg2->tag == tvCOMPLEX)) 
     || ((arg1->imval != (float__t)0.0L) 
     || (arg2->imval != (float__t)0.0L))) return true;
 switch (idx)
  {
  case vf_pow:
  case vf_rootn:
   {
    if (arg2->fval == (float__t)arg2->ival)
     return false; // If the exponent is an integer, we can use real exponentiation for real bases
    if (arg1->fval >= (float__t)0.0L)
     return false; // if the base is non-negative, we can use real exponentiation for real bases
    return true;   // Otherwise, we need complex exponentiation
   }
   break;
   case vf_logn:
   {
    if (arg1->fval > (float__t)0.0L) return false; // For positive values — regular log
    return true;                       // For negative values and ZERO — complex log
                                       // (although log(0) will throw an error in both cases)
   }
   break;
  }
 return false;
}

// This function performs various vectorized operations on complex numbers based on the index
// provided.
void vfunc2 (value *res, value *arg1, value *arg2, int idx)
{
 if (res == nullptr || arg1 == nullptr || arg2 == nullptr) return;
 if (is_complex2 (arg1, arg2, idx) || (res->tag == tvCOMPLEX) || (res->imval != (float__t)0.0L))
  {
   float__t out_re = (float__t)0.0L;
   float__t out_im = (float__t)0.0L;
   float__t re1 = arg1->get ();
   float__t im1 = arg1->imval;
   float__t re2 = arg2->get ();
   float__t im2 = arg2->imval;
   switch (idx)
    {
    case vf_pow:
     {
      PowC (re1, im1, re2, im2, out_re, out_im);
     }
     break;
    case vf_rootn:
     {
      RootNC (re1, im1, re2, im2, out_re, out_im);
     }
     break;
    case vf_logn:
     {
      LognC (re1, im1, re2, im2, out_re, out_im);
     }
     break;
    case vf_cplx:
     {
      out_re = re1;
      out_im = re2;
     }
     break;
    case vf_polar:
     {
      PolarC (re1, im1, re2, im2, out_re, out_im);
     }
     break;
    case vf_hypot:
    {
      HypotC (re1, im1, re2, im2, out_re, out_im);
    }
    break;
    case vf_atan2:
     {
      Atan2C (re1, im1, re2, im2, out_re, out_im);
     }
     break;
    case vf_fmod:
     {
      FmodC (re1, im1, re2, im2, out_re, out_im);
     }
     break;
    }
   res->fval  = out_re;
   res->imval = out_im;
   res->tag   = tvCOMPLEX;
   res->ival  = (int64_t)res->fval;
  }
 else
  {
   switch (idx)
    {
    case vf_pow:
     {
      res->fval = Pow (arg1->get (), arg2->get ());
     }
     break;
    case vf_rootn:
     {
      res->fval = Rootn (arg1->get (), arg2->get ());
     }
     break;
    case vf_logn:
     {
      res->fval = Logn (arg1->get (), arg2->get ());
     }
     break;
    case vf_cplx:
     {
      res->fval  = arg1->get ();
      res->imval = arg2->get ();
      res->tag   = tvCOMPLEX;
      res->ival  = (int64_t)res->fval;
      return;
     }
    case vf_polar:
     {

      res->fval  = arg1->get () * Cos (arg2->get ());
      res->imval = arg1->get () * Sin (arg2->get ());
      res->tag   = tvCOMPLEX;
      res->ival  = (int64_t)res->fval;
      return;
     }
    case vf_hypot:
     {
      res->fval = Hypot (arg1->get (), arg2->get ());
     }
     break;
    case vf_atan2:
     {
      res->fval = Atan2l (arg1->get (), arg2->get ());
     }
     break;

    }
   res->imval = (float__t)0.0L;
   res->tag   = tvFLOAT;
   res->ival  = (int64_t)res->fval;
  }
}

bool is_complex1 (value *arg, int idx)
{
 if (((arg->tag == tvCOMPLEX) || (arg->imval != (float__t)0.0L))) return true;
 switch (idx)
  {
  case vf_sqrt:
   {
    if (arg->fval >= (float__t)0.0L)
     return false; // If the input is non-negative, we can use real square root
    return true;                        // Otherwise, we need complex square root
   }
   break;
  case vf_acos:
  case vf_asin:
   {
    if (Abs (arg->fval) <= (float__t)1.0L) return false;
    return true;
   }
   break;
  case vf_log:
  case vf_log10:
  case vf_log2:
   {
    if (arg->fval > (float__t)0.0L) return false; // For positive values — regular log
    return true;                       // For negative values and ZERO — complex log
                                       // (although log(0) will throw an error in both cases)
   }
   break;
  }
  return false;
}

// This function performs various vectorized operations on complex numbers based on the index
void vfunc (value *res, value *arg, int idx)
{
 if (res == nullptr || arg == nullptr) return;
 if ((res->tag == tvCOMPLEX) || (res->imval != (float__t)0.0L)
     || is_complex1 (arg, idx))
  {
   float__t out_re = (float__t)0.0L;
   float__t out_im = (float__t)0.0L;
   float__t re     = arg->get ();
   float__t im     = arg->imval;
   switch (idx)
    {
    case vf_abs:
     {
      res->fval  = Hypot (re, im);
      res->tag   = tvFLOAT;
      res->imval = (float__t)0.0L;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_pol:
     {
      res->fval  = atan2l (im, re); // argument
      res->imval = (float__t)0.0L;
      res->tag   = tvFLOAT;
      res->ival  = (int64_t)res->fval;
     }
     return;
    //
    case vf_sin:
     {
      SinC (re, im, out_re, out_im);
     }
     break;
    case vf_cos:
     {
      CosC (re, im, out_re, out_im);
     }
     return;
    case vf_tan:
     {
      TanC (re, im, out_re, out_im);
     }
     break;
    case vf_cot:
     {
      CotC (re, im, out_re, out_im);
     }
     break;
    //
    case vf_sinh:
     {
      SinhC (re, im, out_re, out_im);
     }
     break;
    case vf_cosh:
     {
      CoshC (re, im, out_re, out_im);
     }
     break;
    case vf_tanh:
     {
      TanhC (re, im, out_re, out_im);
     }
     break;
    case vf_ctnh:
     {
      CothC (re, im, out_re, out_im);
     }
     break;
    //
    case vf_asin:
     {
      AsinC (re, im, out_re, out_im);
     }
     break;
    case vf_acos:
     {
      AcosC (re, im, out_re, out_im);
     }
     break;
    case vf_atan:
     {
      AtanC (re, im, out_re, out_im);
     }
     break;
    case vf_acot:
     {
      AcotC (re, im, out_re, out_im);
     }
     break;
    //
    case vf_asinh:
     {
      AsinhC (re, im, out_re, out_im);
     }
     break;
    case vf_acosh:
     {
      AcoshC (re, im, out_re, out_im);
     }
     break;
    case vf_atanh:
     {
      AtanhC (re, im, out_re, out_im);
     }
     break;
    case vf_acoth:
     {
      AcothC (re, im, out_re, out_im);
     }
     break;
    //
    case vf_exp:
     {
      ExpC (re, im, out_re, out_im);
     }
     break;
    case vf_exp10:
     {
      Exp10C (re, im, out_re, out_im);
     }
     break;
    case vf_log:
     {
      LnC (re, im, out_re, out_im);
     }
     break;
    case vf_log10:
     {
      Log10C (re, im, out_re, out_im);
     }
     break;
    case vf_log2:
     {
      Log2C (re, im, out_re, out_im);
     }
     break;
    case vf_sqrt:
     {
      SqrtC (re, im, out_re, out_im);
     }
     break;
    case vf_root3:
     {
      Root3C (re, im, out_re, out_im);
     }
     break;
    case vf_rnd:
     {
      res->fval  = Random (re);
      res->imval = Random (im);
      res->tag   = tvCOMPLEX;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_round:
     {
      RoundC (re, im, out_re, out_im);
     }
     break;
    case vf_ceil:
     {
      CeilC (re, im, out_re, out_im);
     }
     break;
    case vf_floor:
     {
      FloorC (re, im, out_re, out_im);
     }
     break;
    case vf_frac:
     {
      FracC (re, im, out_re, out_im);
     }
     break;
    case vf_int:
     {
      IntC (re, im, out_re, out_im);
     }
     break;
    case vf_float:
     {
      FloatC (re, im, out_re, out_im);
     }
     break;

    case vf_factorial:
     {
      FactorialC (re, im, out_re, out_im);
     }
     break;
    case vf_re:
     {
      res->fval  = re; // argument
      res->imval = (float__t)0.0L;
      res->tag   = tvFLOAT;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_im:
     {
      res->fval  = im; // argument
      res->imval = (float__t)0.0L;
      res->tag   = tvFLOAT;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_conj:
     {
      res->fval  = re;  // argument
      res->imval = -im; // conjugate
      res->tag   = tvCOMPLEX;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_db:
     {
       res->fval = DB (Hypot (re, im));
       res->imval = (float__t)0.0L;
       res->tag   = tvFLOAT;
       res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_np:
     {
       res->fval = NP (Hypot (re, im));
       res->imval = (float__t)0.0L;
       res->tag   = tvFLOAT;
       res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_adb:
     {
      AdbC (re, im, out_re, out_im);
     }
     break;
    case vf_anp:
     {
      AnpC (re, im, out_re, out_im);
     }
     break;
    }

   res->fval  = out_re;
   res->imval = out_im;
   res->tag   = tvCOMPLEX;
   res->ival  = (int64_t)res->fval;
  }
 else
  {
   switch (idx)
    {
    case vf_abs:
     {
      res->fval = Abs (arg->fval);
     }
     break;
    case vf_pol:
     {
      res->fval = (float__t)0.0L;
     }
     break;
    case vf_sin:
     {
      res->fval = Sin (arg->fval);
     }
     break;
    case vf_cos:
     {
      res->fval = Cos (arg->fval);
     }
     break;
    case vf_tan:
     {
      res->fval = Tan (arg->fval);
     }
     break;
    case vf_cot:
     {
      res->fval = Ctg (arg->fval);
     }
     break;
    //
    case vf_sinh:
     {
      res->fval = Sinh (arg->fval);
     }
     break;
    case vf_cosh:
     {
      res->fval = Cosh (arg->fval);
     }
     break;
    case vf_tanh:
     {
      res->fval = Tanh (arg->fval);
     }
     break;
    case vf_ctnh:
     {
      res->fval = Ctanh (arg->fval);
     }
     break;
    //
    case vf_asin:
     {
      res->fval = Asin (arg->fval);
     }
     break;
    case vf_acos:
     {
      res->fval = Acos (arg->fval);
     }
     break;
    case vf_atan:
     {
      res->fval = Atan (arg->fval);
     }
     break;
    case vf_acot:
     {
      res->fval = Acot (arg->fval);
     }
     break;
    //
    case vf_asinh:
     {
      res->fval = Arsh (arg->fval);
     }
     break;
    case vf_acosh:
     {
      res->fval = Arch (arg->fval);
     }
     break;
    case vf_atanh:
     {
      res->fval = Arth (arg->fval);
     }
     break;
    case vf_acoth:
     {
      res->fval = Arcth (arg->fval);
     }
     break;
    //
    case vf_exp:
     {
      res->fval = Exp (arg->fval);
     }
     break;
    case vf_exp10:
     {
      res->fval = Exp10 (arg->fval);
     }
     break;
    case vf_log:
     {
      res->fval = Log (arg->fval);
     }
     break;
    case vf_log10:
     {
      res->fval = Lg (arg->fval);
     }
     break;
    case vf_log2:
     {
      res->fval = Log2   (arg->fval);
     }
     break;
    case vf_sqrt:
     {
      res->fval = Sqrt (arg->fval);
     }
     break;
    case vf_root3:
     {
      res->fval = Root3 (arg->fval);
     }
     break;
    case vf_re:
     {
      res->fval = arg->fval;
     }
     break;
    case vf_im:
     {
      res->fval = (float__t)0.0L;
     }
     break;
    case vf_conj:
     {
      res->fval = arg->fval;
     }
     break;
    case vf_rnd:
     {
      res->fval = Random (arg->fval);
     }
     break;
    case vf_round:
     {
      res->fval = Round (arg->fval);
     }
     break;
    case vf_ceil:
     {
      res->fval = Ceil (arg->fval);
     }
     break;
    case vf_floor:
     {
      res->fval = Floor (arg->fval);
     }
     break;
    case vf_frac:
     {
      res->fval = Frac (arg->fval);
     }
     break;
    case vf_int:
     {
      res->fval = Int (arg->fval);
     }
     break;
    case vf_float:
     {
      res->fval = Float (arg->fval);
     }
     break;

     case vf_db:
     {
      res->fval = DB (arg->fval);
     }
     break;
     case vf_np:
     {
      res->fval = NP (arg->fval);
     }
     break;
     case vf_adb:
     {
      res->fval = ADB (arg->fval);
     }
     break;
     case vf_anp:
     {
      res->fval = ANP (arg->fval);
     }
     break;

    case vf_factorial:
     {
      res->fval = Factorial (arg->fval);
     }
     break;
    }
   res->tag   = tvFLOAT;
   res->imval = (float__t)0.0L;
   res->ival  = (int64_t)res->fval;
  }
}

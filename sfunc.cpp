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

#define M_PI_2l 1.5707963267948966192313216916398
#define nullptr NULL

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

#pragma warning(disable : 4996)
#pragma warning(disable : 4244)

#define M_PI    3.1415926535897932384626433832795
#define M_PI_2l 1.5707963267948966192313216916398
#define M_E     2.7182818284590452353602874713527
#endif

int_t To_int (int_t val)
{
 return val;
}

float__t To_float (float__t val)
{
 return val;
}

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

int_t Now (int_t n)
{
 return (int_t)time (nullptr) + n * 60 * 60;
}

float__t Erf (float__t x)
{
 // constants
 float__t a1 = 0.254829592;
 float__t a2 = -0.284496736;
 float__t a3 = 1.421413741;
 float__t a4 = -1.453152027;
 float__t a5 = 1.061405429;
 float__t p  = 0.3275911;

 // Save the sign of x
 int sign = (x < 0) ? -1 : 1;
 x        = fabsl (x);

 // A&S formula 7.1.26
 float__t t = 1.0 / (1.0 + p * x);
 float__t y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * expl (-x * x);

 return sign * y;
}

float__t Erfc (float__t x)
{
 return 1 - Erf (x);
}

float__t Random (float__t x)
{
 return rand () * x / RAND_MAX;
}

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
 if (x == 0) return M_PI_2l; // pi/2
 return atanl (1.0L / x);
#else
 if (x == 0) return M_PI_2; // pi/2
 return atan (1.0 / x);
#endif
}

float__t Atan2l (float__t x, float__t y)
{
 if (x != 0 && y != 0)
  {
#ifdef _long_double_
   return atan2l (x, y);
#else
   return atan2 (x, y);
#endif
  }
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0/0.0;
}

float__t Cos (float__t x)
{
#ifdef _long_double_
 return cosl (x);
#else
 return cos (x);
#endif
}

float__t Cosh (float__t x)
{
#ifdef _long_double_
 return coshl (x);
#else
 return cosh (x);
#endif
}

float__t Exp (float__t x)
{
#ifdef _long_double_
 return expl (x);
#else
 return exp (x);
#endif
}

float__t Sin (float__t x)
{
#ifdef _long_double_
 return sinl (x);
#else
 return sin (x);
#endif
}

float__t Sinh (float__t x)
{
#ifdef _long_double_
 return sinhl (x);
#else
 return sinh (x);
#endif
}

float__t Tan (float__t x)
{
#ifdef _long_double_
 return tanl (x);
#else
 return tan (x);
#endif
}

float__t Tanh (float__t x)
{
#ifdef _long_double_
 return tanhl (x);
#else
 return tanh (x);
#endif
}

float__t Pow (float__t x, float__t y)
{
 if ((x > 0) || ((x == 0) && (y > 0)) || ((x < 0) && (y - floor (y) == 0)))
  {
#ifdef _long_double_
   return powl (x, y); // x^y
#else
   return pow (x, y);
#endif
  }
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
}

float__t Sqrt (float__t x)
{
// return sqrtl(x);
#ifdef _long_double_
 if (x >= 0)
  return sqrtl (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0/0.0;
#else
 if (x >= 0)
  return sqrt (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t Hypot (float__t x, float__t y)
{
 return Sqrt (x * x + y * y);
}

float__t Acos (float__t x)
{
#ifdef _long_double_
 if ((x <= 1) && (x >= -1))
  return acosl (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0/0.0;
#else
 if ((x <= 1) && (x >= -1))
  return acos (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t Asin (float__t x)
{
#ifdef _long_double_
 if ((x <= 1) && (x >= -1))
  return asinl (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#else
 if ((x <= 1) && (x >= -1))
  return asin (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t Log (float__t x)
{
#ifdef _long_double_
 if (x > 0)
  return logl (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#else
 if (x > 0)
  return log (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t Lg (float__t x)
{
#ifdef _long_double_
 if (x > 0)
  return log10l (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#else
 if (x > 0)
  return log10 (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t NP (float__t x)
{
#ifdef _long_double_
 if (x > 0)
  return 20 * log10l (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#else
 if (x > 0)
  return 20 * log10 (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t DB (float__t x)
{
#ifdef _long_double_
 if (x > 0)
  return 10 * log10l (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#else
 if (x > 0)
  return 10 * log10 (x);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif
}

float__t Ctanh (float__t x) // cth
{
#ifdef _long_double_
 if (x > 0)
  return (expl (2 * x) + 1) / (expl (2 * x) - 1);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#else                                                 /*_long_double_*/
 if (x > 0)
  return (exp (2 * x) + 1) / (exp (2 * x) - 1);
 else
  return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#endif                                                /*_long_double_*/
}

float__t Arsh (float__t x)
{
#ifdef _long_double_
 return logl (x + sqrt (x * x + 1));
#else  /*_long_double_*/
 return log (x + sqrt (x * x + 1));
#endif /*_long_double_*/
}

float__t Arch (float__t x)
{
#ifdef _long_double_
 return logl (x + sqrtl (x * x - 1));
#else  /*_long_double_*/
 return log (x + sqrt (x * x - 1));
#endif /*_long_double_*/
}

float__t Arth (float__t x)
{
 if ((x == 1) || (x == -1)) return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#ifdef _long_double_
 return logl ((1 + x) / (1 - x)) / 2;
#else  /*_long_double_*/
 return log ((1 + x) / (1 - x)) / 2;
#endif /*_long_double_*/
}

float__t Arcth (float__t x)
{
 if ((x == 1) || (x == -1)) return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#ifdef _long_double_
 return logl ((x + 1) / (x - 1)) / 2;
#else  /*_long_double_*/
 return log ((x + 1) / (x - 1)) / 2;
#endif /*_long_double_*/
}

float__t Exp10 (float__t x)
{
#ifdef _long_double_
 return expl (x * logl (10.0));
#else  /*_long_double_*/
 return exp (x * log (10.0));
#endif /*_long_double_*/
}

float__t ANP (float__t x)
{
 return Exp10 (x / 20);
}

float__t ADB (float__t x)
{
 return Exp10 (x / 10);
}

float__t Sing (float__t x)
{
#ifdef _long_double_
 return sinl (M_PI * x / 180);
#else  /*_long_double_*/
 return sin (M_PI * x / 180);
#endif /*_long_double_*/
}

float__t Cosg (float__t x)
{
#ifdef _long_double_
 return cosl (M_PI * x / 180);
#else  /*_long_double_*/
 return cos (M_PI * x / 180);
#endif /*_long_double_*/
}

float__t Tgg (float__t x)
{
#ifdef _long_double_
 return tanl (M_PI * x / 180);
#else  /*_long_double_*/
 return tan (M_PI * x / 180);
#endif /*_long_double_*/
}

float__t Ctgg (float__t x)
{
 if (x == 0) return std::numeric_limits<float__t>::infinity (); // 1/0.0;
#ifdef _long_double_
 return 1 / tanl (M_PI * x / 180);
#else  /*_long_double_*/
 return 1 / tan (M_PI * x / 180);
#endif /*_long_double_*/
}

float__t Ctg (float__t x)
{
 if (x == 0) return std::numeric_limits<float__t>::infinity (); // 1/0.0;
#ifdef _long_double_
 return 1 / tanl (x);
#else  /*_long_double_*/
 return 1 / tan (x);
#endif /*_long_double_*/
}

float__t Round (float__t x)
{
 return (float__t)((int)(x + 0.5));
}

float__t Ceil (float__t x)
{
 return (float__t)((int)(x + 1));
}

float__t Floor (float__t x)
{
 return (float__t)((int)(x));
}

float__t Frac (float__t x)
{
 float__t d;
#ifdef _long_double_
 return modfl (x, &d);
#else
 return modf (x, &d);
#endif
}

float__t Min (float__t x, float__t y)
{
 return (x < y) ? x : y;
}

float__t Log2 (float__t x)
{
 if (x <= 0) return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#ifdef _long_double_
 return logl (x) / logl (2.0);
#else  /*_long_double_*/
 return log (x) / log (2.0);
#endif /*_long_double_*/
}

/*
float__t powl(float__t x, float__t y)
{
  return exp(y*log(x));
}
*/
float__t Logn (float__t x, float__t y)
{
 if (x <= 0) return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
 if (y <= 0) return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
#ifdef _long_double_
 return logl (y) / logl (x);
#else  /*_long_double_*/
 return log (y) / log (x);
#endif /*_long_double_*/
}

float__t Root3 (float__t x)
{
#ifdef _long_double_
 return powl (x, 1 / 3.0);
#else  /*_long_double_*/
 return pow (x, 1 / 3.0);
#endif /*_long_double_*/
}

float__t Rootn (float__t x, float__t y)
{
#ifdef _long_double_
 return powl (x, 1 / y);
#else  /*_long_double_*/
 return pow (x, 1 / y);
#endif /*_long_double_*/
}

/*SWG Table https://en.wikipedia.org/wiki/Standard_wire_gauge*/
double tswg[] = { 12.700, 11.786, 10.973, 10.160, 9.449,  8.839,  8.230,  7.620,  7.010,  6.401,
                  5.893,  5.385,  4.877,  4.470,  4.064,  3.658,  3.251,  2.946,  2.642,  2.337,
                  2.032,  1.829,  1.626,  1.422,  1.219,  1.016,  0.914,  0.813,  0.711,  0.610,
                  0.559,  0.5080, 0.4572, 0.4166, 0.3759, 0.3454, 0.3150, 0.2946, 0.2743, 0.2540,
                  0.2337, 0.2134, 0.1930, 0.1727, 0.1524, 0.1321, 0.1219, 0.1118, 0.1016, 0.0914,
                  0.0813, 0.0711, 0.0610, 0.0508, 0.0406, 0.0305, 0.0254 };

float__t Swg (float__t x) // SWG to mm
{
 int n = x;
 if ((n >= -6) && (n <= 50))
  return tswg[n + 6];
 else
  return 0;
}

float__t SSwg (float__t x) // SWG to mm^2
{
 return pow (Swg (x), 2) * M_PI / 4.0;
}

float__t Aswg (float__t x) // mm to SWG
{
 if ((x < 0.0254) || (x > 12.7)) return std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
 int n = 0;
 while (tswg[n] >= x) n++;
 if (n)
  {
   if (fabs (tswg[n] - x) < fabs (tswg[n - 1] - x))
    return n - 6;
   else
    return n - 7;
  }
 else
  return -6;
}

/* https://en.wikipedia.org/wiki/American_wire_gauge */
float__t Awg (float__t x) // AWG to mm
{
 return exp (2.1104 - 0.11594 * x);
}

float__t SAwg (float__t x) // AWG to mm^2
{
 return pow (exp (2.1104 - 0.11594 * x), 2) * M_PI / 4.0;
}

float__t Cs (float__t x) // diameter to mm^2
{
 return pow (x, 2) * M_PI / 4.0;
}

float__t Acs (float__t x) // mm^2 to diameter
{
 return sqrt (4.0 * x / M_PI);
}

float__t Aawg (float__t x) // mm to AWG
{
 return (2.1104 - log (x)) / 0.11594;
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
float__t Ee (float__t x, float__t y) // find standard value
{
 if (y)
  {
   int n;
   int N          = x;
   double delta_n = 10;
   double delta_n1;
   double V = fabs (y);

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
     int nn = log10 (V) * N + 0.5;
     if (nn > N) nn = N;
     switch (N)
      {
      case 3:
       if ((nn >= 0) && (nn < sizeof (E3)))
        V = E3[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      case 6:
       if ((nn >= 0) && (nn < sizeof (E6)))
        V = E6[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      case 12:
       if ((nn >= 0) && (nn < sizeof (E12)))
        V = E12[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      case 24:
       if ((nn >= 0) && (nn < sizeof (E24)))
        V = E24[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      case 48:
       if ((nn >= 0) && (nn < sizeof (E48)))
        V = E48[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      case 96:
       if ((nn >= 0) && (nn < sizeof (E96)))
        V = E96[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      case 192:
       if ((nn >= 0) && (nn < sizeof (E192)))
        V = E192[nn];
       else
        V = std::numeric_limits<float__t>::quiet_NaN ();
       break;
      default:
       V = std::numeric_limits<float__t>::quiet_NaN (); // 0.0 / 0.0;
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
    return -V;
   else
    return V;
  }
 else
  return 0;
}

float__t Max (float__t x, float__t y)
{
 return (x > y) ? x : y;
}

float__t Factorial (float__t x)
{
 int n        = (int)x;
 float__t res = 1.0;
 for (; n > 1; n--) res = res * (float__t)n;
 return res;
}

float__t Farenheit (float__t x)
{
 return (x - 32.0) * 5.0 / 9.0;
}

float__t Cmp (float__t x, float__t y, float__t prec)
{
 float__t rel_diff = fabs (x - y) / (x > y ? x : y);
 if (rel_diff < prec)
  return 0;
 else if (x < y)
  return -1;
 else
  return 1;
}

// Vref=Vout*Rl/(Rh+Rl)
// Vout=Vref*(Rh+Rl)/Rl
float__t Vout (float__t Vref, float__t Rh, float__t Rl)
{
 return Vref * (Rh + Rl) / Rl;
}

// int __CRTDECL _matherr(struct _exception *e)
//{
//   return 0;             /* error has been handled */
// }

bool IsNaN (const double fVal)
{
 return (((*(__int64 *)(&fVal) & 0x7FF0000000000000ull) == 0x7FF0000000000000ull)
         && ((*(__int64 *)(&fVal) & 0x000FFFFFFFFFFFFFull) != 0x0000000000000000ull));
}

bool IsNaNL (const long double ldVal)
{
 return IsNaN ((double)ldVal);
}

template <class T> T tmax (T x, T y)
{
 return (x > y) ? x : y;
}

template <class T> bool tisnan (T f)
{
 T _nan = (T)0.0 / (T)0.0;
 return 0 == memcmp ((void *)&f, (void *)&_nan, sizeof (T));
}

/*
bool IsNaNL(const long float__t ldVal)
{
 typedef union
 {
  long float__t ld;
  short w[5];
  struct Parts
   {
    __int64 frac;
    short exp;
   }parts;
 } *pextrec;

 short e = (*(pextrec)&ldVal).parts.exp;
 __int64 f = (*(pextrec)&ldVal).parts.frac;
 short ww[5];
 for(int i=0; i<5; i++) ww[i] = (*(pextrec)&ldVal).w[i];
 return IsNaN((float__t)ldVal);
 return (((*(pextrec)&ldVal).parts.exp & 0x7fff == 0x7fff) &&
         ((*(pextrec)&ldVal).parts.frac & 0x7FFFFFFFFFFFFFFFi64 != 0));
}
*/

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

int_t fprn (char *dest, char *sfmt, int args, value *v_stack)
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
 char param[16] = { 0 };
 int p          = 0;
 int i;
 int n = 0;
 bool flag;
 char *dst = dest;
 if (!sfmt) return 0;
 do
  {
   c    = '\0';
   i    = 0;
   flag = false;
   fmt  = tNone;
   do
    {
     cc = c;
     c = pfmt[i++] = *sfmt++;
     pfmt[i]       = '\0';
     if (flag)
      {
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
         if (p < sizeof (param)) param[p++] = c;
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
   while (c && (i < STRBUF));
   if (!c) sfmt--; // poit to '\0' ???
   if (n < args)
    {
     switch (fmt)
      {
      case tNone:
       dst += sprintf (dst, pfmt);
       break;
      case tPtr:
       strcpy (dst, pfmt);
       dst += strlen (pfmt);
       break;
      case tComp:
       {
        float__t cd = v_stack[n].get ();
        dst += fmtc (dst, pfmt);
        dst += b2scistr (dst, cd);
       }
       break;
      case tBin:
       {
        __int64 bi = v_stack[n].get_int ();
        dst += fmtc (dst, pfmt);
        dst += b2str (dst, pfmt, bi);
       }
       break;
      case tChar:
       {
        if (cc == 'l')
         {
          int ii = v_stack[n].get_int ();
          dst += fmtc (dst, pfmt);
          dst += wchr2str (dst, ii);
         }
        else
         {
          char ci = v_stack[n].get_int ();
          dst += sprintf (dst, pfmt, ci);
         }
       }
       break;
      case tSpc:
       {
        char ci = v_stack[n].get_int ();
        dst += fmtc (dst, pfmt);
        dst += chr2str (dst, ci);
       }
       break;
      case tSci:
       {
        double dd = v_stack[n].get ();
        dst += fmtc (dst, pfmt);
        dst += d2scistr (dst, dd);
       }
       break;
      case tFract:
       {
        int en    = 0;
        double dd = v_stack[n].get ();
        dst += fmtc (dst, pfmt);
        if (param[0]) en = atoi (param);
        dst += d2frcstr (dst, dd, en);
       }
       break;
      case tNrm:
       {
        double dd = v_stack[n].get ();
        dst += fmtc (dst, pfmt);
        dst += d2nrmstr (dst, dd);
       }
       break;
      case tTime:
       {
        __int64 bi = v_stack[n].get_int ();
        dst += fmtc (dst, pfmt);
        dst += t2str (dst, bi);
       }
       break;
      case tInt:
       {
        if (cc == 'l')
         {
          long li = v_stack[n].get_int ();
          dst += sprintf (dst, pfmt, li);
         }
        else if (cc == 'L')
         {
          __int64 Li = v_stack[n].get_int ();
          dst += sprintf (dst, pfmt, Li);
         }
        else if (cc == 'h')
         {
          short hi = v_stack[n].get_int ();
          dst += sprintf (dst, pfmt, hi);
         }
        else
         {
          int ii = v_stack[n].get_int ();
          dst += sprintf (dst, pfmt, ii);
         }
       }
       break;
      case tFloat:
       {
        if (cc == 'L')
         {
          long double Ld = v_stack[n].get ();
          dst += sprintf (dst, pfmt, Ld);
         }
        else
         {
          double dd = v_stack[n].get ();
          dst += sprintf (dst, pfmt, dd);
         }
       }
       break;
      case tDeg:
       {
        double dd = v_stack[n].get ();
        dst += fmtc (dst, pfmt);
        dst += dgr2str (dst, dd);
       }
       break;
      case tString:
       dst += sprintf (dst, pfmt, v_stack[n].sval);
       break;
      }
     n++;
    }
   // else dst += sprintf(dst, fmt);
  }
 while (*sfmt && (i < STRBUF) && (n < args));
 return dst - dest;
}

#ifdef __BORLANDC__
int timezone(void) //return seconds
{
    return -_timezone;
}
#else
int timezone (void) // return seconds
{
 TIME_ZONE_INFORMATION tzi;
 DWORD tzResult    = GetTimeZoneInformation (&tzi);
 long timezoneBias = tzi.Bias; // in minutes
 int tzHours       = timezoneBias * 60;
 return tzHours;
}
#endif
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
   result              = mktime (&breakdown) - timezone ();
  }
 return (int_t)result;
}

const char *wavelength_info (float__t wavelength_m)
{
 // wavelength_m - wavelength in meters

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
 double wavelength_nm = wavelength_m * 1e9;

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
 long double ex = expl (x);
 re             = ex * cosl (y);
 im             = ex * sinl (y);
#else
 double ex = exp (x);
 re        = ex * cos (y);
 im        = ex * sin (y);
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

// Tangent of a complex number: tan(z) = sin(z) / cos(z)
void TanC (float__t x, float__t y, float__t &re, float__t &im)
{
 float__t sin_re, sin_im, cos_re, cos_im;
 SinC (x, y, sin_re, sin_im);
 CosC (x, y, cos_re, cos_im);
 // (a+bi)/(c+di) = [(ac+bd) + i(bc-ad)] / (c^2 + d^2)
 float__t denom = cos_re * cos_re + cos_im * cos_im;
 if (denom == 0)
  {
   re = std::numeric_limits<float__t>::quiet_NaN ();
   im = std::numeric_limits<float__t>::quiet_NaN ();
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
 if (denom == 0)
  {
   re = std::numeric_limits<float__t>::quiet_NaN ();
   im = std::numeric_limits<float__t>::quiet_NaN ();
   return;
  }
 re = tan_re / denom;
 im = -tan_im / denom;
}

// Arcsine of a complex number: arcsin(z) = -i * ln(iz + sqrt(1 - z^2))
void AsinC (float__t x, float__t y, float__t &re, float__t &im)
{
 // iz = -y + ix
 float__t a = 1 - (x * x - y * y);
 float__t b = -2 * x * y;
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
 float__t a = x * x - y * y - 1;
 float__t b = 2 * x * y;
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

 re = 0.5 * (ln1_im - ln2_im);
 im = 0.5 * (ln2_re - ln1_re);
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
 if (denom == 0)
  {
   re = std::numeric_limits<float__t>::quiet_NaN ();
   im = std::numeric_limits<float__t>::quiet_NaN ();
   return;
  }
 re = (sinh_re * cosh_re + sinh_im * cosh_im) / denom;
 im = (sinh_im * cosh_re - sinh_re * cosh_im) / denom;
}

// Natural logarithm of a complex number: ln(z) = ln|z| + i*arg(z)
void LnC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 re = 0.5L * logl (x * x + y * y);
 im = atan2l (y, x);
#else
 re = 0.5 * log (x * x + y * y);
 im = atan2 (y, x);
#endif
}

// Square root of a complex number: sqrt(z) = sqrt(r) * [cos(phi/2) + i*sin(phi/2)]
void SqrtC (float__t x, float__t y, float__t &re, float__t &im)
{
#ifdef _long_double_
 long double r   = hypotl (x, y);
 long double phi = atan2l (y, x);
 r               = sqrtl (r);
 re              = r * cosl (phi / 2);
 im              = r * sinl (phi / 2);
#else
 double r   = hypot (x, y);
 double phi = atan2 (y, x);
 r          = sqrt (r);
 re         = r * cos (phi / 2);
 im         = r * sin (phi / 2);
#endif
}
// Arccotangent of a complex number: acot(z) = arctan(1/z)
void AcotC (float__t x, float__t y, float__t &re, float__t &im)
{
 // 1/z = (x - iy) / (x^2 + y^2)
 float__t denom = x * x + y * y;
 float__t zx    = x / denom;
 float__t zy    = -y / denom;
 AtanC (zx, zy, re, im);
}

// Hyperbolic cotangent of a complex number: coth(z) = 1 / tanh(z)
void CothC (float__t x, float__t y, float__t &re, float__t &im)
{
 float__t tanh_re, tanh_im;
 TanhC (x, y, tanh_re, tanh_im);
 float__t denom = tanh_re * tanh_re + tanh_im * tanh_im;
 if (denom == 0)
  {
   re = std::numeric_limits<float__t>::quiet_NaN ();
   im = std::numeric_limits<float__t>::quiet_NaN ();
   return;
  }
 re = tanh_re / denom;
 im = -tanh_im / denom;
}

// Hyperbolic arcsine of a complex number: asinh(z) = ln(z + sqrt(z^2 + 1))
void AsinhC (float__t x, float__t y, float__t &re, float__t &im)
{
 // z^2 + 1
 float__t a = x * x - y * y + 1;
 float__t b = 2 * x * y;
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
 SqrtC (x + 1, y, sqrt1_re, sqrt1_im);
 // sqrt(z - 1)
 float__t sqrt2_re, sqrt2_im;
 SqrtC (x - 1, y, sqrt2_re, sqrt2_im);
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
 float__t a1 = 1 + x;
 float__t b1 = y;
 float__t ln1_re, ln1_im;
 LnC (a1, b1, ln1_re, ln1_im);

 // 1 - z
 float__t a2 = 1 - x;
 float__t b2 = -y;
 float__t ln2_re, ln2_im;
 LnC (a2, b2, ln2_re, ln2_im);

 re = 0.5 * (ln1_re - ln2_re);
 im = 0.5 * (ln1_im - ln2_im);
}

// Hyperbolic arccotangent of a complex number: acoth(z) = 0.5 * [ln(1 + 1/z) - ln(1 - 1/z)]
void AcothC (float__t x, float__t y, float__t &re, float__t &im)
{
 // 1/z = (x - iy) / (x^2 + y^2)
 float__t denom  = x * x + y * y;
 float__t inv_re = x / denom;
 float__t inv_im = -y / denom;

 // 1 + 1/z
 float__t a1 = 1 + inv_re;
 float__t b1 = inv_im;
 float__t ln1_re, ln1_im;
 LnC (a1, b1, ln1_re, ln1_im);

 // 1 - 1/z
 float__t a2 = 1 - inv_re;
 float__t b2 = -inv_im;
 float__t ln2_re, ln2_im;
 LnC (a2, b2, ln2_re, ln2_im);

 re = 0.5 * (ln1_re - ln2_re);
 im = 0.5 * (ln1_im - ln2_im);
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
 if (denom == 0)
  {
   re = std::numeric_limits<float__t>::quiet_NaN ();
   im = std::numeric_limits<float__t>::quiet_NaN ();
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
 if (denom == 0)
  {
   re = std::numeric_limits<float__t>::quiet_NaN ();
   im = std::numeric_limits<float__t>::quiet_NaN ();
   return;
  }
 re = (ln_num_re * ln_den_re + ln_num_im * ln_den_im) / denom;
 im = (ln_num_im * ln_den_re - ln_num_re * ln_den_im) / denom;
}

void vfunc2 (value *res, value *arg1, value *arg2, int idx)
{
 if (res == nullptr || arg1 == nullptr || arg2 == nullptr) return;
 if (((arg1->tag == tvCOMPLEX) || (arg2->tag == tvCOMPLEX) || (res->tag == tvCOMPLEX))
     || ((arg1->imval != 0.0) || (arg2->imval != 0.0) || (res->imval != 0.0)))
  {
   float__t out_re = 0.0;
   float__t out_im = 0.0;
   float__t re1    = arg1->get ();
   float__t im1    = arg1->imval;
   float__t re2    = arg2->get ();
   float__t im2    = arg2->imval;
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
    }
   res->imval = 0;
   res->tag   = tvFLOAT;
   res->ival  = (int64_t)res->fval;
  }
}

void vfunc (value *res, value *arg, int idx)
{
 if (res == nullptr || arg == nullptr) return;
 if (((arg->tag == tvCOMPLEX) || (res->tag == tvCOMPLEX))
     || ((arg->imval != 0.0) || (res->imval != 0.0)))
  {
   float__t out_re = 0.0;
   float__t out_im = 0.0;
   float__t re     = arg->get ();
   float__t im     = arg->imval;
   switch (idx)
    {
    case vf_abs:
     {
      res->fval  = hypotl (re, im);
      res->tag   = tvFLOAT;
      res->imval = 0.0;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_pol:
     {
      res->fval  = atan2l (im, re); // argument
      res->imval = 0.0;
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
    case vf_log:
     {
      LnC (re, im, out_re, out_im);
     }
     break;
    case vf_sqrt:
     {
      SqrtC (re, im, out_re, out_im);
     }
     break;
    case vf_re:
     {
      res->fval  = re; // argument
      res->imval = 0.0;
      res->tag   = tvFLOAT;
      res->ival  = (int64_t)res->fval;
     }
     return;
    case vf_im:
     {
      res->fval  = im; // argument
      res->imval = 0.0;
      res->tag   = tvFLOAT;
      res->ival  = (int64_t)res->fval;
     }
     return;
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
      res->fval = fabsl (arg->fval);
     }
     break;
    case vf_pol:
     {
      res->fval = 0.0;
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
    case vf_log:
     {
      res->fval = Log (arg->fval);
     }
     break;
    case vf_sqrt:
     {
      res->fval = Sqrt (arg->fval);
     }
     break;
    case vf_re:
     {
      res->fval = arg->fval;
     }
     break;
    case vf_im:
     {
      res->fval = 0.0;
     }
     break;
    }
   res->tag   = tvFLOAT;
   res->imval = 0.0;
   res->ival  = (int64_t)res->fval;
  }
}

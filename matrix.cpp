#pragma region Include Headers and Define Constants
#include <windows.h>
#ifdef __BORLANDC__
#include <stdint.h>
#include <malloc.h>
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

#else //__BORLANDC__

#define __USE_MINGW_ANSI_STDIO 1
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
#include <malloc.h>
#include <time.h>

#endif //__BORLANDC__

#ifdef _ENABLE_DEBUG_LOG_
#include <fstream>
#endif

#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"
#include "script.h"

#include "ver.h"

#ifdef _float128_
#include <quadmath.h>
#endif


#ifdef __BORLANDC__
#define M_PI_2l 1.5707963267948966192313216916398L
#define PHI     1.6180339887498948482045868343656L //(1+sqrt(5))/2 golden ratio
#define M_PId   3.1415926535897932384626433832795L
#define M_PI_2d 1.5707963267948966192313216916398L
#define M_Ed    2.71828182845904523536028747135266250L
#define PHId    1.61803398874989484820458683436563812L //(1+sqrt(5))/2 golden ratio
#define M_2PI   6.283185307179586476925286766559005768394L
#pragma warn -8004 // assigned a value that is never used
#pragma warn -8080
#pragma warn -8060
#pragma warn -8066
#pragma warn -8070
#pragma warn -8027

#include <float.h>
int isinf_f(float x) { return x < -FLT_MAX || x > FLT_MAX; }
int isinf_d(double x) { return x < -DBL_MAX || x > DBL_MAX; }
int isinf_l(float__t x) { return x < -LDBL_MAX||x > LDBL_MAX; }
#else //__BORLANDC__

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
#endif                                             /*_float128_*/
#ifndef __GNUC__
#pragma warning(disable : 4996) // 'function': was declared deprecated
#pragma warning(disable : 4244) // 'argument': conversion from 'type1' to 'type2', possible loss of data
#endif              //__GNUC__
#endif //__BORLANDC__
#pragma endregion

#pragma region MATRIX OPERATIONS

// matrixbin: binary operations, matrixuno: unary operations
// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Allocate a new matrix rows*cols, register for auto-free
// Returns NULL on failure (error already set)
float__t *calculator::mxAlloc (int rows, int cols)
{
 float__t *mval = (float__t *)sf_alloc (rows * cols * sizeof (float__t));
 if (!mval)
  {
   mxerror ("memory allocation failed");
   return NULL;
  }
 return mval;
}

// Fill res from left op right, element-wise, both must be same-size matrices
// op: 0=add, 1=sub, 2=mul, 3=div
bool calculator::mxElemOp (value &res, value &left, value &right, int op)
{
 if (left.mrows != right.mrows || left.mcols != right.mcols)
  {
   mxerror ("dimensions must match");
   return false;
  }
 int rows       = left.mrows;
 int cols       = left.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 switch (op)
  {
  case 0:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] + right.mval[i];
   break;
  case 1:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] - right.mval[i];
   break;
  case 2:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] * right.mval[i];
   break;
  case 3:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] / right.mval[i];
   break;
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// Fill res from matrix op scalar or scalar op matrix
// side: 0 = matrix op scalar, 1 = scalar op matrix
bool calculator::mxScalarOp (value &res, value &mx, float__t scalar, int op, bool scalar_left)
{
 int rows       = mx.mrows;
 int cols       = mx.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 switch (op)
  {
  case 0: // add: scalar+M == M+scalar
   for (int i = 0; i < n; i++) mval[i] = mx.mval[i] + scalar;
   break;
  case 1: // sub: M-scalar vs scalar-M
   if (scalar_left)
    for (int i = 0; i < n; i++) mval[i] = scalar - mx.mval[i];
   else
    for (int i = 0; i < n; i++) mval[i] = mx.mval[i] - scalar;
   break;
  case 2: // mul: scalar*M == M*scalar
   for (int i = 0; i < n; i++) mval[i] = mx.mval[i] * scalar;
   break;
  case 3: // div: M/scalar vs scalar/M (element-wise)
   if (scalar_left)
    for (int i = 0; i < n; i++) mval[i] = scalar / mx.mval[i];
   else
    for (int i = 0; i < n; i++) mval[i] = mx.mval[i] / scalar;
   break;
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// True matrix multiplication: res = left * right
bool calculator::mxMatMul (value &res, value &left, value &right)
{
 if (left.mcols != right.mrows)
  {
   mxerror ("dimensions incompatible for multiplication");
   return false;
  }
 int rows       = left.mrows;
 int cols       = right.mcols;
 int inner      = left.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++)
   {
    float__t sum = 0.0L;
    for (int k = 0; k < inner; k++) sum += left.mval[r * inner + k] * right.mval[k * cols + c];
    mval[r * cols + c] = sum;
   }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}
// Matrix scalar and matrix->matrix functions
// All use class fields: error(), mxAlloc(), registerString()
// Scalar results return float__t (qnan on error)
// Matrix results: bool fn(value &res, value &M) — false on error

// ---------------------------------------------------------------------------
// INTERNAL: Gauss-Jordan elimination on a square matrix
// Computes inverse and determinant simultaneously.
// aug[n][2n] = [M | I], modified in place.
// Returns true on success, false if singular.
// det is accumulated during elimination.
// ---------------------------------------------------------------------------
bool calculator::mxGaussJordan (float__t *aug, int n, float__t &det)
{
 det = 1.0L;
 for (int col = 0; col < n; col++)
  {
   // find pivot row (partial pivoting for numerical stability)
   int pivot_row = -1;
   float__t best = 0.0L;
   for (int row = col; row < n; row++)
    {
     float__t v = fabsl (aug[row * 2 * n + col]);
     if (v > best)
      {
       best      = v;
       pivot_row = row;
      }
    }
   if (pivot_row < 0 || best < 1e-15L)
    {
     det = 0.0L;
     mxerror ("is singular");
     return false;
    }
   // swap rows
   if (pivot_row != col)
    {
     for (int k = 0; k < 2 * n; k++)
      {
       float__t tmp = aug[col * 2 * n + k];
       aug[col * 2 * n + k] = aug[pivot_row * 2 * n + k];
       aug[pivot_row * 2 * n + k] = tmp;
      }
     det = -det; // row swap flips sign of determinant
    }
   // accumulate determinant
   float__t pivot = aug[col * 2 * n + col];
   det *= pivot;
   // scale pivot row
   for (int k = 0; k < 2 * n; k++) aug[col * 2 * n + k] /= pivot;
   // eliminate column in all other rows
   for (int row = 0; row < n; row++)
    {
     if (row == col) continue;
     float__t factor = aug[row * 2 * n + col];
     if (factor == 0.0L) continue;
     for (int k = 0; k < 2 * n; k++) aug[row * 2 * n + k] -= factor * aug[col * 2 * n + k];
    }
  }
 return true;
}

// ---------------------------------------------------------------------------
// INTERNAL: build augmented matrix [M | I] for Gauss-Jordan
// Returns allocated float__t[n * 2n] or NULL on error
// ---------------------------------------------------------------------------
float__t *calculator::mxMakeAug (value &M)
{
 int n         = M.mrows; // must be square, caller checks
 float__t *aug = (float__t *)malloc (n * 2 * n * sizeof (float__t));
 if (!aug)
  {
   mxerror ("memory allocation failed");
   return NULL;
  }
 for (int r = 0; r < n; r++)
  {
   for (int c = 0; c < n; c++) aug[r * 2 * n + c] = M.mval[r * n + c];          // left half: M
   for (int c = 0; c < n; c++) aug[r * 2 * n + n + c] = (r == c) ? 1.0L : 0.0L; // right half: I
  }
 return aug;
}

// ---------------------------------------------------------------------------
// SCALAR FUNCTIONS
// ---------------------------------------------------------------------------

// tr(M) — trace: sum of diagonal elements, defined for any matrix (min dimension)
float__t calculator::mxTrace (value &M)
{
 int n = (M.mrows < M.mcols) ? M.mrows : M.mcols;
 float__t sum = 0.0L;
 for (int i = 0; i < n; i++) sum += M.mval[i * M.mcols + i];
 return sum;
}

// det(M) — determinant, square matrix only
float__t calculator::mxDet (value &M)
{
 if (M.mrows != M.mcols)
  {
   mxerror ("det: matrix must be square");
   return qnan;
  }
 int n = M.mrows;
 // special cases for speed
 if (n == 1) return M.mval[0];
 if (n == 2) return M.mval[0] * M.mval[3] - M.mval[1] * M.mval[2];
 float__t *aug = mxMakeAug (M);
 if (!aug) return qnan;
 float__t det = 1.0L;
 bool ok = mxGaussJordan (aug, n, det);
 free (aug);
 return ok ? det : qnan;
}

// norm(M) — Frobenius norm: sqrt(sum of squares of all elements)
float__t calculator::mxNorm (value &M)
{
 float__t sum = (float__t)0.0L;
 int n = M.mrows * M.mcols;
 for (int i = 0; i < n; i++) sum += M.mval[i] * M.mval[i];
 return Sqrt (sum);
}

float__t calculator::mxDim (value & M, t_mxDim dim)
{ 
  switch (dim)
  {
   case mxRows:
   return M.mrows;
    break;
   case mxCols:
    return M.mcols;
    break;
   case mxSize:
    return M.mrows * M.mcols;
    break;
   default:
    return 0; // should not happen, caller checks
    break;
  }
}

// ---------------------------------------------------------------------------
// MATRIX -> MATRIX FUNCTIONS
// ---------------------------------------------------------------------------

// mxInv: inverse matrix via Gauss-Jordan, result stored in res
// Used for both inv(M) function and !M operator
bool calculator::mxInv (value &res, value &M)
{
 if (M.mrows != M.mcols)
  {
   mxerror ("inv: matrix must be square");
   return false;
  }
 int n = M.mrows;
 // special case 1x1
 if (n == 1)
  {
   if (fabsl (M.mval[0]) < 1e-15L)
    {
     mxerror ("inv: matrix is singular");
     return false;
    }
   float__t *mval = mxAlloc (1, 1);
   if (!mval) return false;
   mval[0]   = 1.0L / M.mval[0];
   res.tag   = tvMATRIX;
   res.mrows = 1;
   res.mcols = 1;
   res.mval  = mval;
   return true;
  }
 float__t *aug = mxMakeAug (M);
 if (!aug) return false;
 float__t det = 1.0L;
 if (!mxGaussJordan (aug, n, det))
  {
   free (aug);
   return false; // error already set by mxGaussJordan
  }
 // extract right half of aug -> result
 float__t *mval = mxAlloc (n, n);
 if (!mval)
  {
   free (aug);
   return false;
  }
 for (int r = 0; r < n; r++)
  for (int c = 0; c < n; c++) mval[r * n + c] = aug[r * 2 * n + n + c];
 free (aug);
 res.tag   = tvMATRIX;
 res.mrows = n;
 res.mcols = n;
 res.mval  = mval;
 return true;
}

// mxElem: element-wise matrix function
bool calculator::mxElemFn (v_func fidx, value &res, value &M)
{

 if (fidx == vf_pol_rt)
  {
   return mxPolyRoots (res, M);
  }

 bool result = true;
 int rows    = M.mrows;
 int cols    = M.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval)
  {
   mxerror ("memory allocation failed");
   return false;
  }
 int n = rows * cols;
 if (fidx == vf_rand)
  {
   float__t rmax = 0;
   for (int i = 0; i < n; i++) rmax += M.mval[i] * M.mval[i];
   rmax = Sqrt (rmax)/Sqrt(n);
   for (int i = 0; i < n; i++) mval[i] = Random(rmax);
  }
 else
 for (int i = 0; i < n; i++)
  {
   switch (fidx)
    {
    case vf_abs:
     mval[i] = Abs (M.mval[i]);
     break;
    case vf_floor:
     mval[i] = Floor (M.mval[i]);
     break;
    case vf_ceil:
     mval[i] = Ceil (M.mval[i]);
     break;
    case vf_round:
     mval[i] = Round (M.mval[i]);
     break;
    case vf_frac:
     mval[i] = Frac (M.mval[i]);
     break;
    case vf_int:
     mval[i] = Int (M.mval[i]);
     break;
    case vf_float:
     mval[i] = Float (M.mval[i]);
     break;
    default:
     {
      mxerror ("Illegal operation");
      result = false;
     }
     break;
    }
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return result;
}

// Find roots of polynomial given coefficients in a row-matrix
// Coefficients ordered from highest degree to constant: [a_n, a_(n-1), ..., a_1, a_0]
// For polynomial: a_n*x^n + a_(n-1)*x^(n-1) + ... + a_1*x + a_0 = 0
// Returns a matrix of complex roots (N rows × 2 cols: [real_part, imag_part])
// Supports polynomials up to degree 4
bool calculator::mxPolyRoots (value &res, value &coeffs)
{
 // Validate input: must be a row vector (1 row, N columns)
 if (coeffs.tag != tvMATRIX || coeffs.mrows != 1)
  {
   mxerror ("polynomial coefficients must be a row vector");
   return false;
  }

 int n = coeffs.mcols; // number of coefficients
 if (n < 2)
  {
   mxerror ("polynomial must have at least 2 coefficients");
   return false;
  }

 int degree = n - 1; // polynomial degree

 // Check if leading coefficient is non-zero
 if (coeffs.mval[0] == 0.0L)
  {
   mxerror ("leading coefficient must be non-zero");
   return false;
  }

 // Normalize coefficients (divide by leading coefficient)
 float__t *c = (float__t *)alloca (n * sizeof (float__t));
 //float__t c[8]; // max degree 4 => max 5 coefficients

 for (int i = 0; i < n; i++) c[i] = coeffs.mval[i] / coeffs.mval[0];

 // Allocate result matrix: degree rows × 2 columns (real, imag)
 float__t *roots = mxAlloc (degree, 2);
 if (!roots) return false;

 // Initialize all roots to zero
 for (int i = 0; i < degree * 2; i++) roots[i] = 0.0L;

 switch (degree)
  {
  case 1: // Linear: ax + b = 0 => x = -b/a
   {
    roots[0] = -c[1]; // real part
    roots[1] = 0.0L;  // imaginary part
   }
   break;

  case 2: // Quadratic: ax^2 + bx + c = 0
   {
    float__t b  = c[1];
    float__t cc = c[2];
    float__t D  = b * b - 4.0L * cc; // discriminant

    if (D >= 0.0L)
     {
      // Two real roots
      float__t sqrtD = Sqrt (D);
      roots[0]       = (-b + sqrtD) / 2.0L; // x1 real
      roots[1]       = 0.0L;                // x1 imag
      roots[2]       = (-b - sqrtD) / 2.0L; // x2 real
      roots[3]       = 0.0L;                // x2 imag
     }
    else
     {
      // Two complex conjugate roots
      float__t sqrtD = Sqrt (-D);
      roots[0]       = -b / 2.0L;     // x1 real
      roots[1]       = sqrtD / 2.0L;  // x1 imag
      roots[2]       = -b / 2.0L;     // x2 real
      roots[3]       = -sqrtD / 2.0L; // x2 imag
     }
   }
   break;

  case 3: // Cubic: x^3 + ax^2 + bx + c = 0 (Cardano's formula)
   {
    float__t a  = c[1];
    float__t b  = c[2];
    float__t cc = c[3];

    // Convert to depressed cubic: t^3 + pt + q = 0
    // where x = t - a/3
    float__t p = b - a * a / 3.0L;
    float__t q = 2.0L * a * a * a / 27.0L - a * b / 3.0L + cc;

    // Discriminant for cubic
    float__t D = -(4.0L * p * p * p + 27.0L * q * q);

    float__t offset = -a / 3.0L;

    if (D > 0)
     {
      // Three distinct real roots (casus irreducibilis)
      float__t m   = 2.0L * Sqrt (-p / 3.0L);
      float__t phi = Acos (3.0L * q / (p * m)) / 3.0L;

      roots[0] = m * Cos (phi) + offset;
      roots[1] = 0.0L;
      roots[2] = m * Cos (phi - 2.0L * M_PI / 3.0L) + offset;
      roots[3] = 0.0L;
      roots[4] = m * Cos (phi - 4.0L * M_PI / 3.0L) + offset;
      roots[5] = 0.0L;
     }
    else
     {
      // One real root and two complex conjugate roots
      float__t delta = q * q / 4.0L + p * p * p / 27.0L;
      float__t u, v;

      if (delta >= 0)
       {
        float__t sqrtDelta = Sqrt (delta);
        float__t A         = -q / 2.0L + sqrtDelta;
        float__t B         = -q / 2.0L - sqrtDelta;

        u = (A >= 0) ? Pow (A, 1.0L / 3.0L) : -Pow (-A, 1.0L / 3.0L);
        v = (B >= 0) ? Pow (B, 1.0L / 3.0L) : -Pow (-B, 1.0L / 3.0L);
       }
      else
       {
        float__t r     = Sqrt (-p * p * p / 27.0L);
        float__t theta = Acos (-q / (2.0L * r));
        u              = 2.0L * Pow (r, 1.0L / 3.0L) * Cos (theta / 3.0L);
        v              = 0.0L;
       }

      // Real root
      roots[0] = u + v + offset;
      roots[1] = 0.0L;

      // Complex conjugate pair
      float__t re_part = -(u + v) / 2.0L + offset;
      float__t im_part = (u - v) * Sqrt (3.0L) / 2.0L;

      roots[2] = re_part;
      roots[3] = im_part;
      roots[4] = re_part;
      roots[5] = -im_part;
     }
   }
   break;

   case 4: // Quartic: x^4 + ax^3 + bx^2 + cx + d = 0 (Ferrari's method)
   {
    float__t a  = c[1];
    float__t b  = c[2];
    float__t cc = c[3];
    float__t d  = c[4];

    // Depressed quartic: y^4 + py^2 + qy + r = 0
    // where x = y - a/4
    float__t offset = -a / 4.0L;
    float__t a2     = a * a;
    float__t p      = b - 3.0L * a2 / 8.0L;
    float__t q      = a2 * a / 8.0L - a * b / 2.0L + cc;
    float__t r      = -3.0L * a2 * a2 / 256.0L + a2 * b / 16.0L - a * cc / 4.0L + d;

    if (fabsl (q) < 1e-10L)
     {
      // Biquadratic: y^4 + py^2 + r = 0
      // Substitute z = y^2: z^2 + pz + r = 0
      float__t D = p * p - 4.0L * r;

      float__t sqrtD = (D >= 0) ? Sqrt (D) : Sqrt (-D);
      float__t z1    = (-p + (D >= 0 ? sqrtD : 0.0L)) / 2.0L;
      float__t z2    = (-p - (D >= 0 ? sqrtD : 0.0L)) / 2.0L;

      if (D >= 0)
       {
        // Two real values for z
        // y = ±sqrt(z)
        if (z1 >= 0)
         {
          float__t sqrtz1 = Sqrt (z1);
          roots[0]        = sqrtz1 + offset;
          roots[1]        = 0.0L;
          roots[2]        = -sqrtz1 + offset;
          roots[3]        = 0.0L;
         }
        else
         {
          float__t sqrtz1 = Sqrt (-z1);
          roots[0]        = offset;
          roots[1]        = sqrtz1;
          roots[2]        = offset;
          roots[3]        = -sqrtz1;
         }

        if (z2 >= 0)
         {
          float__t sqrtz2 = Sqrt (z2);
          roots[4]        = sqrtz2 + offset;
          roots[5]        = 0.0L;
          roots[6]        = -sqrtz2 + offset;
          roots[7]        = 0.0L;
         }
        else
         {
          float__t sqrtz2 = Sqrt (-z2);
          roots[4]        = offset;
          roots[5]        = sqrtz2;
          roots[6]        = offset;
          roots[7]        = -sqrtz2;
         }
       }
      else
       {
        // D < 0: z is complex, so z = (-p ± i*sqrt(-D))/2
        // Both z values are complex conjugates
        float__t z_re = -p / 2.0L;
        float__t z_im = sqrtD / 2.0L;

        // For z1 = z_re + i*z_im, we need y = ±sqrt(z)
        // sqrt(a + bi) = sqrt(r)*[cos(θ/2) + i*sin(θ/2)]
        // where r = |z| = sqrt(a^2 + b^2), θ = atan2(b, a)

        float__t z1_abs = Sqrt (z_re * z_re + z_im * z_im);
        float__t z1_arg = Atan2l (z_im, z_re);
        float__t sqrt_r = Sqrt (z1_abs);

        // y1 = sqrt(z1) = sqrt_r * [cos(θ/2) + i*sin(θ/2)]
        float__t y1_re = sqrt_r * Cos (z1_arg / 2.0L);
        float__t y1_im = sqrt_r * Sin (z1_arg / 2.0L);

        roots[0] = y1_re + offset;
        roots[1] = y1_im;
        roots[2] = -y1_re + offset;
        roots[3] = -y1_im;

        // For z2 = z_re - i*z_im (complex conjugate)
        float__t z2_arg = Atan2l (-z_im, z_re);

        // y3 = sqrt(z2)
        float__t y3_re = sqrt_r * Cos (z2_arg / 2.0L);
        float__t y3_im = sqrt_r * Sin (z2_arg / 2.0L);

        roots[4] = y3_re + offset;
        roots[5] = y3_im;
        roots[6] = -y3_re + offset;
        roots[7] = -y3_im;
       }
     }
    else
     {
      // General case: solve resolvent cubic
      // m^3 - 2pm^2 + (p^2 - 4r)m + q^2 = 0
      // We need to find the largest positive real root m
      float__t ac  = -2.0L * p;
      float__t bc  = p * p - 4.0L * r;
      float__t ccc = q * q;

      // Depressed cubic for m: t^3 + pt + q = 0
      float__t pc = bc - ac * ac / 3.0L;
      float__t qc = 2.0L * ac * ac * ac / 27.0L - ac * bc / 3.0L + ccc;

      float__t delta = qc * qc / 4.0L + pc * pc * pc / 27.0L;
      float__t m;

      if (delta >= 0)
       {
        // One real root and two complex conjugate roots
        float__t sqrtDelta = Sqrt (delta);
        float__t A         = -qc / 2.0L + sqrtDelta;
        float__t B         = -qc / 2.0L - sqrtDelta;

        float__t u = (A >= 0) ? Pow (A, 1.0L / 3.0L) : -Pow (-A, 1.0L / 3.0L);
        float__t v = (B >= 0) ? Pow (B, 1.0L / 3.0L) : -Pow (-B, 1.0L / 3.0L);

        m = u + v - ac / 3.0L;
       }
      else
       {
        // Three real roots - choose the largest positive one
        float__t rr     = Sqrt (-pc * pc * pc / 27.0L);
        float__t theta  = Acos (-qc / (2.0L * rr));
        float__t cbrt_r = Pow (rr, 1.0L / 3.0L);

        float__t m1 = 2.0L * cbrt_r * Cos (theta / 3.0L) - ac / 3.0L;
        float__t m2 = 2.0L * cbrt_r * Cos ((theta - 2.0L * M_PI) / 3.0L) - ac / 3.0L;
        float__t m3 = 2.0L * cbrt_r * Cos ((theta - 4.0L * M_PI) / 3.0L) - ac / 3.0L;

        // Choose the largest positive root
        m = m1;
        if (m2 > m && m2 > 0) m = m2;
        if (m3 > m && m3 > 0) m = m3;
       }

      // Ensure m is positive (or at least non-negative)
      if (m < 0.0L) m = 0.0L;

      // Now solve two quadratics using the resolvent root m
      // y^4 + py^2 + qy + r = (y^2 + s1*y + t1)(y^2 + s2*y + t2)
      // where s1 = sqrt(m), s2 = -sqrt(m)
      // and t1, t2 are determined from p, q, r

      float__t sqrtm = Sqrt (m);

      // From Ferrari's method:
      // s1 + s2 = 0 => s1 = sqrt(m), s2 = -sqrt(m)
      // t1 + t2 = p + m
      // s1*t2 + s2*t1 = q => sqrt(m)*(t2 - t1) = q
      // t1*t2 = r

      float__t t_sum  = p + m;
      float__t t_diff = (sqrtm != 0.0L) ? q / sqrtm : 0.0L;

      float__t t1 = (t_sum - t_diff) / 2.0L;
      float__t t2 = (t_sum + t_diff) / 2.0L;

      // First quadratic: y^2 + sqrt(m)*y + t1 = 0
      float__t D1 = sqrtm * sqrtm - 4.0L * t1;
      if (D1 >= 0)
       {
        float__t sqrtD1 = Sqrt (D1);
        roots[0]        = (-sqrtm + sqrtD1) / 2.0L + offset;
        roots[1]        = 0.0L;
        roots[2]        = (-sqrtm - sqrtD1) / 2.0L + offset;
        roots[3]        = 0.0L;
       }
      else
       {
        float__t sqrtD1 = Sqrt (-D1);
        roots[0]        = -sqrtm / 2.0L + offset;
        roots[1]        = sqrtD1 / 2.0L;
        roots[2]        = -sqrtm / 2.0L + offset;
        roots[3]        = -sqrtD1 / 2.0L;
       }

      // Second quadratic: y^2 - sqrt(m)*y + t2 = 0
      float__t D2 = sqrtm * sqrtm - 4.0L * t2;
      if (D2 >= 0)
       {
        float__t sqrtD2 = Sqrt (D2);
        roots[4]        = (sqrtm + sqrtD2) / 2.0L + offset;
        roots[5]        = 0.0L;
        roots[6]        = (sqrtm - sqrtD2) / 2.0L + offset;
        roots[7]        = 0.0L;
       }
      else
       {
        float__t sqrtD2 = Sqrt (-D2);
        roots[4]        = sqrtm / 2.0L + offset;
        roots[5]        = sqrtD2 / 2.0L;
        roots[6]        = sqrtm / 2.0L + offset;
        roots[7]        = -sqrtD2 / 2.0L;
       }
     }
   }
   break;
   default: // Polynomial degree > 4: use Durand-Kerner method (numerical)
    {
     // Durand-Kerner (Weierstrass) method for finding all roots simultaneously
     const int max_iterations = 100;
     const float__t tolerance = 1e-12L;

     // Allocate temporary arrays for current and next approximations
     float__t *roots_re = (float__t *)alloca (degree * sizeof (float__t));
     float__t *roots_im = (float__t *)alloca (degree * sizeof (float__t));
     float__t *next_re  = (float__t *)alloca (degree * sizeof (float__t));
     float__t *next_im  = (float__t *)alloca (degree * sizeof (float__t));

     // Initialize with spiral pattern: r_k = r^k * e^(2πik/n)
     // where r = max(1, |a_(n-1)/a_n|^(1/n)) to get good initial radius
     float__t init_radius = 1.0L;
     if (degree > 1 && fabsl (c[1]) > 0.0L) init_radius = Pow (fabsl (c[1]), 1.0L / degree);
     if (init_radius < 1.0L) init_radius = 1.0L;

     for (int k = 0; k < degree; k++)
      {
       float__t angle = 2.0L * M_PI * k / degree;
       float__t r     = init_radius * Pow (0.9L + 0.1L * k / degree, (float__t)k);
       roots_re[k]    = r * Cos (angle);
       roots_im[k]    = r * Sin (angle);
      }

     // Durand-Kerner iterations
     bool converged = false;
     for (int iter = 0; iter < max_iterations && !converged; iter++)
      {
       converged = true;

       // For each root approximation
       for (int i = 0; i < degree; i++)
        {
         // Evaluate P(z_i) where P is the polynomial
         float__t p_re = c[0];
         float__t p_im = 0.0L;

         for (int j = 1; j <= degree; j++)
          {
           // Multiply by z_i: (p_re + i*p_im) * (roots_re[i] + i*roots_im[i])
           float__t temp_re = p_re * roots_re[i] - p_im * roots_im[i];
           float__t temp_im = p_re * roots_im[i] + p_im * roots_re[i];

           // Add coefficient c[j]
           p_re = temp_re + c[j];
           p_im = temp_im;
          }

         // Compute product of (z_i - z_j) for all j ≠ i
         float__t prod_re = 1.0L;
         float__t prod_im = 0.0L;

         for (int j = 0; j < degree; j++)
          {
           if (j != i)
            {
             // (z_i - z_j)
             float__t diff_re = roots_re[i] - roots_re[j];
             float__t diff_im = roots_im[i] - roots_im[j];

             // Multiply: prod *= diff
             float__t temp_re = prod_re * diff_re - prod_im * diff_im;
             float__t temp_im = prod_re * diff_im + prod_im * diff_re;

             prod_re = temp_re;
             prod_im = temp_im;
            }
          }

         // Divide P(z_i) by product: correction = P(z_i) / prod
         float__t denom = prod_re * prod_re + prod_im * prod_im;

         if (denom > 1e-30L)
          {
           float__t corr_re = (p_re * prod_re + p_im * prod_im) / denom;
           float__t corr_im = (p_im * prod_re - p_re * prod_im) / denom;

           // Update: z_i = z_i - correction
           next_re[i] = roots_re[i] - corr_re;
           next_im[i] = roots_im[i] - corr_im;

           // Check convergence
           float__t change = Sqrt (corr_re * corr_re + corr_im * corr_im);
           if (change > tolerance) converged = false;
          }
         else
          {
           // Product too small (roots too close), keep current value
           next_re[i] = roots_re[i];
           next_im[i] = roots_im[i];
          }
        }

       // Copy next to current
       for (int i = 0; i < degree; i++)
        {
         roots_re[i] = next_re[i];
         roots_im[i] = next_im[i];
        }
      }

     // Clean up near-zero imaginary parts (numerical noise)
     for (int i = 0; i < degree; i++)
      {
       if (fabsl (roots_im[i]) < tolerance * fabsl (roots_re[i])) roots_im[i] = 0.0L;
      }

     // Sort roots: real roots first (sorted by real part), then complex pairs
     // Use simple bubble sort (degree is small)
     for (int i = 0; i < degree - 1; i++)
      {
       for (int j = i + 1; j < degree; j++)
        {
         bool swap = false;

         // Real roots come before complex
         bool i_is_real = (fabsl (roots_im[i]) < tolerance);
         bool j_is_real = (fabsl (roots_im[j]) < tolerance);

         if (j_is_real && !i_is_real)
          swap = true;
         else if (i_is_real && j_is_real)
          {
           // Both real: sort by real part
           if (roots_re[j] < roots_re[i]) swap = true;
          }
         else if (!i_is_real && !j_is_real)
          {
           // Both complex: sort by real part, then by imaginary part
           if (roots_re[j] < roots_re[i])
            swap = true;
           else if (fabsl (roots_re[j] - roots_re[i]) < tolerance && roots_im[j] < roots_im[i])
            swap = true;
          }

         if (swap)
          {
           // Swap roots[i] and roots[j]
           float__t temp = roots_re[i];
           roots_re[i]   = roots_re[j];
           roots_re[j]   = temp;

           temp        = roots_im[i];
           roots_im[i] = roots_im[j];
           roots_im[j] = temp;
          }
        }
      }

     // Copy results to output matrix
     for (int i = 0; i < degree; i++)
      {
       roots[i * 2]     = roots_re[i];
       roots[i * 2 + 1] = roots_im[i];
      }
    }
    break;
   }

 res.tag   = tvMATRIX;
 res.mrows = degree;
 res.mcols = 2;
 res.mval  = roots;
 return true;
}

double calculator::Median (const char *fname, const char *msk,double totalN, double minV, double maxV)
{
 double low         = minV;
 double high        = maxV;
 double targetCount = totalN / 2.0;
 double mid         = low;

 // 45-50 iterations are sufficient to exhaust the precision of double
 for (int i = 0; i < 50; i++)
  {
   mid                 = (low + high) / 2.0;
   double currentCount = 0;

   FILE *f = nullptr;
   f=fopen (fname, "r");
   if (f)
    {
     char line[1024];
     double v = qnan;
     while (fgets (line, sizeof (line), f))
      {
       // Use "all-purpose" scanner
       if (strscan (line,msk, 1, &v) == 1)
        {
         if (v <= mid) currentCount++;
        }
      }
     fclose (f);
    }
   else
    {
     mxerror ("cannot open data file");
     return qnan;
    }

   if (currentCount <= targetCount)
    low = mid;
   else
    high = mid;

   // If the interval becomes negligibly small, exit
   if (fabs (high - low) < 1e-9) break;
  }
 return mid;
}

// StatFn: compute statistical function on a column of numbers read from a file
float__t calculator::StatFn (const char *fname, const char *msk, sfntype sfn, float__t x)
{
 FILE *f    = nullptr;
 double n = 0, sumX = 0, sumX2 = 0, mean = 0, M2 = 0;
 double minVal = qnan, maxVal = qnan;

 f = fopen(fname, "r");
 if (f)
  {
   char line[1024];
   while (fgets (line, sizeof (line), f))
    {
     double xx = qnan;
     if (strscan (line, msk, 1, &xx) == 1)
      { // Read only the first number in the line
       if (n == 0)
        {
         minVal = maxVal = xx;
        }
       else
        {
         if (xx < minVal) minVal = xx;
         if (xx > maxVal) maxVal = xx;
        }
       // Accumulation (Welford's method or simple sums)
       n++;
       double delta = xx - mean;
       mean += delta / n;
       M2 += delta * (xx - mean);
       sumX += xx;
       sumX2 += xx * xx;
      }
    }
   fclose (f);
  }
 else
  {
   mxerror ("cannot open data file");
   return qnan;
  }

 switch (sfn)
  {
  case sfNum: // Number of values
   return (float__t)n;
  case sfMean: // Arithmetic mean
   return (float__t)mean;
  case sfMedian: // Median (not implemented, requires storing all values)
   return (float__t)Median(fname, msk, n, minVal, maxVal);
  case sfRMS:                             // Root mean square
   return (float__t)((n > 0) ? sqrt (sumX2 / n) : 0.0L); // Root mean square 
  case sfSumX:                            // Sum of values
   return (float__t)sumX;
  case sfStdDevP:         // Population standard deviation
   return (float__t)((n > 0) ? sqrt (M2 / n) : 0.0L); // Population standard deviation (sigma)
  case sfStdDevS:         // Sample standard deviation
   return (float__t)((n > 1) ? sqrt (M2 / (n - 1)) : 0.0L); // Sample standard deviation (s)
  case sfMin:             // Minimum value
   return (float__t)minVal;
  case sfMax:             // Maximum value
   return (float__t)maxVal;

  case sfNormP: // Probability P(t): P(X <= x)
  case sfNormQ: // Probability Q(t): P(0 <= X <= t) or P(mean <= X <= x)
  case sfNormR: // Probability R(t): P(X > x)
   {
    float__t m = (float__t)mean;
    float__t s = (float__t)((n > 1) ? sqrt (M2 / (n - 1)) : 0.0L);

    if (s == 0)
     {
      if (sfn == sfNormP) return (x >= m) ? 1.0 : 0.0;
      if (sfn == sfNormQ) return 0.0;
      if (sfn == sfNormR) return (x > m) ? 0.0 : 1.0;
     }

    // 1. Evaluate Z-score (normalized deviation)
    // t = (x - mean) / sigma
    float__t t_score = (x - m) / s;

    // 2. Compute P(t) - basic integral function
    // Using Erf(t / sqrt(2))
    float__t p_val = 0.5 * (1.0 + Erf (t_score / sqrt (2.0)));

    if (sfn == sfNormP) return p_val;

    if (sfn == sfNormQ)
     {
      // Q(t) in Casio — this is the probability that the value lies between
      // the mean and x. Mathematically, this is |P(t) - 0.5|
      return fabsl (p_val - 0.5);
     }

    if (sfn == sfNormR)
     {
      // R(t) — this is the "right tail", the probability that the value > x
      // Mathematically, this is 1.0 - P(t)
      return 1.0 - p_val;
     }
   }
   break;
  case sfInvNorm: // Inverse Normal Distribution
   {
    // x here — this is the probability P (from 0 to 1) provided by the user
    float__t P = x;
    if (P <= 0.0L || P >= 1.0L)
     {
      mxerror ("probability must be between 0 and 1");
      return qnan;
     }

    float__t m = (float__t)mean;
    float__t s = (float__t)((n > 1) ? Sqrt (M2 / (n - 1)) : 0.0L);

    // 1. Find the Z-score using erfinv
    // Use Erfinv_refine for higher accuracy if needed
    float__t z = Sqrt (2.0) * Erfinv (2.0 * P - 1.0);
    //float__t z = Sqrt (2.0) * Erfinv_refine (2.0 * P - 1.0, 50);

    // 2. Denormalize: x = mu + z * sigma
    return m + z * s;
   }
  case sfNormPD:
   {
    float__t m = (float__t)mean;
    float__t s = (float__t)((n > 1) ? Sqrt (M2 / (n - 1)) : 0.0L);
    if (s == 0) return (x == m) ? inf : 0.0;

    float__t diff = x - m;
    // Classical Gaussian formula
    return (1.0 / (s * Sqrt (M_2PI))) * Exp (-(diff * diff) / (2.0 * s * s));
   }
  default:
   return (float__t)0.0L;
  }
 return qnan; // Should never reach here
}

bool calculator::mxRegrFn (const char *fname, const char *msk, int n, rtype rt, value &res)
{
 FILE *f = nullptr;
 
 if (n<=0)
  {
   mxerror ("degree must be positive");
   return false;
  }
 // Linearized types always have degree 1 (line y=a+bx)
 int degree = (rt == rtPoly) ? n : 1;

 if (degree > 6)
  {
   mxerror ("degree too big (max 6)");
   return false;
  }

 // Initialize accumulators on the temporary stack
 int s_size   = 2 * degree + 1;
 int sy_size  = degree + 1;
 float__t *S  = (float__t *)alloca (s_size * sizeof (float__t));
 float__t *SY = (float__t *)alloca (sy_size * sizeof (float__t));

 for (int i = 0; i < s_size; i++) S[i] = 0;
 for (int i = 0; i < sy_size; i++) SY[i] = 0;

 // 1. Stream reading of the file and accumulation of sums
 f = fopen (fname, "r");
 if (f)
  {
   char line[1024];
   while (fgets (line, sizeof (line), f))
    {
     double xd = qnan, yd = qnan;
     // strscan ignores garbage and understands suffixes (100k, 5m)
     if (strscan (line, msk, 2, &xd, &yd) == 2)
      {
       float__t x = (float__t)xd;
       float__t y = (float__t)yd;
       // Linearization of data before least squares
       switch (rt)
        {
        case rtExp:
         if (y <= 0) continue;
         y = Log (y);
         break;
        case rtLg:
         if (x <= 0) continue;
         x = Log (x);
         break;
        case rtPow:
         if (x <= 0 || y <= 0) continue;
         x = Log (x);
         y = Log (y);
         break;
        case rtInv:
         if (x == 0) continue;
         x = (float__t)(1.0L / x);
         break;
        default:
         break;
        }

       // Accumulate sums of powers
       float__t px = (float__t)1.0L;
       for (int i = 0; i < s_size; i++)
        {
         S[i] += px;
         if (i < sy_size) SY[i] += y * px;
         px *= x;
        }
      }
    }
   fclose (f);
  }
 else
  {
   mxerror ("cannot open data file");
   return false;
  }

 // 2. Forming matrices to solve the system M * A = B
 value valM, valB, valInvM, valCoeff;
 int dim = degree + 1;

 valM.tag   = tvMATRIX;
 valM.mrows = dim;
 valM.mcols = dim;
 valM.mval  = mxAlloc (dim, dim);

 valB.tag   = tvMATRIX;
 valB.mrows = dim;
 valB.mcols = 1;
 valB.mval  = mxAlloc (dim, 1);

 for (int r = 0; r < dim; r++)
  {
   for (int c = 0; c < dim; c++)
    {
     valM.mval[r * dim + c] = S[r + c];
    }
   valB.mval[r] = SY[r];
  }

 // 3. Solving the system using internal functions
 // A = inv(M) * B
 if (!mxInv (valInvM, valM))
  {
   mxerror ("Matrix is singular (not enough distinct points?)");
   return false;
  }

 if (!mxMatMul (valCoeff, valInvM, valB)) return false;

 // 4. Post-processing and preparing the result
 // mval for the result (matrix-row 1 x n+1)
 float__t *res_mval = mxAlloc (1, dim);

 // Transfer coefficients in order from highest degree to lowest (as in polynomial)
 for (int i = 0; i < dim; i++)
  {
   res_mval[i] = valCoeff.mval[degree - i];
  }

 // Inverse transformation for exponential and power types: ln(a) -> a
 if (rt == rtExp || rt == rtPow)
  {
   // Coefficient 'a' is at the end of the res_mval array (corresponds to a0)
   res_mval[dim - 1] = expl (res_mval[dim - 1]);
  }

 res.tag   = tvMATRIX;
 res.mrows = 1;
 res.mcols = dim;
 res.mval  = res_mval;

 return true;
}

// mxCalcFn: calculate the value of the regression function at a given x using the coefficients in M
float__t calculator::mxCalcFn(value M, rtype rt, float__t x)
{
  if (M.tag != tvMATRIX)
   {
    mxerror ("matrix required");
    return qnan;
   }
 
  switch (rt)
   {
    case rtPoly:
    {
     if (M.mrows == 1 && M.mcols <= MAX_C)
      {
       // Coefficients ordered from highest degree to constant: [a_n, a_(n-1), ..., a_1, a_0]
       // For polynomial: a_n*x^n + a_(n-1)*x^(n-1) + ... + a_1*x + a_0 = 0
       // Horner's method for evaluating polynomial at x 
       float__t result = 0.0L;
       for (int i = 0; i < M.mcols; i++)
        {
         result = result * x + M.mval[i];
        }
       return result;
      }
     else
     {
       mxerror ("polynomial coefficients must be a row vector");
       return qnan;
     }
    }
    break;
    case rtExp: //y=a*exp(b*x), [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a * Exp (b * x);
       }
      else
       {
        mxerror ("exponential regression requires exactly 2 coefficients");
        return qnan;
       }
     }
     break;
     case rtPow: //y=a*x^b, [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a * Pow (x, b);
       }
      else
       {
        mxerror ("power regression requires exactly 2 coefficients");
        return qnan;
       }
     }
     break;
     case rtLg: // y=a+b*ln(x), [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a + b * Log (x);
       }
      else
       {
        mxerror ("logarithmic regression requires exactly 2 coefficients");
        return qnan;
       }
     }
    break;
    case rtInv: // y=a+b/x, [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a + b / x;
       }
      else
       {
        mxerror ("inverse regression requires exactly 2 coefficients");
        return qnan;
       }
     }
     break;
   }
  return qnan; // Should not reach here
}

// mxNeg: element-wise negation (unary minus) — also in matrixuno, here for completeness
bool calculator::mxNeg (value &res, value &M)
{
 int rows = M.mrows;
 int cols = M.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = -M.mval[i];
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxTranspose: transpose — also in matrixuno via ~, here for completeness
bool calculator::mxTranspose (value &res, value &M)
{
 int rows = M.mrows;
 int cols = M.mcols;
 float__t *mval = mxAlloc (cols, rows); // note swapped dimensions
 if (!mval) return false;
 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++) mval[c * rows + r] = M.mval[r * cols + c];
 res.tag   = tvMATRIX;
 res.mrows = cols;
 res.mcols = rows;
 res.mval  = mval;
 return true;
}

// mxZeros: matrix of zeros
bool calculator::mxZeros (value &res, int rows, int cols)
{
 if (rows <= 0 || cols <= 0)
  {
   mxerror ("dimensions must be positive");
   return false;
  }
 if (rows > MAX_R || cols > MAX_C)
  {
   mxerror ("dimensions too large");
   return false;
  }
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) 
  {
   mxerror ("out of memory");
   return false;
  }
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = (float__t)0.0L;
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxDiag: diagonal matrix 
bool calculator::mxDiag (value &res, int rows, int cols)
{
 if (rows <= 0 || cols <= 0)
  {
   mxerror ("dimensions must be positive");
   return false;
  }
 if (rows > MAX_R || cols > MAX_C)
  {
   mxerror ("dimensions too large");
   return false;
  }
 float__t *mval = mxAlloc (rows, cols);
 if (!mval)
  {
   mxerror ("out of memory");
   return false;
  }
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = (float__t)0.0L;
 for (int i = 0; i < rows && i < cols; i++) mval[i * cols + i] = (float__t)1.0L;
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxDot: dot product of two vectors (1xN or Nx1)
// Returns scalar result in res.fval
bool calculator::mxDot (value & res, value & A, value & B)
{
 if (A.tag != tvMATRIX || B.tag != tvMATRIX)
  {
   mxerror ("dot: both arguments must be matrices");
   return false;
  }
 // both must be vectors (one dimension == 1) and same total size
 bool aVec = (A.mrows == 1 || A.mcols == 1);
 bool bVec = (B.mrows == 1 || B.mcols == 1);
 if (!aVec || !bVec)
  {
   mxerror ("dot: arguments must be vectors (1xN or Nx1)");
   return false;
  }
 int na = A.mrows * A.mcols;
 int nb = B.mrows * B.mcols;
 if (na != nb)
  {
   mxerror ("dot: vector sizes must match");
   return false;
  }
 float__t sum = 0.0L;
 for (int i = 0; i < na; i++) sum += A.mval[i] * B.mval[i];
 res.tag   = tvFLOAT;
 res.fval  = sum;
 res.imval = ((float__t)0.0L);
 return true;
}

// mxCross: cross product of two 3D vectors (1x3 or 3x1)
// Result is a vector of the same shape as A
bool calculator::mxCross (value & res, value & A, value & B)
{
 if (A.tag != tvMATRIX || B.tag != tvMATRIX)
  {
   mxerror ("cross: both arguments must be matrices");
   return false;
  }
 bool aVec = (A.mrows == 1 || A.mcols == 1);
 bool bVec = (B.mrows == 1 || B.mcols == 1);
 if (!aVec || !bVec)
  {
   mxerror ("cross: arguments must be vectors (1x3 or 3x1)");
   return false;
  }
 int na = A.mrows * A.mcols;
 int nb = B.mrows * B.mcols;
 if (na != 3 || nb != 3)
  {
   mxerror ("cross: cross product requires 3-element vectors");
   return false;
  }
 float__t *mval = mxAlloc (A.mrows, A.mcols); // same shape as A
 if (!mval) return false;
 mval[0]   = A.mval[1] * B.mval[2] - A.mval[2] * B.mval[1];
 mval[1]   = A.mval[2] * B.mval[0] - A.mval[0] * B.mval[2];
 mval[2]   = A.mval[0] * B.mval[1] - A.mval[1] * B.mval[0];
 res.tag   = tvMATRIX;
 res.mrows = A.mrows;
 res.mcols = A.mcols;
 res.mval  = mval;
 return true;
}
// ---------------------------------------------------------------------------
// matrixbin — binary operations
// ---------------------------------------------------------------------------
t_mresult calculator::matrixbin (value &res, value &left, value &right, t_operator cop)
{
 if (left.tag != tvMATRIX && right.tag != tvMATRIX) return mrSKIP; // not a matrix operation

 bool lm = (left.tag == tvMATRIX);
 bool rm = (right.tag == tvMATRIX);
 bool ls = left.is_scalar ();
 bool rs = right.is_scalar ();

 switch (cop)
  {
  // ---- ADD ----
  case toADD:
  case toSETADD:
   if (lm && rm)
    {
     if (!mxElemOp (res, left, right, 0)) return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 0, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 0, true)) return mrERROR;
    }
   return mrDONE;

  // ---- SUB ----
  case toSUB:
  case toSETSUB:
   if (lm && rm)
    {
     if (!mxElemOp (res, left, right, 1)) return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 1, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 1, true)) return mrERROR;
    }
   return mrDONE;

  // ---- MUL ----
  case toMUL:
  case toSETMUL:
   if (lm && rm)
    {
     if (!mxMatMul (res, left, right)) return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 2, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 2, true)) return mrERROR;
    }
   return mrDONE;

  // ---- DIV ----
  case toDIV:
  case toSETDIV:
   if (lm && rm)
    {
     mxerror ("division is not defined (use inv())");
     return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 3, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 3, true)) return mrERROR;
    }
   return mrDONE;

  // ---- POW ----
  case toPOW:
  case toSETPOW:
   // M^n — repeated multiplication, only integer n, square matrix
   if (lm && rs)
    {
     if (left.mrows != left.mcols)
      {
       mxerror ("Matrix power requires a square matrix");
       return mrERROR;
      }
     long long n = (long long)right.fval;
     if ((float__t)n != right.fval || n < 0)
      {
       mxerror ("Matrix power requires a non-negative integer exponent");
       return mrERROR;
      }
     // start with identity matrix
     // deep copy of left before we start writing to res
     // (res and left may alias the same v_stack entry)
     int sz = left.mrows;
     int sz2 = sz * sz;
     float__t *curbuf = (float__t *)malloc (sz2 * sizeof (float__t));
     if (!curbuf)
      {
       mxerror ("memory allocation failed");
       return mrERROR;
      }
     memcpy (curbuf, left.mval, sz2 * sizeof (float__t));
     value cur = left;
     cur.mval  = curbuf; // cur now has its own independent copy

     // start with identity matrix
     float__t *mval = mxAlloc (sz, sz);
     if (!mval)
      {
       free (curbuf);
       return mrERROR;
      }
     for (int r = 0; r < sz; r++)
      for (int c = 0; c < sz; c++) mval[r * sz + c] = (r == c) ? 1.0L : 0.0L;
     res.tag   = tvMATRIX;
     res.mrows = sz;
     res.mcols = sz;
     res.mval  = mval;

     for (long long i = 0; i < n; i++)
      {
       float__t *tmpbuf = (float__t *)malloc (sz2 * sizeof (float__t));
       if (!tmpbuf)
        {
         free (curbuf);
         mxerror ("memory allocation failed");
         return mrERROR;
        }
       memcpy (tmpbuf, res.mval, sz2 * sizeof (float__t));
       value tmp = res;
       tmp.mval  = tmpbuf;
       if (!mxMatMul (res, tmp, cur))
        {
         free (tmpbuf);
         free (curbuf);
         return mrERROR;
        }
       free (tmpbuf);
      }
     free (curbuf);
     return mrDONE;
    }
   error ("Matrix power: left must be matrix, right must be non-negative integer scalar");
   return mrERROR;

  // ---- parallel resistors M//M or M//scalar ----
  case toPAR:
   if (lm && rm)
    {
     // element-wise: (a*b)/(a+b)
     if (left.mrows != right.mrows || left.mcols != right.mcols)
      {
       mxerror ("dimensions must match for // operator");
       return mrERROR;
      }
     int rows = left.mrows;
     int cols = left.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.mval[i] * right.mval[i]) / (left.mval[i] + right.mval[i]);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   else if (lm && rs)
    {
     int rows = left.mrows;
     int cols = left.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.mval[i] * right.fval) / (left.mval[i] + right.fval);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   else if (ls && rm)
    {
     int rows = right.mrows;
     int cols = right.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.fval * right.mval[i]) / (left.fval + right.mval[i]);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   return mrSKIP;

  // ---- comparison, bitwise, shifts — not defined for matrices ----
  case toEQ:
  case toNE:
   if (lm && rm)
    {
     if (left.mrows != right.mrows || left.mcols != right.mcols)
      {
       error ("Matrix dimensions must match for comparison");
       return mrERROR;
      }
     int n = left.mrows * left.mcols;
     bool equal = true;
     for (int i = 0; i < n; i++)
      if (left.mval[i] != right.mval[i])
       {
        equal = false;
        break;
       }
     res.tag  = tvINT;
     res.ival = (cop == toEQ) ? (equal ? 1 : 0) : (equal ? 0 : 1);
     return mrDONE;
    }
   else if (lm && rs)
    {
     // A==scalar: true if ALL elements equal scalar
     int n      = left.mrows * left.mcols;
     bool equal = true;
     for (int i = 0; i < n; i++)
      if (left.mval[i] != right.fval)
       {
        equal = false;
        break;
       }
     res.tag  = tvINT;
     res.ival = (cop == toEQ) ? (equal ? 1 : 0) : (equal ? 0 : 1);
     return mrDONE;
    }
   else if (ls && rm)
    {
     // scalar==A: same
     int n = right.mrows * right.mcols;
     bool equal = true;
     for (int i = 0; i < n; i++)
      if (right.mval[i] != left.fval)
       {
        equal = false;
        break;
       }
     res.tag  = tvINT;
     res.ival = (cop == toEQ) ? (equal ? 1 : 0) : (equal ? 0 : 1);
     return mrDONE;
    }
   return mrSKIP;
  case toGT:
  case toGE:
  case toLT:
  case toLE:
  case toAND:
  case toOR:
  case toXOR:
  case toASL:
  case toASR:
  case toLSR:
  case toSETOR:
  case toSETXOR:
  case toSETASL:
  case toSETASR:
  case toSETLSR:
   mxerror ("operation not defined");
   return mrERROR;

  default:
   break;
  }
 return mrSKIP;
}

// ---------------------------------------------------------------------------
// matrixuno — unary operations
// ---------------------------------------------------------------------------
t_mresult calculator::matrixuno (value &res, value &operand, t_operator cop)
{
 if (operand.tag != tvMATRIX) return mrSKIP;

 int rows = operand.mrows;
 int cols = operand.mcols;
 int n    = rows * cols;

 switch (cop)
  {
  // ---- unary minus ----
  case toMINUS:
   {
    float__t *mval = mxAlloc (rows, cols);
    if (!mval) return mrERROR;
    for (int i = 0; i < n; i++) mval[i] = -operand.mval[i];
    res.tag   = tvMATRIX;
    res.mrows = rows;
    res.mcols = cols;
    res.mval  = mval;
    return mrDONE;
   }

  // ---- unary plus — no-op ----
  case toPLUS:
   res = operand;
   return mrDONE;

  // ---- ~ transpose (for real matrix = conjugate transpose) ----
  case toCOM:
   {
    float__t *mval = mxAlloc (cols, rows); // note: transposed dimensions
    if (!mval) return mrERROR;
    for (int r = 0; r < rows; r++)
     for (int c = 0; c < cols; c++) mval[c * rows + r] = operand.mval[r * cols + c];
    res.tag   = tvMATRIX;
    res.mrows = cols; // transposed
    res.mcols = rows;
    res.mval  = mval;
    return mrDONE;
   }

  // ---- ! logical not — not defined ----
  case toNOT:
   {
    if (!mxInv (res, operand)) return mrERROR; 
    res.tag   = tvMATRIX;
    return mrDONE;
   }

  default:
   mxerror ("not defined for matrices");
   return mrERROR;
   break;
  }
 return mrSKIP;
}
#pragma endregion
//---------------------------------------------------------------------------

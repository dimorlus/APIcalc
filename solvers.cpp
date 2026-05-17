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

#pragma region SOLVERS

// Newton-Raphson solution of the equation solve(x(2x+2)-2, x:=0)
// expr -> x(2x+2)-2, x:=0
// Complex support
// #define _DAMPED_
bool calculator::Solve (const char *expr, t_symbol tag, float__t &re_res, float__t &im_res)
{
 if (!expr || !*expr)
  {
   errorf (0, "empty expression");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 char sexpr[STRBUF], svar[STRBUF], nvar[MAXOP];

 if (!Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0))
  {
   errorf (pos, "solve: invalid expression");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 child->evaluate_f (svar);
 if (child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   re_res = qnan;
   im_res = qnan;
   return false;
  }
 strcpy (nvar, (char *)child->get_last_var ());
 float__t xr = child->get_re_res ();
 float__t xi = child->get_im_res (); // 0 if real initial approximation

 if (tag == tsCALC)
  {
   child->addfvar (nvar, xr, xi);
   child->evaluate_f (sexpr);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     re_res = qnan;
     im_res = qnan;
     return false;
    }
   re_res = child->get_re_res ();
   im_res = child->get_im_res ();
   fflags |= child->isfflags ();
   delete child;
   return true;
  }

#ifdef _float128_
 const float__t tol   = 1e-28Q;
 const float__t delta = 1.5e-17Q;
#else
 const float__t tol   = 1e-12L;
 const float__t delta = 1.5e-10L;
#endif
 const int maxIter = 100;
 bool converged    = false;

 for (int i = 0; i < maxIter; i++)
  {
   // f(z)
   child->addfvar (nvar, xr, xi);
   child->evaluate_f (sexpr);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     re_res = qnan;
     im_res = qnan;
     return false;
    }
   float__t fr = child->get_re_res ();
   float__t fi = child->get_im_res ();

   // |f(z)|
   float__t fabs_f = Sqrt (fr * fr + fi * fi);
   if (fabs_f < tol)
    {
     converged = true;
     break;
    }

    // step d = max(|z|, 1) * delta
#ifdef _float128_
   float__t d = fmaxq (sqrtq (xr * xr + xi * xi), 1.0Q) * delta;
#else
#ifdef __BORLANDC__
   float__t ax = Sqrt (xr * xr + xi * xi);
   float__t d  = ((ax > 1.0L) ? ax : 1.0L) * delta;
#else
   float__t d = fmaxl (Sqrt (xr * xr + xi * xi), 1.0L) * delta;
#endif
#endif
   // f(z+d) and f(z-d)  on the real axis
   child->addfvar (nvar, xr + d, xi);
   child->evaluate_f (sexpr);
   float__t fpr_r = child->get_re_res (), fpr_i = child->get_im_res ();

   child->addfvar (nvar, xr - d, xi);
   child->evaluate_f (sexpr);
   float__t fmr_r = child->get_re_res (), fmr_i = child->get_im_res ();

   // f'(z) ≈ (f(z+d) - f(z-d)) / (2d)
   float__t fp_r = (fpr_r - fmr_r) / ((float__t)2.0 * d);
   float__t fp_i = (fpr_i - fmr_i) / ((float__t)2.0 * d);

   float__t fp2 = fp_r * fp_r + fp_i * fp_i;
   if (fp2 < tol * tol)
    {
     xr += d * (float__t)1000.0;
     continue;
    }

   // Newton step: z -= f(z)/f'(z)
   float__t step_r = (fr * fp_r + fi * fp_i) / fp2;
   float__t step_i = (fi * fp_r - fr * fp_i) / fp2;
   float__t xr_new = xr - step_r;
   float__t xi_new = xi - step_i;

   if (isnan (xr_new) || isnan (xi_new))
    {
     errorf (pos, "Solution diverged");
     delete child;
     re_res = qnan;
     im_res = qnan;
     return false;
    }

#ifdef _DAMPED_
   // damped Newton:
   float__t step_abs = Sqrt (step_r * step_r + step_i * step_i);
   float__t z_abs    = Sqrt (xr * xr + xi * xi);
   float__t max_step = ((float__t)1.0 + z_abs) * (float__t)10.0; // step limit
   if (step_abs > max_step)
    {
     float__t scale = max_step / step_abs;
     step_r *= scale;
     step_i *= scale;
    }
   xr = xr - step_r;
   xi = xi - step_i;
#else
   float__t step_abs = Sqrt (step_r * step_r + step_i * step_i);
   float__t z_abs    = Sqrt (xr * xr + xi * xi);
   if (step_abs < tol * ((float__t)1.0 + z_abs))
    {
     xr        = xr_new;
     xi        = xi_new;
     converged = true;
     break;
    }
   xr = xr_new;
   xi = xi_new;
#endif
  }

 fflags |= child->isfflags ();
 delete child;

 if (!converged)
  {
   errorf (pos, "No solution found");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 re_res = xr;
 im_res = xi;
 return true;
}

// Gauss-Kronrod G7/K15 adaptive quadrature
// C++98 compatible (BCB6 / VS2022)
//
// K15 nodes (positive half, index 0 = center node = 0.0)
// G7 uses nodes at indices 0, 2, 4, 6  (every other K15 node)

static const float__t GK_NODES[8] = {
 0.0L,
 0.20778495500789846760L,
 0.40584515137739716691L,
 0.58608723546769113029L,
 0.74153118559939443986L,
 0.86486442335976907279L,
 0.94910791234275852453L,
 0.99145537112081263921L,
};

// K15 weights: index i corresponds to nodes +-GK_NODES[i]
// (index 0 has no symmetry — centre point)
static const float__t K15_WEIGHTS[8] = {
 0.20948214108472782801L, 0.20443294007529889241L, 0.19035057806478540991L, 0.16900472663926790283L,
 0.14065325971552591875L, 0.10479001032224928880L, 0.06309209262997855329L, 0.02293532201052922497L,
};

// G7 weights: 4 values for nodes at GK_NODES[0,2,4,6]
static const float__t G7_WEIGHTS[4] = {
 0.41795918367346938776L,
 0.38183005050511894495L,
 0.27970539148927664160L,
 0.12948496616886732340L,
};

// G7 node indices into GK_NODES[]
static const int G7_IDX[4] = { 0, 2, 4, 6 };

// ---------------------------------------------------------------------------

// Evaluate f(x) = sexpr with variable svar set to x in child calculator
// Returns qnan on any error
float__t calculator::gkEval (calculator *pCalc, char *sexpr, const char *svar, float__t x)
{
 pCalc->addfvar (svar, x);
 float__t val = pCalc->evaluate_f (sexpr);
 if ((isnan (val) || isinf (val)) && pCalc->errt () == teMath) return (float__t)0.0L;

 if (pCalc->err[0]) return qnan;
 return val;
}

// Single G7/K15 panel on [a, b], no recursion
GKResult calculator::gkPanel (calculator *pCalc, char *sexpr, const char *svar, float__t a,
                              float__t b)
{
 GKResult res    = { 0.0L, 0.0L, true };
 float__t center = (a + b) / 2.0L;
 float__t half   = (b - a) / 2.0L;

 // f values at all 15 points: fL[i] = f(center - half*node[i])
 //                            fR[i] = f(center + half*node[i])
 // index 0: fL[0] == fR[0] == f(center)
 float__t fL[8], fR[8];
 fL[0] = fR[0] = gkEval (pCalc, sexpr, svar, center);
 if (isnan (fL[0]))
  {
   res.ok = false;
   return res;
  }

 for (int i = 1; i < 8; i++)
  {
   fL[i] = gkEval (pCalc, sexpr, svar, center - half * GK_NODES[i]);
   if (isnan (fL[i]))
    {
     res.ok = false;
     return res;
    }
   fR[i] = gkEval (pCalc, sexpr, svar, center + half * GK_NODES[i]);
   if (isnan (fR[i]))
    {
     res.ok = false;
     return res;
    }
  }

 // K15: sum over all 8 node pairs (index 0 counted once)
 float__t k15 = K15_WEIGHTS[0] * fL[0];
 for (int i = 1; i < 8; i++) k15 += K15_WEIGHTS[i] * (fL[i] + fR[i]);
 k15 *= half;

 // G7: sum over node indices 0, 2, 4, 6
 float__t g7 = G7_WEIGHTS[0] * fL[0];
 for (int i = 1; i < 4; i++)
  {
   int idx = G7_IDX[i];
   g7 += G7_WEIGHTS[i] * (fL[idx] + fR[idx]);
  }
 g7 *= half;

 res.value = k15;
 res.error = fabsl (k15 - g7);
 return res;
}

// Adaptive G7/K15: recursively subdivide until error < tol or maxDepth reached
GKResult calculator::gkAdaptive (calculator *pCalc, char *sexpr, const char *svar, float__t a,
                                 float__t b,
                                 float__t tol, // not divided!
                                 int depth, int maxDepth, int &callCount, int maxCalls)
{
 if (++callCount > maxCalls) // hard stop
  {
   GKResult res = gkPanel (pCalc, sexpr, svar, a, b);
   res.ok       = true;
   return res;
  }

 GKResult res = gkPanel (pCalc, sexpr, svar, a, b);
 if (!res.ok) return res;

 if (res.error <= tol || depth >= maxDepth) return res;

 float__t mid = (a + b) / 2.0L;

 GKResult left
     = gkAdaptive (pCalc, sexpr, svar, a, mid, tol, depth + 1, maxDepth, callCount, maxCalls);
 GKResult right
     = gkAdaptive (pCalc, sexpr, svar, mid, b, tol, depth + 1, maxDepth, callCount, maxCalls);

 if (!left.ok || !right.ok)
  {
   GKResult bad = { qnan, qnan, false };
   return bad;
  }

 GKResult combined;
 combined.value = left.value + right.value;
 combined.error = left.error + right.error;
 combined.ok    = true;
 return combined;
}


bool calculator::For (const char *expr, value &res)
{
 if (expr && *expr)
  {
   char sexpr[STRBUF];
   char sfrom[MAXOP];
   char sto[MAXOP];
   char svar[STRBUF];

   float__t vfrom  = qnan;
   float__t vto    = qnan;
   float__t fvx    = qnan;
   float__t result = 0;
   int callCount   = 0;

   if (!Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0))
    {
     result_fval = qnan;
     return false;
    }

   calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
   if (!child)
    {
     errorf (pos, "Out of memory");
     result_fval = qnan;
     return false;
    }

   vfrom = child->evaluate_f (sfrom);
   if (isnan (vfrom) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
   vto = child->evaluate_f (sto);
   if (isnan (vto) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
   {
    float__t fvx            = (float__t)0.0L;
    uint64_t init_ms        = GetTickCount64 ();
    uint64_t last_gui_check = 0;

    if (vfrom > vto)
     {
      do
       {
        if (check_break (init_ms, last_gui_check) != brNONE)
         {
          delete child;
          result_fval = qnan;
          return false;
         }

        child->addfvar (svar, vfrom);
        fvx = child->evaluate_f (sexpr); // evaluate the function for
                                         // the syntax check before starting the integration

        if (isnan (fvx) || child->err[0])
         {
          errorf (pos, "%s", child->err);
          delete child;
          result_fval = qnan;
          return false;
         }
        vfrom -= (float__t)1.0L; // increment by 1 for summation, this can be modified to support
                                 // different step sizes
       }
      while (vfrom >= vto);
     }
    else
     {
      do
       {
        if (check_break (init_ms, last_gui_check) != brNONE)
         {
          delete child;
          result_fval = qnan;
          return qnan;
         }

        child->addfvar (svar, vfrom);
        fvx = child->evaluate_f (sexpr); // evaluate the function for
                                         // the syntax check before starting the integration

        if (isnan (fvx) || child->err[0])
         {
          errorf (pos, "%s", child->err);
          delete child;
          result_fval = qnan;
          return false;
         }
        vfrom += (float__t)1.0L; // increment by 1 for summation, this can be modified to support
                                 // different step sizes
       }
      while (vfrom <= vto);
     }
   }

   res.tag = child->get_res_tag ();
   if (res.tag == tvMATRIX)
    {
     res.fval       = child->get_re_res ();
     res.ival       = child->get_int_res ();
     res.imval      = child->get_im_res ();
     res.sval       = dupString (get_str_res ());
     mxresult_t mxr = child->get_mx_res ();
     res.mcols      = mxr.cols;
     res.mrows      = mxr.rows;
     int msize      = mxr.rows * mxr.cols * sizeof (float__t);
     if (msize)
      {
       float__t *new_mval = (float__t *)sf_alloc (msize);
       if (new_mval)
        {
         memcpy (new_mval, mxr.mval, msize);
         res.mval = new_mval;
        }
       else
        {
         errorf (res.pos, "Out of memory");
         delete child;
         result_fval = qnan;
         return false;
        }
      }
    }
   else
    {
     res.fval  = child->get_re_res ();
     res.ival  = child->get_int_res ();
     res.imval = child->get_im_res ();
     res.sval  = dupString (get_str_res ());
    }

   fflags |= child->isfflags ();
   delete child;
   return true;
  }
 return false;
}



// integr(expr(x), from, to, x) integr(sin(x)/x, 0.001, pi, x)
// sum(expr(x), from, to, x) 
// expr -> sin(x)/x, x
float__t calculator::Integr (const char *expr, t_symbol tag)
{
 if (expr && *expr)
  {
   char sexpr[STRBUF];
   char sfrom[MAXOP];
   char sto[MAXOP];
   char svar[STRBUF];

   float__t vfrom = qnan;
   float__t vto   = qnan;
   float__t fvx   = qnan;
   float__t result = 0;
   int callCount   = 0;

   if (!Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0))
    {
     return result_fval = qnan;
    }

   calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
   if (!child)
    {
     errorf (pos, "Out of memory");
     return result_fval = qnan;
    }

   vfrom = child->evaluate_f (sfrom);
   if (isnan (vfrom) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }
   vto = child->evaluate_f (sto);
   if (isnan (vto) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }

   if (tag == tsINTEGR)
    {
     {
      child->addfvar (svar, vfrom);
      float__t fvx = child->evaluate_f (sexpr); // evaluate the function for
                                           // the syntax check before starting the integration
      if (isnan (fvx) && child->errt () == teMath) 
       {
        fvx = 0; // treat math error at the starting point as zero (e.g. sin(x)/x at x=0)
       }
      else
      if (isnan (fvx) || child->err[0])
       {
        errorf (pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
       }
     }

     GKResult gkresult = gkAdaptive (child,
                                     sexpr,     // expression
                                     svar,      // variable name ("x")
                                     vfrom,     // lower limit
                                     vto,       // upper limit
                                     1e-10L,    // tolerance
                                     0,         // initial depth
                                     20,        // maximum depth
                                     callCount, // call count
                                     1000);     // maximum calls
     if (!gkresult.ok)
      {
       errorf (pos, "Integration failed");
       delete child;
       return result_fval = qnan;
      }
     result = gkresult.value;
    }
   else if (tag == tsSUM)
    {
     uint64_t init_ms = GetTickCount64 ();
     uint64_t last_gui_check = 0;

     float__t fvx = 0.0;
     if (vfrom > vto)
      {
       do
        {
         if (check_break (init_ms, last_gui_check) != brNONE)
          {
           delete child;
           return result_fval = qnan;
          }

         child->addfvar (svar, vfrom);
         fvx += child->evaluate_f (sexpr); 
         if ((isnan (fvx) || isinf (fvx)) && child->errt () == teMath)
          {
           fvx = 0; // treat math error at the starting point as zero (e.g. sin(x)/x at x=0)
          }
         else
         if (isnan (fvx) || child->err[0])
          {
           errorf (pos, "%s", child->err);
           delete child;
           return result_fval = qnan;
          }
         vfrom -= 1.0; // increment by 1 for summation, this can be modified to support different
                       // step sizes
        }
       while (vfrom >= vto);
      }
     else
      {
       do
        {
         if (check_break (init_ms, last_gui_check) != brNONE)
          {
           delete child;
           return result_fval = qnan;
          }

         child->addfvar (svar, vfrom);
         fvx += child->evaluate_f (sexpr); 
         if ((isnan (fvx) || isinf (fvx)) && child->errt () == teMath)
          {
           fvx = 0; // treat math error at the starting point as zero (e.g. sin(x)/x at x=0)
          }
         else
         if (isnan (fvx) || child->err[0])
          {
           errorf (pos, "%s", child->err);
           delete child;
           return result_fval = qnan;
          }
         vfrom += 1.0; // increment by 1 for summation, this can be modified to support different
                       // step sizes
        }
       while (vfrom <= vto);
      }
     result = fvx;
    }
   else
    {
     errorf (pos, "Unknown tag");
     result = qnan;
    }
   fflags |= child->isfflags ();
   delete child;
   return result;
  }
 return result_fval = qnan; // placeholder for actual integration result
}

// Numerical differentiation using central difference
// diff(expr(x), point, x) diff(sin(x)/x, 0.01, x)
float__t calculator::Diff (const char *expr)
{
 if (expr && *expr)
  {
     char sexpr[STRBUF];
     char sfrom[MAXOP];
     char svar[STRBUF];

     float__t x  = qnan;
     float__t fvx    = qnan;
     float__t result = 0;

     if (!Split (expr, sexpr, STRBUF, sfrom, MAXOP,  svar, STRBUF, nullptr, 0))
      {
       return result_fval = qnan;
      }

     calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
     if (!child)
      {
       errorf (pos, "Out of memory");
       return result_fval = qnan;
      }

     x = child->evaluate_f (sfrom);
     if (isnan (x) || child->err[0])
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     // Numerical derivative (central difference)
#ifdef _float128_
     float__t delta = fmaxq (fabsq (x), 1.0Q) * 1.5e-17Q;
#elif defined(__BORLANDC__)
     float__t ax    = fabsl (x);
     float__t delta = ((ax > 1.0L) ? ax : 1.0L) * 1.5e-10L;
#else
     float__t delta = fmaxl (fabsl (x), 1.0L) * 1.5e-10L;
#endif
     child->addfvar (svar, x + delta);
     float__t fxp = child->evaluate_f (sexpr);
     if (isnan (fxp) || child->err[0])
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     child->addfvar (svar, x - delta);
     float__t fxm = child->evaluate_f (sexpr);
     if (isnan (fxm) || child->err[0])
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     float__t fp = (fxp - fxm) / ((float__t)2.0 * delta);

     fflags |= child->isfflags ();
     delete child;
     return fp;
    }
  return result_fval = qnan; 
}

// Find extremum using Golden Section Search (derivative-free)
// extr(expr(x), initial, x) -> extr(x^(1/x), 2, x)
float__t calculator::Extremum (const char *expr)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return result_fval = qnan;
  }

 char sexpr[STRBUF], svar[STRBUF], sinit[MAXOP];

 if (!Split (expr, sexpr, STRBUF, sinit, MAXOP, svar, STRBUF, NULL, 0))
  {
   errorf (pos, "extremum: invalid expression");
   return result_fval = qnan;
  }

 calculator *child = new calculator (scfg | SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   return result_fval = qnan;
  }

 float__t x0 = child->evaluate_f (sinit);
 if (isnan (x0) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

#ifdef _float128_
 const float__t tol     = 1e-32Q;
 const float__t phi_inv = 0.6180339887498948482045868343656Q;
#else
 const float__t tol     = 1e-15L;
 const float__t phi_inv = 0.6180339887498948482045868343656L;
#endif

 // Helper: evaluate function
 //float__t eval_at_x;
 bool eval_ok;

#define EVAL_F(xval, result)                                                                       \
 do                                                                                                \
  {                                                                                                \
   eval_ok = true;                                                                                 \
   child->addfvar (svar, xval);                                                                    \
   result = child->evaluate_f (sexpr);                                                             \
   if (isnan (result) && child->errt () != teMath)                                                 \
    {                                                                                              \
     if (child->err[0]) eval_ok = false;                                                           \
    }                                                                                              \
   if (isnan (result)) result = 0;                                                                 \
  }                                                                                                \
 while (0)

 // Step 1: Initial bracket around x0
 float__t step = (float__t)0.5;
 float__t xa   = x0 - step;
 float__t xb   = x0;
 float__t xc   = x0 + step;

 float__t fa, fb, fc;
 EVAL_F (xa, fa);
 if (!eval_ok)
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 EVAL_F (xb, fb);
 if (!eval_ok)
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 EVAL_F (xc, fc);
 if (!eval_ok)
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 // Determine if seeking max or min
 bool seeking_max = true;
 if ((fb < fa && fb < fc)) seeking_max = false;

 // Step 2: Expand bracket until we have proper containment
 // For max: need fb > fa and fb > fc
 // For min: need fb < fa and fb < fc
 int i;
 for (i = 0; i < 30; i++)
  {
   bool bracketed;
   if (seeking_max)
    bracketed = (fb > fa && fb > fc);
   else
    bracketed = (fb < fa && fb < fc);

   if (bracketed) break;

   // Move in direction of improvement
   if (seeking_max)
    {
     // Move towards higher value
     if (fa > fc)
      {
       // Shift right side to center, expand left
       xc = xb;
       fc = fb;
       xb = xa;
       fb = fa;
       xa = xb - step;
       EVAL_F (xa, fa);
      }
     else
      {
       // Shift left side to center, expand right
       xa = xb;
       fa = fb;
       xb = xc;
       fb = fc;
       xc = xb + step;
       EVAL_F (xc, fc);
      }
    }
   else
    {
     // Move towards lower value
     if (fa < fc)
      {
       xc = xb;
       fc = fb;
       xb = xa;
       fb = fa;
       xa = xb - step;
       EVAL_F (xa, fa);
      }
     else
      {
       xa = xb;
       fa = fb;
       xb = xc;
       fb = fc;
       xc = xb + step;
       EVAL_F (xc, fc);
      }
    }

   if (!eval_ok)
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }

   step *= (float__t)1.2;
  }

 // Now [xa, xc] brackets the extremum with xb inside
 // Use Golden Section Search on [xa, xc]
 float__t a = xa;
 float__t b = xc;
 float__t x1, x2, f1, f2;

 // Initial golden points
 x1 = b - (b - a) * phi_inv;
 x2 = a + (b - a) * phi_inv;

 EVAL_F (x1, f1);
 if (!eval_ok)
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 EVAL_F (x2, f2);
 if (!eval_ok)
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 // Golden section iterations
 for (i = 0; i < 200; i++)
  {
   if (fabsl (b - a) < tol * ((float__t)1.0 + fabsl (a) + fabsl (b))) break;

   bool condition = seeking_max ? (f1 > f2) : (f1 < f2);

   if (condition)
    {
     // Narrow to [a, x2]
     b  = x2;
     x2 = x1;
     f2 = f1;
     x1 = b - (b - a) * phi_inv;
     EVAL_F (x1, f1);
    }
   else
    {
     // Narrow to [x1, b]
     a  = x1;
     x1 = x2;
     f1 = f2;
     x2 = a + (b - a) * phi_inv;
     EVAL_F (x2, f2);
    }

   if (!eval_ok)
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }
  }

 float__t x_result = (a + b) * (float__t)0.5;

 // Step 4: Brent's parabolic interpolation polish
 float__t x = x_result;
 float__t w = x_result;
 float__t v = x_result;
 float__t fx, fw, fv;

 EVAL_F (x, fx);
 fw = fx;
 fv = fx;

 float__t d_brent = 0;
 float__t e_brent = 0;

 for (i = 0; i < 50; i++)
  {
   float__t xm   = (float__t)0.5 * (a + b);
   float__t tol1 = tol * fabsl (x) + tol;
   float__t tol2 = (float__t)2.0 * tol1;

   if (fabsl (x - xm) <= tol2 - (float__t)0.5 * (b - a)) break;

   bool do_golden = true;

   if (fabsl (e_brent) > tol1)
    {
     // Try parabolic fit
     float__t r = (x - w) * (fx - fv);
     float__t q = (x - v) * (fx - fw);
     float__t p = (x - v) * q - (x - w) * r;
     q          = (float__t)2.0 * (q - r);

     if (q > 0)
      p = -p;
     else
      q = -q;

     float__t e_tmp = e_brent;
     e_brent        = d_brent;

     if (fabsl (p) < fabsl ((float__t)0.5 * q * e_tmp) && p > q * (a - x) && p < q * (b - x))
      {
       d_brent    = p / q;
       float__t u = x + d_brent;

       if ((u - a) < tol2 || (b - u) < tol2) d_brent = (xm > x) ? tol1 : -tol1;

       do_golden = false;
      }
    }

   if (do_golden)
    {
     e_brent = (x >= xm) ? a - x : b - x;
     d_brent = phi_inv * e_brent;
    }

   float__t u = x + ((fabsl (d_brent) >= tol1) ? d_brent : ((d_brent > 0) ? tol1 : -tol1));
   float__t fu;
   EVAL_F (u, fu);

   bool better = seeking_max ? (fu > fx) : (fu < fx);

   if (better)
    {
     if (u >= x)
      a = x;
     else
      b = x;

     v  = w;
     fv = fw;
     w  = x;
     fw = fx;
     x  = u;
     fx = fu;
    }
   else
    {
     if (u < x)
      a = u;
     else
      b = u;

     bool w_better = seeking_max ? (fu > fw || w == x) : (fu < fw || w == x);
     bool v_better = seeking_max ? (fu > fv || v == x || v == w) : (fu < fv || v == x || v == w);

     if (w_better)
      {
       v  = w;
       fv = fw;
       w  = u;
       fw = fu;
      }
     else if (v_better)
      {
       v  = u;
       fv = fu;
      }
    }
  }

#undef EVAL_F

 fflags |= child->isfflags ();
 delete child;
 return x;
}
#pragma endregion

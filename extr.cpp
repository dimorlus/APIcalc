// Find extremum using gradient descent + Newton's method
// extremum(expr(x), initial, x) -> extremum(x^2-4*x+3, 2, x)
float__t calculator::Extremum(const char *expr)
{
    if (!expr || !*expr)
    {
        errorf(pos, "empty expression");
        return result_fval = qnan;
    }

    char sexpr[STRBUF], svar[STRBUF], sinit[MAXOP];
    
    if (!Split(expr, sexpr, STRBUF, sinit, MAXOP, svar, STRBUF, nullptr, 0))
    {
        errorf(pos, "extremum: invalid expression");
        return result_fval = qnan;
    }

    calculator *child = new calculator(scfg|SNAN, hash_table, 
                                       (MASK_DEFAULT | MASK_VARIABLE), deep);
    if (!child)
    {
        errorf(pos, "Out of memory");
        return result_fval = qnan;
    }

    float__t x = child->evaluate_f(sinit);
    if (isnan(x) || child->err[0])
    {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }

#ifdef _float128_
    const float__t tol = 1e-28Q;
    const float__t delta = 1.5e-17Q;
#else
    const float__t tol = 1e-12L;
    const float__t delta = 1.5e-10L;
#endif
    const int maxIter = 100;

    // Newton's method on f'(x) = 0
    for (int i = 0; i < maxIter; i++)
    {
        // First derivative (central difference)
#ifdef _float128_
        float__t d = fmaxq(fabsq(x), 1.0Q) * delta;
#elif defined(__BORLANDC__)
        float__t ax = fabsl(x);
        float__t d = ((ax > 1.0L) ? ax : 1.0L) * delta;
#else
        float__t d = fmaxl(fabsl(x), 1.0L) * delta;
#endif

        child->addfvar(svar, x + d);
        float__t fxp = child->evaluate_f(sexpr);
        if (isnan(fxp) || child->err[0])
        {
            errorf(pos, "%s", child->err);
            delete child;
            return result_fval = qnan;
        }

        child->addfvar(svar, x - d);
        float__t fxm = child->evaluate_f(sexpr);
        if (isnan(fxm) || child->err[0])
        {
            errorf(pos, "%s", child->err);
            delete child;
            return result_fval = qnan;
        }

        float__t fp = (fxp - fxm) / ((float__t)2.0 * d);  // f'(x)

        // Check convergence
        if (fabsl(fp) < tol)
        {
            fflags |= child->isfflags();
            delete child;
            return x;  // Found extremum
        }

        // Second derivative for Newton step
        child->addfvar(svar, x);
        float__t fx = child->evaluate_f(sexpr);
        float__t fpp = (fxp - (float__t)2.0 * fx + fxm) / (d * d);  // f''(x)

        if (fabsl(fpp) < tol)
        {
            // Fallback to gradient descent
            x -= fp * (float__t)0.01;
        }
        else
        {
            // Newton step: x -= f'(x) / f''(x)
            x -= fp / fpp;
        }

        if (isnan(x))
        {
            errorf(pos, "Extremum search diverged");
            delete child;
            return result_fval = qnan;
        }
    }

    errorf(pos, "No extremum found");
    delete child;
    return result_fval = qnan;
}

#ifdef _comment_
// Find extremum using gradient descent + Newton's method
// extremum(expr(x), initial, x) -> extremum(x^2-4*x+3, 2, x)
float__t calculator::Extremum (const char *expr)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return result_fval = qnan;
  }

 char sexpr[STRBUF], svar[STRBUF], sinit[MAXOP];

 if (!Split (expr, sexpr, STRBUF, sinit, MAXOP, svar, STRBUF, nullptr, 0))
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

 float__t x = child->evaluate_f (sinit);
 if (isnan (x) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

#ifdef _float128_
 const float__t tol   = 1e-28Q;
 const float__t delta = 1.5e-17Q;
#else
 const float__t tol   = 1e-12L;
 const float__t delta = 1.5e-10L;
#endif
 const int maxIter = 100;

 // Newton's method on f'(x) = 0
 for (int i = 0; i < maxIter; i++)
  {
   // First derivative (central difference)
#ifdef _float128_
   float__t d = fmaxq (fabsq (x), 1.0Q) * delta;
#elif defined(__BORLANDC__)
   float__t ax = fabsl (x);
   float__t d  = ((ax > 1.0L) ? ax : 1.0L) * delta;
#else
   float__t d = fmaxl (fabsl (x), 1.0L) * delta;
#endif

   child->addfvar (svar, x + d);
   float__t fxp = child->evaluate_f (sexpr);
   if (isnan (fxp) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }

   child->addfvar (svar, x - d);
   float__t fxm = child->evaluate_f (sexpr);
   if (isnan (fxm) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }

   float__t fp = (fxp - fxm) / ((float__t)2.0 * d); // f'(x)

   // Check convergence
   if (fabsl (fp) < tol)
    {
     fflags |= child->isfflags ();
     delete child;
     return x; // Found extremum
    }

   // Second derivative for Newton step
   child->addfvar (svar, x);
   float__t fx  = child->evaluate_f (sexpr);
   float__t fpp = (fxp - (float__t)2.0 * fx + fxm) / (d * d); // f''(x)

   if (fabsl (fpp) < tol)
    {
     // Fallback to gradient descent
     x -= fp * (float__t)0.01;
    }
   else
    {
     // Newton step: x -= f'(x) / f''(x)
     x -= fp / fpp;
    }

   if (isnan (x))
    {
     errorf (pos, "Extremum search diverged");
     delete child;
     return result_fval = qnan;
    }
  }

 errorf (pos, "No extremum found");
 delete child;
 return result_fval = qnan;
}
//#endif 
// Find extremum using robust combined method
// extr(expr(x), initial, x) -> extr(x^(1/x), 2, x)
float__t calculator::Extremum (const char *expr)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return result_fval = qnan;
  }

 char sexpr[STRBUF], svar[STRBUF], sinit[MAXOP];

 if (!Split (expr, sexpr, STRBUF, sinit, MAXOP, svar, STRBUF, nullptr, 0))
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
 const float__t tol   = 1e-28Q;
 const float__t delta = 1.5e-17Q;
#else
 const float__t tol   = 1e-12L;
 const float__t delta = 1.5e-10L;
#endif
 const int maxIter = 200;

 // Lambda to compute f'(x)
 auto computeDerivative = [&] (float__t x, float__t &fp) -> bool {
#ifdef _float128_
  float__t d = fmaxq (fabsq (x), 1.0Q) * delta;
#elif defined(__BORLANDC__)
  float__t ax = fabsl (x);
  float__t d  = ((ax > 1.0L) ? ax : 1.0L) * delta;
#else
  float__t d = fmaxl (fabsl (x), 1.0L) * delta;
#endif

  child->addfvar (svar, x + d);
  float__t fxp = child->evaluate_f (sexpr);
  if (isnan (fxp) && child->errt () != teMath)
   {
    if (child->err[0]) return false;
   }
  if (isnan (fxp)) fxp = 0;

  child->addfvar (svar, x - d);
  float__t fxm = child->evaluate_f (sexpr);
  if (isnan (fxm) && child->errt () != teMath)
   {
    if (child->err[0]) return false;
   }
  if (isnan (fxm)) fxm = 0;

  fp = (fxp - fxm) / ((float__t)2.0 * d);
  return true;
 };

 // Step 1: Bracket the root of f'(x) = 0
 float__t x = x0;
 float__t fp0;
 if (!computeDerivative (x0, fp0))
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 // Try to find bracket by expanding search
 float__t step = fabsl (x0) * (float__t)0.1;
 if (step < (float__t)0.1) step = (float__t)0.1;

 float__t xa = x0 - step;
 float__t xb = x0 + step;
 float__t fpa, fpb;

 // Expand bracket until we find sign change in f'(x)
 bool bracketed = false;
 for (int i = 0; i < 10; i++)
  {
   if (!computeDerivative (xa, fpa) || !computeDerivative (xb, fpb))
    {
     step *= (float__t)0.5;
     xa = x0 - step;
     xb = x0 + step;
     continue;
    }

   if (fpa * fpb < 0)
    {
     bracketed = true;
     break;
    }

   // Expand in direction of decreasing |f'|
   if (fabsl (fpa) < fabsl (fpb))
    {
     xb  = x0;
     fpb = fp0;
     x0  = xa;
     fp0 = fpa;
     xa  = x0 - step;
    }
   else
    {
     xa  = x0;
     fpa = fp0;
     x0  = xb;
     fp0 = fpb;
     xb  = x0 + step;
    }
   step *= (float__t)1.5;
  }

 if (!bracketed)
  {
   // Fallback to gradient descent from x0
   x                      = x0;
   float__t learning_rate = (float__t)0.01;

   for (int i = 0; i < maxIter; i++)
    {
     float__t fp;
     if (!computeDerivative (x, fp))
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     if (fabsl (fp) < tol)
      {
       fflags |= child->isfflags ();
       delete child;
       return x;
      }

     // Adaptive learning rate
     float__t x_new = x - learning_rate * fp;
     float__t fp_new;
     if (!computeDerivative (x_new, fp_new))
      {
       learning_rate *= (float__t)0.5;
       continue;
      }

     if (fabsl (fp_new) < fabsl (fp))
      {
       x = x_new;
       learning_rate *= (float__t)1.1;
      }
     else
      {
       learning_rate *= (float__t)0.5;
      }

     if (learning_rate < tol * (float__t)0.001) break;
    }

   errorf (pos, "No extremum found");
   delete child;
   return result_fval = qnan;
  }

 // Step 2: Brent's method on [xa, xb] to find f'(x) = 0
 float__t xc      = xb;
 float__t fpc     = fpb;
 float__t d_brent = 0, e_brent = 0;

 for (int i = 0; i < maxIter; i++)
  {
   if (fabsl (fpa) < fabsl (fpb))
    {
     float__t tmp = xa;
     xa           = xb;
     xb           = tmp;
     tmp          = fpa;
     fpa          = fpb;
     fpb          = tmp;
    }

   float__t tol1 = (float__t)2.0 * delta * fabsl (xb) + (float__t)0.5 * tol;
   float__t xm   = (float__t)0.5 * (xa - xb);

   if (fabsl (xm) <= tol1 || fabsl (fpb) < tol)
    {
     fflags |= child->isfflags ();
     delete child;
     return xb;
    }

   if (fabsl (e_brent) >= tol1 && fabsl (fpc) > fabsl (fpb))
    {
     float__t s, p, q, r;
     s = fpb / fpc;
     if (fabsl (xa - xc) < tol)
      {
       p = (float__t)2.0 * xm * s;
       q = (float__t)1.0 - s;
      }
     else
      {
       q = fpc / fpa;
       r = fpb / fpa;
       p = s * ((float__t)2.0 * xm * q * (q - r) - (xb - xc) * (r - (float__t)1.0));
       q = (q - (float__t)1.0) * (r - (float__t)1.0) * (s - (float__t)1.0);
      }

     if (p > 0) q = -q;
     p = fabsl (p);

     float__t min1 = (float__t)3.0 * xm * q - fabsl (tol1 * q);
     float__t min2 = fabsl (e_brent * q);
     if ((float__t)2.0 * p < (min1 < min2 ? min1 : min2))
      {
       e_brent = d_brent;
       d_brent = p / q;
      }
     else
      {
       d_brent = xm;
       e_brent = d_brent;
      }
    }
   else
    {
     d_brent = xm;
     e_brent = d_brent;
    }

   xa  = xb;
   fpa = fpb;

   if (fabsl (d_brent) > tol1)
    xb += d_brent;
   else
    xb += (xm > 0 ? tol1 : -tol1);

   if (!computeDerivative (xb, fpb))
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }
  }

 errorf (pos, "No extremum found (max iterations)");
 delete child;
 return result_fval = qnan;
}

#endif
#ifdef _comment_
// Helper: Compute f'(x) using 5-point stencil (more accurate)
bool calculator::computeDerivative (calculator *child, char *sexpr, char *svar, float__t x,
                                    float__t &fp, float__t dx)
{
#ifdef _float128_
 float__t d = fmaxq (fabsq (x), 1.0Q) * dx;
#elif defined(__BORLANDC__)
 float__t ax = fabsl (x);
 float__t d  = ((ax > 1.0L) ? ax : 1.0L) * dx;
#else
 float__t d = fmaxl (fabsl (x), 1.0L) * dx;
#endif

 // 5-point stencil: f'(x) ≈ [-f(x+2h) + 8f(x+h) - 8f(x-h) + f(x-2h)] / (12h)
 child->addfvar (svar, x + (float__t)2.0 * d);
 float__t f2p = child->evaluate_f (sexpr);
 if (isnan (f2p) && child->errt () != teMath)
  {
   if (child->err[0]) return false;
  }
 if (isnan (f2p)) f2p = 0;

 child->addfvar (svar, x + d);
 float__t fp1 = child->evaluate_f (sexpr);
 if (isnan (fp1) && child->errt () != teMath)
  {
   if (child->err[0]) return false;
  }
 if (isnan (fp1)) fp1 = 0;

 child->addfvar (svar, x - d);
 float__t fm1 = child->evaluate_f (sexpr);
 if (isnan (fm1) && child->errt () != teMath)
  {
   if (child->err[0]) return false;
  }
 if (isnan (fm1)) fm1 = 0;

 child->addfvar (svar, x - (float__t)2.0 * d);
 float__t f2m = child->evaluate_f (sexpr);
 if (isnan (f2m) && child->errt () != teMath)
  {
   if (child->err[0]) return false;
  }
 if (isnan (f2m)) f2m = 0;

 fp = (-f2p + (float__t)8.0 * fp1 - (float__t)8.0 * fm1 + f2m) / ((float__t)12.0 * d);
 return true;
}

// Helper: Compute f''(x) using 5-point stencil
bool calculator::computeSecondDerivative (calculator *child, char *sexpr, char *svar, float__t x,
                                          float__t &fpp, float__t dx)
{
#ifdef _float128_
 float__t d = fmaxq (fabsq (x), 1.0Q) * dx;
#elif defined(__BORLANDC__)
 float__t ax = fabsl (x);
 float__t d  = ((ax > 1.0L) ? ax : 1.0L) * dx;
#else
 float__t d = fmaxl (fabsl (x), 1.0L) * dx;
#endif

 // 5-point stencil: f''(x) ≈ [-f(x+2h) + 16f(x+h) - 30f(x) + 16f(x-h) - f(x-2h)] / (12h²)
 child->addfvar (svar, x + (float__t)2.0 * d);
 float__t f2p = child->evaluate_f (sexpr);
 if (isnan (f2p)) f2p = 0;

 child->addfvar (svar, x + d);
 float__t fp1 = child->evaluate_f (sexpr);
 if (isnan (fp1)) fp1 = 0;

 child->addfvar (svar, x);
 float__t f0 = child->evaluate_f (sexpr);
 if (isnan (f0)) f0 = 0;

 child->addfvar (svar, x - d);
 float__t fm1 = child->evaluate_f (sexpr);
 if (isnan (fm1)) fm1 = 0;

 child->addfvar (svar, x - (float__t)2.0 * d);
 float__t f2m = child->evaluate_f (sexpr);
 if (isnan (f2m)) f2m = 0;

 fpp = (-f2p + (float__t)16.0 * fp1 - (float__t)30.0 * f0 + (float__t)16.0 * fm1 - f2m)
       / ((float__t)12.0 * d * d);
 return true;
}

// Find extremum using robust combined method with Newton refinement
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
 const float__t tol   = 1e-32Q;
 const float__t delta = 5e-19Q; // Меньший шаг для 128-bit
#else
 const float__t tol   = 1e-14L;
 const float__t delta = 5e-12L; // Меньший шаг для 64-bit
#endif
 const int maxIter = 300; // Больше итераций

 // Step 1: Bracket the root of f'(x) = 0
 float__t x = x0;
 float__t fp0;
 if (!computeDerivative (child, sexpr, svar, x0, fp0, delta))
  {
   errorf (pos, "%s", child->err);
   delete child;
   return result_fval = qnan;
  }

 float__t step = fabsl (x0) * (float__t)0.1;
 if (step < (float__t)0.1) step = (float__t)0.1;

 float__t xa = x0 - step;
 float__t xb = x0 + step;
 float__t fpa, fpb;

 bool bracketed = false;
 int i;
 for (i = 0; i < 15; i++) // Больше попыток bracketing
  {
   if (!computeDerivative (child, sexpr, svar, xa, fpa, delta)
       || !computeDerivative (child, sexpr, svar, xb, fpb, delta))
    {
     step *= (float__t)0.5;
     xa = x0 - step;
     xb = x0 + step;
     continue;
    }

   if (fpa * fpb < 0)
    {
     bracketed = true;
     break;
    }

   if (fabsl (fpa) < fabsl (fpb))
    {
     xb  = x0;
     fpb = fp0;
     x0  = xa;
     fp0 = fpa;
     xa  = x0 - step;
    }
   else
    {
     xa  = x0;
     fpa = fp0;
     x0  = xb;
     fp0 = fpb;
     xb  = x0 + step;
    }
   step *= (float__t)1.5;
  }

 if (!bracketed)
  {
   // Gradient descent fallback
   x                      = x0;
   float__t learning_rate = (float__t)0.01;

   for (i = 0; i < maxIter; i++)
    {
     float__t fp;
     if (!computeDerivative (child, sexpr, svar, x, fp, delta))
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     if (fabsl (fp) < tol * (float__t)100.0)
      {
       break;
      }

     float__t x_new = x - learning_rate * fp;
     float__t fp_new;
     if (!computeDerivative (child, sexpr, svar, x_new, fp_new, delta))
      {
       learning_rate *= (float__t)0.5;
       continue;
      }

     if (fabsl (fp_new) < fabsl (fp))
      {
       x = x_new;
       learning_rate *= (float__t)1.05; // Медленнее увеличиваем
      }
     else
      {
       learning_rate *= (float__t)0.5;
      }

     if (learning_rate < tol * (float__t)0.001) break;
    }

   xa = x - fabsl (x) * (float__t)0.01;
   xb = x + fabsl (x) * (float__t)0.01;
   if (!computeDerivative (child, sexpr, svar, xa, fpa, delta)
       || !computeDerivative (child, sexpr, svar, xb, fpb, delta))
    {
     errorf (pos, "Cannot refine extremum");
     delete child;
     return result_fval = qnan;
    }
   bracketed = (fpa * fpb < 0);
   if (!bracketed)
    {
     fflags |= child->isfflags ();
     delete child;
     return x;
    }
  }

 // Step 2: Brent's method (more iterations, tighter tolerance)
 float__t xc      = xb;
 float__t fpc     = fpb;
 float__t d_brent = 0, e_brent = 0;

 for (i = 0; i < maxIter; i++)
  {
   if (fabsl (fpa) < fabsl (fpb))
    {
     float__t tmp = xa;
     xa           = xb;
     xb           = tmp;
     tmp          = fpa;
     fpa          = fpb;
     fpb          = tmp;
    }

   float__t tol1 = (float__t)2.0 * delta * fabsl (xb) + (float__t)0.5 * tol;
   float__t xm   = (float__t)0.5 * (xa - xb);

   if (fabsl (xm) <= tol1 || fabsl (fpb) < tol)
    {
     x = xb;
     break;
    }

   if (fabsl (e_brent) >= tol1 && fabsl (fpc) > fabsl (fpb))
    {
     float__t s, p, q, r;
     s = fpb / fpc;
     if (fabsl (xa - xc) < tol)
      {
       p = (float__t)2.0 * xm * s;
       q = (float__t)1.0 - s;
      }
     else
      {
       q = fpc / fpa;
       r = fpb / fpa;
       p = s * ((float__t)2.0 * xm * q * (q - r) - (xb - xc) * (r - (float__t)1.0));
       q = (q - (float__t)1.0) * (r - (float__t)1.0) * (s - (float__t)1.0);
      }

     if (p > 0) q = -q;
     p = fabsl (p);

     float__t min1 = (float__t)3.0 * xm * q - fabsl (tol1 * q);
     float__t min2 = fabsl (e_brent * q);
     if ((float__t)2.0 * p < (min1 < min2 ? min1 : min2))
      {
       e_brent = d_brent;
       d_brent = p / q;
      }
     else
      {
       d_brent = xm;
       e_brent = d_brent;
      }
    }
   else
    {
     d_brent = xm;
     e_brent = d_brent;
    }

   xa  = xb;
   fpa = fpb;

   if (fabsl (d_brent) > tol1)
    xb += d_brent;
   else
    xb += (xm > 0 ? tol1 : -tol1);

   if (!computeDerivative (child, sexpr, svar, xb, fpb, delta))
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }
  }

 // Step 3: Newton refinement with second derivative (more iterations, smaller steps)
 for (i = 0; i < 50; i++) // Больше итераций для полировки
  {
   float__t fp, fpp;
   float__t refined_delta = delta * (float__t)0.01; // Ещё меньший шаг

   if (!computeDerivative (child, sexpr, svar, x, fp, refined_delta)) break;
   if (!computeSecondDerivative (child, sexpr, svar, x, fpp, refined_delta)) break;

   if (fabsl (fp) < tol) break;

   if (fabsl (fpp) < tol * (float__t)0.001) break;

   float__t step_newton = fp / fpp;

   // Более агрессивное damping для пологих функций
   float__t max_step = fabsl (x) * (float__t)0.01;
   if (fabsl (step_newton) > max_step) step_newton = (step_newton > 0 ? 1 : -1) * max_step;

   x -= step_newton;

   if (fabsl (step_newton) < tol * (float__t)0.1 * fabsl (x)) break;
  }

 fflags |= child->isfflags ();
 delete child;
 return x;
}
#endif

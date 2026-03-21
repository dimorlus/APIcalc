// Newton-Raphson solution of the equation solve(x(2x+2)-2, x:=0)
// expr -> x(2x+2)-2, x:=0

//Follow methods of calculator class available
// after evaluate_f (expr);
// float__t get_re_res () { return result_fval; }
// float__t get_im_res () { return result_imval; }
// before evaluate_f (expr);
// Add or assign if existing a floating-point or complex variable to the calculator
// void addfvar (const char *name, float__t fval, float__t ival = (float__t)0.0L);                
float__t calculator::Solve (const char *expr, t_symbol tag)
{
 if (expr && *expr)
  {
   char sexpr[STRBUF];
   char svar[STRBUF];
   char nvar[MAXOP];
   char *p = sexpr;
   float__t vvar = qnan;
   // copy all characters from expr (i. e. 'x(2x+2)-2' ) to sexpr until the first ',' or 
   // end of string is reached or  buffer limit is reached
   while (*expr && (*expr != ',') && (p - sexpr < STRBUF - 1))
    {
     *p++ = *expr++;
    }
   *p = '\0'; // null-terminate the string
   // copy the remaining characters (i. e. 'x:=0' ) from expr to svar (if any) until the end 
   // of string is reached or buffer limit is reached
   if (*expr == ',')
    {
     expr++;
     p = svar;
     while (*expr && (p - svar < STRBUF - 1))
      {
       *p++ = *expr++;
      }
     *p = '\0'; // null-terminate the string
    }

   calculator *pCalculator = new calculator (scfg, hash_table, MASK_DEFAULT + MASK_VARIABLE, deep);
   if (!pCalculator)
    {
     errorf (pos, "Out of memory");
     result_fval = qnan;
     return qnan;
    }

   float__t result = pCalculator->evaluate_f (svar);
   if (isnan(result) || pCalculator->err[0])
    {
     errorf (pos, "%s", pCalculator->err);
     delete pCalculator;
     result_fval = qnan;
     return qnan;
    }

   char *lv = (char *)pCalculator->get_last_var ();
   strcpy (nvar, lv);
   vvar = result;

   if (tag == tsSOLVE)
   {
    // Newton-Raphson iteration
    const float__t tol = 1e-12L;
    const int maxIter  = 100;
    float__t x         = vvar;
    bool converged     = false;

    for (int i = 0; i < maxIter; i++)
     {
      pCalculator->addfvar (nvar, x);
      float__t fx = pCalculator->evaluate_f (sexpr);
      if (isnan (fx) || pCalculator->err[0])
       {
        errorf (pos, "%s", pCalculator->err[0] ? pCalculator->err : "Error evaluating expression");
        result_fval = qnan;
        delete pCalculator;
        return qnan;
       }

      if (fabsl (fx) < tol)
       {
        converged = true;
        vvar      = x;
        break;
       }

      // Numerical derivative (central difference)
      #ifdef __BORLANDC__
      float__t ax = fabsl (x);
      float__t delta = ((ax>1.0L)?ax:1.0L) * 1.5e-10L; // slightly smaller for float__t
      #else
      float__t delta = fmaxl (fabsl (x), 1.0L) * 1.5e-10L; // slightly smaller for float__t
      #endif
      pCalculator->addfvar (nvar, x + delta);
      float__t fxp = pCalculator->evaluate_f (sexpr);
      if (isnan (fxp) || pCalculator->err[0])
       {
        errorf (pos, "%s", pCalculator->err);
        result_fval = qnan;
        delete pCalculator;
        return qnan;
       }

      pCalculator->addfvar (nvar, x - delta);
      float__t fxm = pCalculator->evaluate_f (sexpr);
      if (isnan (fxm) || pCalculator->err[0])
       {
        errorf (pos, "%s", pCalculator->err);
        result_fval = qnan;
        delete pCalculator;
        return qnan;
       }

      float__t fp = (fxp - fxm) / (2.0L * delta);
      if (fabsl (fp) < tol)
       {
        // Derivative is close to zero - try to shift
        x += delta * 1000.0L;
        continue;
       }

      float__t x_new = x - fx / fp;
      #ifdef __BORLANDC__
      if (isnan (x_new) || isinf_l (x_new))
      #else
      if (isnan (x_new) || isinf (x_new))
      #endif
       {
        errorf (pos, "Solution diverged");
        result_fval = qnan;
        delete pCalculator;
        return qnan;
       }

      if (fabsl (x_new - x) < tol * (1.0L + fabsl (x)))
       {
        converged = true;
        vvar      = x_new;
        break;
       }

      x = x_new;
     }

    if (!converged)
     {
      errorf (pos, "No solution found");
      result_fval = qnan;
      delete pCalculator;
      return qnan;
     }
   }
   if (tag == tsCALC)
   {
    pCalculator->addfvar (nvar, vvar);
    float__t fx = pCalculator->evaluate_f (sexpr);
    if (isnan (fx) || pCalculator->err[0])
     {
      errorf (pos, "%s", pCalculator->err[0] ? pCalculator->err : "Error evaluating expression");
      result_fval = qnan;
      delete pCalculator;
      return qnan;
     }
    vvar = fx;
   }

   fflags |= pCalculator->isfflags ();
   delete pCalculator;
   return vvar;
  }
 else
  {
   errorf (0, "empty expression");
   return qnan;
  }
 return qnan;
}

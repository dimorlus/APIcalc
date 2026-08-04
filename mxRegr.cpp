bool calculator::mxRegrFn (const char *fname, const char *msk, int n, rtype rt, value &res)
{
 FILE *f = nullptr;
 
 if (n<=0)
  {
   mxerror ("degree must be positive");
   return false;
  }
 
 // Linearized types always have degree 1
 int degree = (rt == rtPoly || rt == rtCheb) ? n : 1;

 if (degree > 6)
  {
   mxerror ("degree too big (max 6)");
   return false;
  }

 // For Chebyshev: need to find min/max for normalization
 float__t x_min = 0.0L, x_max = 0.0L;
 bool first_point = true;
 
 if (rt == rtCheb)
  {
   // First pass: find min/max
   f = fopen (fname, "r");
   if (!f)
    {
     mxerror ("cannot open data file");
     return false;
    }
   
   char line[1024];
   while (fgets (line, sizeof (line), f))
    {
     double xd = qnan, yd = qnan;
     if (strscan (line, msk, 2, &xd, &yd) == 2)
      {
       float__t x = (float__t)xd;
       if (first_point)
        {
         x_min = x_max = x;
         first_point = false;
        }
       else
        {
         if (x < x_min) x_min = x;
         if (x > x_max) x_max = x;
        }
      }
    }
   fclose (f);
   
   if (first_point)
    {
     mxerror ("no data points found");
     return false;
    }
   
   // Avoid division by zero
   if (x_max - x_min < 1e-10L)
    {
     mxerror ("all x values are the same");
     return false;
    }
  }

 // Initialize accumulators on the temporary stack
 int s_size   = 2 * degree + 1;
 int sy_size  = degree + 1;
 float__t *S  = (float__t *)alloca (s_size * sizeof (float__t));
 float__t *SY = (float__t *)alloca (sy_size * sizeof (float__t));

 for (int i = 0; i < s_size; i++) S[i] = 0;
 for (int i = 0; i < sy_size; i++) SY[i] = 0;

 // For Chebyshev: need to accumulate matrix differently
 float__t *M_cheb = nullptr;
 float__t *B_cheb = nullptr;
 
 if (rt == rtCheb)
  {
   int dim = degree + 1;
   M_cheb = (float__t *)alloca (dim * dim * sizeof (float__t));
   B_cheb = (float__t *)alloca (dim * sizeof (float__t));
   for (int i = 0; i < dim * dim; i++) M_cheb[i] = 0;
   for (int i = 0; i < dim; i++) B_cheb[i] = 0;
  }

 // Second pass (or first for non-Chebyshev): accumulate sums
 f = fopen (fname, "r");
 if (f)
  {
   char line[1024];
   while (fgets (line, sizeof (line), f))
    {
     double xd = qnan, yd = qnan;
     if (strscan (line, msk, 2, &xd, &yd) == 2)
      {
       float__t x = (float__t)xd;
       float__t y = (float__t)yd;
       
       // Linearization for non-polynomial types
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

       if (rt == rtCheb)
        {
         // Normalize x to [-1, 1]
         float__t x_norm = 2.0L * (x - x_min) / (x_max - x_min) - 1.0L;
         
         // Compute Chebyshev polynomials T_0(x), T_1(x), ..., T_degree(x)
         float__t *T = (float__t *)alloca ((degree + 1) * sizeof (float__t));
         T[0] = 1.0L;
         if (degree >= 1) T[1] = x_norm;
         for (int k = 2; k <= degree; k++)
          T[k] = 2.0L * x_norm * T[k-1] - T[k-2];
         
         // Build normal equation matrix: M[r][c] = Σ T_r(x) * T_c(x)
         for (int r = 0; r <= degree; r++)
          for (int c = 0; c <= degree; c++)
           M_cheb[r * (degree + 1) + c] += T[r] * T[c];
         
         // Build right-hand side: B[r] = Σ y * T_r(x)
         for (int r = 0; r <= degree; r++)
          B_cheb[r] += y * T[r];
        }
       else
        {
         // Power series: accumulate sums of powers
         float__t px = (float__t)1.0L;
         for (int i = 0; i < s_size; i++)
          {
           S[i] += px;
           if (i < sy_size) SY[i] += y * px;
           px *= x;
          }
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

 // 2. Form matrices to solve the system M * A = B
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

 if (rt == rtCheb)
  {
   // Copy from Chebyshev accumulators
   for (int r = 0; r < dim; r++)
    {
     for (int c = 0; c < dim; c++)
      valM.mval[r * dim + c] = M_cheb[r * dim + c];
     valB.mval[r] = B_cheb[r];
    }
  }
 else
  {
   // Power series
   for (int r = 0; r < dim; r++)
    {
     for (int c = 0; c < dim; c++)
      valM.mval[r * dim + c] = S[r + c];
     valB.mval[r] = SY[r];
    }
  }

 // 3. Solve the system using internal functions: A = inv(M) * B
 if (!mxInv (valInvM, valM))
  {
   mxerror ("Matrix is singular (not enough distinct points?)");
   return false;
  }

 if (!mxMatMul (valCoeff, valInvM, valB)) return false;

 // 4. Post-processing and preparing the result
 float__t *res_mval = mxAlloc (1, dim);

 if (rt == rtCheb)
  {
   // For Chebyshev: coefficients are in order [c0, c1, c2, ...]
   // Return as row vector with normalization info encoded
   // Store: [c0, c1, ..., cn] (coefficients stay in natural order)
   for (int i = 0; i < dim; i++)
    res_mval[i] = valCoeff.mval[i];
   
   // Note: caller needs x_min, x_max for denormalization
   // We could store them as additional "metadata" but that breaks matrix format
   // Better: document that Chebyshev regression assumes normalized input
  }
 else
  {
   // Transfer coefficients from highest degree to lowest (polynomial format)
   for (int i = 0; i < dim; i++)
    res_mval[i] = valCoeff.mval[degree - i];
   
   // Inverse transformation for exponential and power types
   if (rt == rtExp || rt == rtPow)
    res_mval[dim - 1] = Exp (res_mval[dim - 1]);
  }

 res.tag   = tvMATRIX;
 res.mrows = 1;
 res.mcols = dim;
 res.mval  = res_mval;

 return true;
}
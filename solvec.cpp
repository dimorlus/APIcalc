float__t calculator::Solve(const char *expr, t_symbol tag)
{
 if (!expr || !*expr)
  { errorf(0, "empty expression"); return qnan; }

 char sexpr[STRBUF], svar[STRBUF], nvar[MAXOP];
 char *p = sexpr;
 while (*expr && (*expr != ',') && (p - sexpr < STRBUF - 1))
  *p++ = *expr++;
 *p = '\0';
 if (*expr != ',')
  { errorf(pos, "solve: missing initial estimate"); return qnan; }
 expr++;
 p = svar;
 while (*expr && (p - svar < STRBUF - 1))
  *p++ = *expr++;
 *p = '\0';

 calculator *pC = new calculator(scfg, hash_table, MASK_DEFAULT + MASK_VARIABLE, deep);
 if (!pC) { errorf(pos, "Out of memory"); return qnan; }

 pC->evaluate_f(svar);
 if (pC->err[0]) { errorf(pos, "%s", pC->err); delete pC; return qnan; }
 strcpy(nvar, (char *)pC->get_last_var());
 float__t xr = pC->get_re_res();
 float__t xi = pC->get_im_res();  // 0 если вещественное начальное приближение

 if (tag == tsCALC)
  {
   pC->addfvar(nvar, xr, xi);
   pC->evaluate_f(sexpr);
   if (pC->err[0]) { errorf(pos, "%s", pC->err); delete pC; return qnan; }
   result_fval  = pC->get_re_res();
   result_imval = pC->get_im_res();
   if (result_imval != (float__t)0.0) result_tag = tvCOMPLEX;
   fflags |= pC->isfflags();
   delete pC;
   return result_fval;
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
   pC->addfvar(nvar, xr, xi);
   pC->evaluate_f(sexpr);
   if (pC->err[0]) { errorf(pos, "%s", pC->err); delete pC; return qnan; }
   float__t fr = pC->get_re_res();
   float__t fi = pC->get_im_res();

   // |f(z)|
   float__t fabs_f = sqrt(fr*fr + fi*fi);
   if (fabs_f < tol) { converged = true; break; }

   // шаг d = max(|z|, 1) * delta
   float__t d = fmax(sqrt(xr*xr + xi*xi), (float__t)1.0) * delta;

   // f(z+d) и f(z-d) по вещественной оси
   pC->addfvar(nvar, xr + d, xi);
   pC->evaluate_f(sexpr);
   float__t fpr_r = pC->get_re_res(), fpr_i = pC->get_im_res();

   pC->addfvar(nvar, xr - d, xi);
   pC->evaluate_f(sexpr);
   float__t fmr_r = pC->get_re_res(), fmr_i = pC->get_im_res();

   // f'(z) ≈ (f(z+d) - f(z-d)) / (2d)
   float__t fp_r = (fpr_r - fmr_r) / ((float__t)2.0 * d);
   float__t fp_i = (fpr_i - fmr_i) / ((float__t)2.0 * d);

   float__t fp2 = fp_r*fp_r + fp_i*fp_i;
   if (fp2 < tol*tol)
    { xr += d * (float__t)1000.0; continue; }

   // Newton step: z -= f(z)/f'(z)
   float__t step_r = (fr*fp_r + fi*fp_i) / fp2;
   float__t step_i = (fi*fp_r - fr*fp_i) / fp2;
   float__t xr_new = xr - step_r;
   float__t xi_new = xi - step_i;

   if (isnan(xr_new) || isnan(xi_new))
    { errorf(pos, "Solution diverged"); delete pC; return qnan; }

   float__t step_abs = sqrt(step_r*step_r + step_i*step_i);
   float__t z_abs    = sqrt(xr*xr + xi*xi);
   if (step_abs < tol * ((float__t)1.0 + z_abs))
    { xr = xr_new; xi = xi_new; converged = true; break; }

   xr = xr_new;
   xi = xi_new;
  }

 fflags |= pC->isfflags();
 delete pC;

 if (!converged)
  { errorf(pos, "No solution found"); result_fval = qnan; return qnan; }

 result_fval  = xr;
 result_imval = xi;
 if (xi != (float__t)0.0) result_tag = tvCOMPLEX;
 return xr;
}
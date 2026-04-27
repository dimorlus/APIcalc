// Обновить главную функцию Plot:
bool calculator::Plot (const char *expr, v_func fidx)
{
 char fname[STRBUF];
 fname[0] = '\0';

 PlotParams params;
 memset (&params, 0, sizeof (params));

 // 1. Prepare data
 if (!PlotPrepare (expr, fidx, fname, params))
  {
   return false;
  }

 NormalizePath (fname, fname, STRBUF);

 // 2. Create or load bitmap
 bmpdraw *bmp = new bmpdraw ();
 
 bool is_overlay = ((fidx == pl_oplot || fidx == pl_oplotpol || fidx == pl_oxyplot ||
                     fidx == pl_oplotlgx || fidx == pl_oplotlgy || fidx == pl_oplotlgxy ||
                     fidx == pl_oplotsmith || fidx == pl_oplotsmithz) &&
                    fname[0]);
 
 bool is_polar = (fidx == pl_plotpol || fidx == pl_fplotpol || fidx == pl_oplotpol);
 
 bool is_parametric = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot);
 
 bool is_logarithmic = (fidx == pl_plotlgx || fidx == pl_fplotlgx || fidx == pl_oplotlgx ||
                        fidx == pl_plotlgy || fidx == pl_fplotlgy || fidx == pl_oplotlgy ||
                        fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);
 
 bool is_smith = (fidx == pl_plotsmith || fidx == pl_fplotsmith || fidx == pl_oplotsmith ||
                  fidx == pl_plotsmithz || fidx == pl_fplotsmithz || fidx == pl_oplotsmithz);

 if (is_overlay)
  {
   if (bmp->load (fname))
    {
     params.width      = bmp->getWidth ();
     params.height     = bmp->getHeight ();
     params.bgc        = bmp->getDominantColor ();
     params.text_color = ~params.bgc;
    }
   else
    {
     is_overlay = false;
     if (!bmp->newbmp (params.width, params.height, params.bgc))
      {
       errorf (pos, "Failed to create bitmap for plotting");
       delete bmp;
       if (params.child) delete params.child;
       if (params.sexpr) free (params.sexpr);
       if (params.sexpr_y) free (params.sexpr_y);
       if (params.svar) free (params.svar);
       result_fval = qnan;
       return false;
      }
    }
  }
 else
  {
   if (!bmp->newbmp (params.width, params.height, params.bgc))
    {
     errorf (pos, "Failed to create bitmap for plotting");
     delete bmp;
     if (params.child) delete params.child;
     if (params.sexpr) free (params.sexpr);
     if (params.sexpr_y) free (params.sexpr_y);
     if (params.svar) free (params.svar);
     result_fval = qnan;
     return false;
    }
  }

 // 3. Draw plot
 bool plot_success;
 if (is_smith)
  {
   plot_success = PlotSmith (bmp, params);
  }
 else if (is_logarithmic)
  {
   plot_success = PlotLogarithmic (bmp, params);
  }
 else if (is_parametric)
  {
   plot_success = PlotParametric (bmp, params);
  }
 else if (is_polar)
  {
   plot_success = PlotPolar (bmp, params);
  }
 else
  {
   plot_success = PlotCartesian (bmp, params);
  }

 if (!plot_success)
  {
   delete bmp;
   delete params.child;
   free (params.sexpr);
   if (params.sexpr_y) free (params.sexpr_y);
   free (params.svar);
   result_fval = qnan;
   return false;
  }

 // 4. Draw axes and grid (skip for overlay mode)
 if (!is_overlay)
  {
   if (is_smith)
    {
     PlotDrawAxesSmith (bmp, params);
    }
   else if (is_logarithmic)
    {
     PlotDrawAxesLog (bmp, params);
    }
   else if (is_parametric)
    {
     PlotDrawAxesParametric (bmp, params);
    }
   else if (is_polar)
    {
     PlotDrawAxesPolar (bmp, params);
    }
   else
    {
     PlotDrawAxesCartesian (bmp, params);
    }
  }

 // 5. Save or display
 if ((fidx == pl_fplot) || (fidx == pl_oplot) || 
     (fidx == pl_fplotpol) || (fidx == pl_oplotpol) ||
     (fidx == pl_fxyplot) || (fidx == pl_oxyplot) ||
     (fidx == pl_fplotlgx) || (fidx == pl_oplotlgx) ||
     (fidx == pl_fplotlgy) || (fidx == pl_oplotlgy) ||
     (fidx == pl_fplotlgxy) || (fidx == pl_oplotlgxy) ||
     (fidx == pl_fplotsmith) || (fidx == pl_oplotsmith) ||
     (fidx == pl_fplotsmithz) || (fidx == pl_oplotsmithz))
  {
   bmp->save (fname);
  }
 else
  {
   if (ShowImageFn)
    {
     ShowImageFn ((void *)bmp);
    }
  }

 // 6. Cleanup
 delete bmp;
 fflags |= params.child->isfflags ();
 delete params.child;
 free (params.sexpr);
 if (params.sexpr_y) free (params.sexpr_y);
 free (params.svar);

 return true;
}
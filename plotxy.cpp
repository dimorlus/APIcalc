// В PlotParams добавить:
struct PlotParams
{
 char *sexpr;
 char *sexpr_y;  // NEW: для второй функции в параметрических графиках
 char *svar;
 // ... остальные поля без изменений
};
// Модифицировать PlotPrepare для обработки параметрических графиков:
bool calculator::PlotPrepare (const char *expr, v_func fidx, char *fname, PlotParams &params)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return false;
  }

 char sexpr[STRBUF];
 char sexpr_y[STRBUF];
 char sfrom[MAXOP];
 char sto[MAXOP];
 char svar[STRBUF];

 bool split_ok = false;
 
 // Determine which split pattern to use
 switch (fidx)
  {
  case pl_fxyplot:
  case pl_oxyplot:
   // fname, x_expr, y_expr, from, to, var
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sexpr_y, STRBUF,
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_xyplot:
   // x_expr, y_expr, from, to, var
   split_ok = Split (expr, sexpr, STRBUF, sexpr_y, STRBUF,
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_fplot:
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
   // fname, expr, from, to, var
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, 
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_plot:
  case pl_plotpol:
  default:
   if (!ShowImageFn) return false;
   // expr, from, to, var
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;
  }

 if (!split_ok)
  {
   result_fval = qnan;
   return false;
  }

 calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return false;
  }

 // Process filename for file-based functions
 if (fidx == pl_fplot || fidx == pl_oplot || 
     fidx == pl_fplotpol || fidx == pl_oplotpol ||
     fidx == pl_fxyplot || fidx == pl_oxyplot)
  {
   child->setFileDlgFn (FileDlgFn);
   child->evaluate_f (fname);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }

   if (child->get_res_tag () != tvSTR)
    {
     errorf (pos, "First argument is not a file name");
     delete child;
     result_fval = qnan;
     return false;
    }
   strncpy (fname, child->get_str_res (), STRBUF - 1);
   fname[STRBUF - 1] = '\0';
  }

 // Evaluate from/to parameters
 float__t vfrom = child->evaluate_f (sfrom);
 if (isnan (vfrom) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 float__t vto = child->evaluate_f (sto);
 if (isnan (vto) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 if (vfrom > vto)
  {
   float__t tmp = vfrom;
   vfrom        = vto;
   vto          = tmp;
  }

 // Test evaluate first expression
 child->addfvar (svar, vfrom);
 float__t fvx = child->evaluate_f (sexpr);

 if (!(isnan (fvx) && child->errt () == teMath))
  {
   if (isnan (fvx) || child->err[0] || !CheckChildRes (child))
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
  }
 else
  fvx = 0;

 // For parametric plots, test second expression
 if (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot)
  {
   float__t fvy = child->evaluate_f (sexpr_y);
   if (!(isnan (fvy) && child->errt () == teMath))
    {
     if (isnan (fvy) || child->err[0] || !CheckChildRes (child))
      {
       errorf (pos, "%s", child->err);
       delete child;
       result_fval = qnan;
       return false;
      }
    }
  }

 // Fill in the parameters
 params.sexpr = strdup (sexpr);
 params.sexpr_y = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot) 
                  ? strdup (sexpr_y) : nullptr;
 params.svar  = strdup (svar);
 params.vfrom = vfrom;
 params.vto   = vto;
 params.ymin  = fvx;
 params.ymax  = fvx;

 params.width = getivar ("plot_width");
 if ((params.width <= 100) || (params.width > 2000)) params.width = 800;

 params.height = getivar ("plot_height");
 if ((params.height <= 100) || (params.height > 2000)) params.height = 600;

 params.bgc     = getivar ("plot_bgc");
 params.fgc     = getivar ("plot_fgc");
 params.padding = 40;

 params.grid_color = 0xC0C0C0;
 params.axis_color = 0x808080;
 params.text_color = ~params.bgc;

 params.child = child;

 return true;
}

// Добавить PlotParametric:
bool calculator::PlotParametric (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t t_from = params.vfrom;
 float__t t_to   = params.vto;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 float__t step = (t_to - t_from) / ((width - 2 * padding) * 2);

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find bounding box
 float__t t = t_from;
 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    return false;

   child->addfvar (params.svar, t);

   float__t x = child->evaluate_f (params.sexpr);
   if (!(isnan (x) && child->errt () == teMath))
    {
     if (isnan (x) || child->err[0] || !CheckChildRes (child))
      {
       t += step;
       continue;
      }
    }
   else
    x = 0;

   float__t y = child->evaluate_f (params.sexpr_y);
   if (!(isnan (y) && child->errt () == teMath))
    {
     if (isnan (y) || child->err[0] || !CheckChildRes (child))
      {
       t += step;
       continue;
      }
    }
   else
    y = 0;

   if (!isChildResReal (child))
    {
     t += step;
     continue;
    }

   if (first_point)
    {
     xmin = xmax = x;
     ymin = ymax = y;
     first_point = false;
    }
   else
    {
     if (x < xmin) xmin = x;
     if (x > xmax) xmax = x;
     if (y < ymin) ymin = y;
     if (y > ymax) ymax = y;
    }

   t += step;
  }
 while (t <= t_to);

 // Add padding
 if (xmin == xmax)
  {
   xmin -= 1.0;
   xmax += 1.0;
  }
 if (ymin == ymax)
  {
   ymin -= 1.0;
   ymax += 1.0;
  }

 float__t x_pad = (xmax - xmin) * 0.1;
 float__t y_pad = (ymax - ymin) * 0.1;
 xmin -= x_pad;
 xmax += x_pad;
 ymin -= y_pad;
 ymax += y_pad;

 // Second pass: draw the curve
 t                       = t_from;
 bool has_valid_points   = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    return false;

   child->addfvar (params.svar, t);

   float__t x = child->evaluate_f (params.sexpr);
   bool x_valid = true;
   if (isnan (x) && child->errt () == teMath)
    x = 0;
   else if (isnan (x) || !isChildResReal (child))
    x_valid = false;

   float__t y = child->evaluate_f (params.sexpr_y);
   bool y_valid = true;
   if (isnan (y) && child->errt () == teMath)
    y = 0;
   else if (isnan (y) || !isChildResReal (child))
    y_valid = false;

   if (x_valid && y_valid)
    {
     int x_screen
         = padding + (int)(((x - xmin) / (xmax - xmin)) * (width - 2 * padding));
     int y_screen
         = height - padding - (int)(((y - ymin) / (ymax - ymin)) * (height - 2 * padding));

     if (has_valid_points)
      {
       bmp->lineTo (x_screen, y_screen, 2, fgc);
      }
     else
      {
       bmp->moveTo (x_screen, y_screen);
       has_valid_points = true;
      }
    }
   else
    {
     has_valid_points = false;
    }

   t += step;
  }
 while (t <= t_to);

 // Store parameters for axis drawing
 params.xmin = xmin;
 params.xmax = xmax;
 params.ymin = ymin;
 params.ymax = ymax;

 return true;
}

// Добавить PlotDrawAxesParametric:
void calculator::PlotDrawAxesParametric (bmpdraw *bmp, PlotParams &params)
{
 // Reuse Cartesian axes
 PlotDrawAxesCartesian (bmp, params);

 // Override title with both functions
 char title[256];
 snprintf (title, sizeof (title), "x=%s, y=%s", params.sexpr, params.sexpr_y);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (params.width / 2 - 80, 5, title, params.text_color, 0, 2);
}

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
 bmpdraw *bmp    = new bmpdraw ();
 bool is_overlay = ((fidx == pl_oplot || fidx == pl_oplotpol || fidx == pl_oxyplot) && fname[0]);
 bool is_polar   = (fidx == pl_plotpol || fidx == pl_fplotpol || fidx == pl_oplotpol);
 bool is_parametric = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot);

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
 if (is_parametric)
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
   if (is_parametric)
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
 if ((fidx == pl_fplot) || (fidx == pl_oplot) || (fidx == pl_fplotpol) || (fidx == pl_oplotpol)
     || (fidx == pl_fxyplot) || (fidx == pl_oxyplot))
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

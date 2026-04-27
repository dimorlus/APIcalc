// Модифицировать PlotPrepare для обработки Smith chart:
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
 char sz0[MAXOP];

 bool split_ok = false;
 
 // Determine which split pattern to use
 switch (fidx)
  {
  case pl_fxyplot:
  case pl_oxyplot:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sexpr_y, STRBUF,
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_xyplot:
   split_ok = Split (expr, sexpr, STRBUF, sexpr_y, STRBUF,
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_fplot:
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
  case pl_fplotlgx:
  case pl_oplotlgx:
  case pl_fplotlgy:
  case pl_oplotlgy:
  case pl_fplotlgxy:
  case pl_oplotlgxy:
  case pl_fplotsmith:
  case pl_oplotsmith:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, 
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_plotsmithz:
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, 
                     svar, STRBUF, sz0, MAXOP, nullptr, 0);
   break;

  case pl_fplotsmithz:
  case pl_oplotsmithz:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sfrom, MAXOP, 
                     sto, MAXOP, svar, STRBUF, sz0, MAXOP, nullptr, 0);
   break;

  case pl_plot:
  case pl_plotpol:
  case pl_plotlgx:
  case pl_plotlgy:
  case pl_plotlgxy:
  case pl_plotsmith:
  default:
   if (!ShowImageFn) return false;
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
     fidx == pl_fxyplot || fidx == pl_oxyplot ||
     fidx == pl_fplotlgx || fidx == pl_oplotlgx ||
     fidx == pl_fplotlgy || fidx == pl_oplotlgy ||
     fidx == pl_fplotlgxy || fidx == pl_oplotlgxy ||
     fidx == pl_fplotsmith || fidx == pl_oplotsmith ||
     fidx == pl_fplotsmithz || fidx == pl_oplotsmithz)
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

 // Evaluate Z0 for Smith chart with explicit Z0
 float__t z0 = 50.0; // Default
 if (fidx == pl_plotsmithz || fidx == pl_fplotsmithz || fidx == pl_oplotsmithz)
  {
   z0 = child->evaluate_f (sz0);
   if (isnan (z0) || child->err[0] || z0 <= 0)
    {
     errorf (pos, "Invalid Z0 (must be positive real number)");
     delete child;
     result_fval = qnan;
     return false;
    }
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

 // Set logarithmic flags
 params.log_x = (fidx == pl_plotlgx || fidx == pl_fplotlgx || fidx == pl_oplotlgx ||
                 fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);
 params.log_y = (fidx == pl_plotlgy || fidx == pl_fplotlgy || fidx == pl_oplotlgy ||
                 fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);

 // Set Z0 for Smith chart
 params.z0 = z0;

 params.child = child;

 return true;
}
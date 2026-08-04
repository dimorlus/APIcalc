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

#pragma region GRAPHICS_ENGINE
bool calculator::CheckChildRes (calculator *child)
{
 if (child->err[0])
  {
   errorf (pos, "%s", child->err);
   return false;
  }
 if (child->get_res_tag () == tvMATRIX)
  {
   errorf (pos, "Result is a matrix, expected a scalar");
   return false;
  }
 if (child->get_res_tag () == tvSTR)
  {
   errorf (pos, "Result is a string, expected a scalar");
   return false;
  }
 if (child->get_res_tag () == tvBMP)
  {
   errorf (pos, "Result is a bitmap, expected a scalar");
   return false;
  }
 return true;
}

bool calculator::isChildResReal (calculator *child)
{
 float__t re = child->get_re_res ();
 float__t im = child->get_im_res ();
 if (isnan (re) || isinf(re) || Abs (im) / AbsC (re, im) > (float__t)1e-12L)
  {
   return false; // treat very small real part with large relative imaginary part as non-real
  }
 return true;
}


void unquote_string (char *str)
{
 if (!str) return;
 char *src = str, *dst = str;
 while (*src)
  {
   if (*src == '"' || *src == '\'') // skip quotes
    {
     src++;
     continue;
    }
   *dst++ = *src++;
  }
 *dst = '\0';
}

// Prepare data for plotting (common part)
bool calculator::PlotPrepare (const char *expr, v_func fidx, PlotParams &params)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return false;
  }

 char sexpr[STRBUF]   = { '\0' };
 char sexpr_y[STRBUF] = { '\0' };
 char sfrom[MAXOP]    = { '\0' };
 char sto[MAXOP]      = { '\0' };
 char svar[STRBUF]    = { '\0' };
 char sz0[MAXOP]      = { '\0' };

 bool split_ok = false;
 bool UsePrevRange = false;
 
 // Determine which split pattern to use
 switch (fidx)
 {
  case pl_plot:     // expr, from, to, var
  case pl_plotpol:
  case pl_plotlgx:
  case pl_plotlgy:
  case pl_plotlgxy:
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   if (!split_ok) split_ok = Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0);
   break;

  case pl_xyplot:  // x_expr, y_expr, from, to, var
   split_ok = Split (expr, sexpr, STRBUF, sexpr_y, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF,
                     nullptr, 0);
   if (!split_ok) split_ok = Split (expr, sexpr, STRBUF, sexpr_y, STRBUF, svar, STRBUF, nullptr, 0);
  break;

  case pl_plotsmith:
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;
  case pl_plotsmithz: // expr, from, to, var, z0
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, sz0, MAXOP,
                     nullptr, 0);
   break;

  case pl_plotdata: // datafile, mask
  case pl_plotdatal:
   split_ok = Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0);    // try get mask
   if (!split_ok) split_ok = Split (expr, sexpr, STRBUF, nullptr, 0);    // get datafile
  break;

  default:
   errorf (pos, "Unknown plot function");
   return false;
  }
 
 if (!split_ok)
  {
   result_fval = qnan;
   return false;
  }

 switch (fidx)
  {
  case pl_plot: // expr, from, to, var
  case pl_plotdata: // datafile, mask
  case pl_plotdatal:
   UsePrevRange = (PlotFunc == pl_plot) || (PlotFunc == pl_plotdata) || (PlotFunc == pl_plotdatal) || (PlotFunc == pl_any);
   break;
  case pl_plotpol:
   UsePrevRange = (PlotFunc == pl_plotpol) || (PlotFunc == pl_any);
   break;
  case pl_plotlgx:
   UsePrevRange = (PlotFunc == pl_plotlgx) || (PlotFunc == pl_any);
   break;
  case pl_plotlgy:
   UsePrevRange = (PlotFunc == pl_plotlgy) || (PlotFunc == pl_any);
   break;
  case pl_plotlgxy:
   UsePrevRange = (PlotFunc == pl_plotlgxy) || (PlotFunc == pl_any);
   break;
  case pl_xyplot: // x_expr, y_expr, from, to, var
   UsePrevRange = (PlotFunc == pl_xyplot) || (PlotFunc == pl_any);
   break;
  case pl_plotsmith:
  case pl_plotsmithz: // expr, from, to, var, z0
   UsePrevRange = false;
   break;
  default:
   UsePrevRange = false;
  }

 calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return false;
  }

 params.bgc = (int)getivar ("plot_bgc");
 params.fgc = (int)getivar ("plot_fgc");
 params.width = (int)getivar ("plot_width");
 if ((params.width <= 100) || (params.width > 2000)) params.width = 800;
 params.height = (int)getivar ("plot_height");
 if ((params.height <= 100) || (params.height > 2000)) params.height = 600;
 params.top = (int)getivar ("plot_top");
 if (params.top < 0 || params.top > 2000) params.top = 0;
 params.left = (int)getivar ("plot_left");
 if (params.left < 0 || params.left > 2000) params.left = 0;
 params.pxsize = (int)getivar ("plot_dotsz");
 if (params.pxsize < 1 || params.pxsize > 4) params.pxsize = 4;

 if (fidx < pl_plotdata)
  {
   float__t vfrom = qnan, vto = qnan;

   switch (fidx)
    {
    case pl_plot: // expr, from, to, var
    case pl_plotlgx:
    case pl_plotlgy:
    case pl_plotlgxy:
     {
      // Evaluate from/to parameters
      // Non-parametric plot should use previous range for X, if they are set, otherwise evaluate
      // from/to
      if (!isnan (Plot_Xmin) && UsePrevRange)
       vfrom = Plot_Xmin; // Use previous Xmin if set
      else
       {
        if (sfrom[0]) vfrom = child->evaluate_f (sfrom);
       }
      if (!isnan (Plot_Xmax) && UsePrevRange)
       vto = Plot_Xmax; // Use previous Xmax if set
      else
       {
        if (sto[0]) vto = child->evaluate_f (sto);
       }
     }
    break;

    case pl_plotsmith:
    case pl_plotsmithz: // expr, from, to, var, z0
    case pl_plotpol:
    case pl_xyplot: // x_expr, y_expr, from, to, var
     {
      // Evaluate from/to parameters
      // Parametric plot may use previous range for parameter, otherwise evaluate from/to
      if (!isnan (Plot_Tmin) && UsePrevRange) vfrom = Plot_Tmin; // Use previous Tmin if set
      if (sfrom[0]) vfrom = child->evaluate_f (sfrom);
      if (!isnan (Plot_Tmax) && UsePrevRange) vto = Plot_Tmax; // Use previous Tmax if set
      if (sto[0]) vto = child->evaluate_f (sto);
     }
    break;
    case pl_plotdata: // datafile, mask
    case pl_plotdatal:
    default:
     break;
    }

   switch (fidx)
    {
    case pl_plot: // expr, from, to, var
    case pl_plotlgx:
    case pl_plotlgy:
    case pl_plotlgxy:
    case pl_plotsmith:
    case pl_plotsmithz: // expr, from, to, var, z0
     Plot_Xmin = vfrom;
     Plot_Xmax = vto;
    break;
    case pl_plotpol:
    case pl_xyplot: // x_expr, y_expr, from, to, var
     Plot_Tmin = vfrom;
     Plot_Tmax = vto;
    break;
    case pl_plotdata: // datafile, mask
    case pl_plotdatal:
    default:
     break;
    }

   if (isnan (vfrom) || isnan (vto) || child->err[0])
    {
     if (child->err[0])
      errorf (pos, "%s", child->err);
     else
      errorf (pos, "Invalid range values");
     delete child;
     result_fval = qnan;
     return false;
    }

   Plot_Xmin = vfrom;
   Plot_Xmax = vto;

   if (vfrom > vto)
    {
     float__t tmp = vfrom;
     vfrom        = vto;
     vto          = tmp;
    }

   // NEW: Check for zero range (prevent infinite loops)
   if (Abs (vto - vfrom) < 1e-10)
    {
     errorf (pos, "Invalid range: from and to values must be different");
     delete child;
     result_fval = qnan;
     return false;
    }
   // Evaluate Z0 for Smith chart with explicit Z0
   float__t z0 = 50.0; // Default
   if (fidx == pl_plotsmithz)
    {
     z0 = child->evaluate_f (sz0);
     if (isnan (z0) || child->err[0] || !CheckChildRes (child) || !isChildResReal (child)
         || z0 <= 0)
      {
       errorf (pos, "Invalid Z0 (must be positive real number)");
       delete child;
       result_fval = qnan;
       return false;
      }
    }

   // Test evaluate first expression
   if (!isname (svar))
    {
     errorf (pos, "Invalid variable name");
     delete child;
     result_fval = qnan;
     return false;
    }
   child->addfvar (svar, vfrom);
   float__t fvx = child->evaluate_f (sexpr);

   if (!((isnan (fvx)|| isinf(fvx)) && child->errt () == teMath))
    {
     if (isnan (fvx) || child->err[0] || !CheckChildRes (child))
      {
       errorf (pos, "%s", child->err);
       delete child;
       result_fval = qnan;
       return false;
      }
    }
   else fvx = 0;

   // For parametric plots, test second expression
   if (fidx == pl_xyplot)
    {
     float__t fvy = child->evaluate_f (sexpr_y);
     if (!((isnan (fvx) || isinf (fvx)) && child->errt () == teMath))
      {
       if (isnan (fvy) || child->err[0] || !CheckChildRes (child))
        {
         errorf (pos, "%s", child->err);
         delete child;
         result_fval = qnan;
         return false;
        }
      }
     else fvx = 0;
    }

   // Fill in the parameters
   params.sexpr   = strdup (sexpr);
   params.sexpr_y = (fidx == pl_xyplot) ? strdup (sexpr_y) : nullptr;
   params.svar    = strdup (svar);
   params.vfrom   = vfrom;
   params.vto     = vto;
   params.ymin    = fvx;
   params.ymax    = fvx;
   params.dot = false;
   params.padding = 40;

   params.grid_color = 0xC0C0C0;
   params.axis_color = 0x808080;
   params.text_color = ~params.bgc;

   // Set logarithmic flags
   params.log_x = (fidx == pl_plotlgx || fidx == pl_plotlgxy);
   params.log_y = (fidx == pl_plotlgy || fidx == pl_plotlgxy);

   // Set Z0 for Smith chart
   params.z0 = z0;

   params.child = child;
  }
 else
  {//for plotdata

   child->setFileDlgFn (FileDlgFn);
   child->evaluate_f (sexpr);
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
   strncpy (sexpr, child->get_str_res (), STRBUF - 1);
   sexpr[STRBUF - 1] = '\0';

   unquote_string (svar); // Remove quotes from mask if present
   // Fill in the parameters
   params.sexpr   = strdup (sexpr);
   params.sexpr_y = nullptr;
   params.svar    = strdup (svar);
   params.vfrom   = 0;
   params.vto     = 0;
   params.ymin    = 0;
   params.ymax    = 0;

   params.dot = (fidx == pl_plotdata);
   params.padding = 40;
   params.grid_color = 0xC0C0C0;
   params.axis_color = 0x808080;
   params.text_color = ~params.bgc;

   // Set logarithmic flags
   params.log_x = false;
   params.log_y = false;

   // Set Z0 for Smith chart
   params.z0 = 0;

   params.child = child;

  }
 return true;
}

// Plot Polar graph (specific part)
bool calculator::PlotPolar (bmpdraw *bmp, PlotParams &params)
{
 calculator *child   = params.child;
 float__t angle_from = params.vfrom;
 float__t angle_to   = params.vto;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 // Center of polar plot is always in the middle
 int center_x = width / 2;
 int center_y = height / 2;

 // Calculate appropriate step for smooth circle
 int available_size     = (width < height ? width : height) - 2 * padding;
 float__t circumference = 2.0 * M_PI * (available_size / 2);
 float__t step          = (angle_to - angle_from) / (circumference / 4.0); // ~4 pixels per step

 float__t rmax           = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find maximum radius
 float__t angle = angle_from;
 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    {
     return false;
    }

   child->addfvar (params.svar, angle);
   float__t r = child->evaluate_f (params.sexpr);

   if (!isnan (r) && isChildResReal (child))
    {
     float__t abs_r = Abs (r);
     if (first_point || abs_r > rmax)
      {
       rmax        = abs_r;
       first_point = false;
      }
    }
   angle += step;
  }
 while (angle <= angle_to);

 if (rmax < 1e-10) rmax = 1.0;

 // Add 10% padding to radius
 rmax *= 1.1;

 if (!isnan (Plot_Rmax) && PlotFunc == pl_plotpol) rmax = Plot_Rmax; // Use previous Rmax if set
 float__t plot_rmax = getfvar ("plot_ymax");
 if (!isnan (plot_rmax) && plot_rmax != 0) rmax = plot_rmax;
 Plot_Rmax = rmax; // Store Rmax for future use

 // Calculate scale: how many pixels per unit radius
 int plot_size  = (width < height ? width : height) - 2 * padding;
 float__t scale = plot_size / (2.0 * rmax);

 // Second pass: draw the curve
 angle = angle_from;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    {
     return false;
    }
   child->addfvar (params.svar, angle);
   float__t r = child->evaluate_f (params.sexpr);

   if (!isnan (r) && isChildResReal (child))
    {
     // Convert polar to Cartesian
     float__t x_cart = r * Cos (angle);
     float__t y_cart = r * Sin (angle);

     // Convert to screen coordinates (centered, Y-axis inverted)
     int x_screen = center_x + (int)(x_cart * scale);
     int y_screen = center_y - (int)(y_cart * scale);

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

   angle += step;
  }
 while (angle <= angle_to);

 // Store parameters for grid drawing
 params.ymin  = -rmax;
 params.ymax  = rmax;
 params.xmin  = -rmax;
 params.xmax  = rmax;
 params.scale = scale;

 return true;
}

// Draw polar grid and axes
void calculator::PlotDrawAxesPolar (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int center_x = width / 2;
 int center_y = height / 2;

 float__t rmax  = params.ymax; // Maximum radius in data units
 float__t scale = params.scale;

 // Draw radial grid (10 circles)
 for (int i = 1; i <= 10; i++)
  {
   float__t r_data = rmax * i / 10.0;
   int r_pixels    = (int)(r_data * scale);

   // Draw circle
   for (int angle_deg = 0; angle_deg < 360; angle_deg += 2)
    {
     float__t angle_rad = angle_deg * M_PI / 180.0;
     int x              = center_x + (int)(r_pixels * Cos (angle_rad));
     int y              = center_y - (int)(r_pixels * Sin (angle_rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      {
       bmp->drawPixel (x, y, grid_color);
      }
    }

   // Draw radius label (only for outermost circle)
   if (i == 10)
    {
     char label[32];
     d2scistr (label, (double)r_data);
     bmp->drawString (center_x + r_pixels + 5, center_y - 40, label, text_color, 0, 1);
    }
  }

 // Draw angular grid (every 30 degrees)
 int max_r_pixels = (int)(rmax * scale);
 for (int angle_deg = 0; angle_deg < 360; angle_deg += 30)
  {
   float__t angle_rad = angle_deg * M_PI / 180.0;

   // Draw radial line
   for (int r = 0; r <= max_r_pixels; r += 4)
    {
     int x = center_x + (int)(r * Cos (angle_rad));
     int y = center_y - (int)(r * Sin (angle_rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      {
       bmp->drawPixel (x, y, grid_color);
      }
    }

   // Draw angle labels
   int label_r = max_r_pixels + 15;
   int label_x = center_x + (int)(label_r * Cos (angle_rad));
   int label_y = center_y - (int)(label_r * Sin (angle_rad));

   char angle_label[16];
   sprintf (angle_label, "%d°", angle_deg);
   bmp->drawString (label_x - 10, label_y - 5, angle_label, text_color, 0, 1);
  }

 // Draw main axes (0° and 90°)
 bmp->drawLine (center_x - max_r_pixels, center_y, center_x + max_r_pixels, center_y, 1,
                axis_color);
 bmp->drawLine (center_x, center_y - max_r_pixels, center_x, center_y + max_r_pixels, 1,
                axis_color);

 // Draw title
 char title[128];
 snprintf (title, sizeof (title), "r=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (5, 5, title, text_color, 0, 2);

 // Draw variable name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}

// Plot Cartesian graph (specific part)
bool calculator::PlotCartesian (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;
 float__t ymin     = params.ymin;
 float__t ymax     = params.ymax;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 float__t step           = (vto - vfrom) / ((width - 2 * padding) / 4);
 float__t save_vfrom     = vfrom;
 bool has_valid_points   = false;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find ymin/ymax
 for (int pass = 0; pass < 2; pass++)
  {
   do
    {
     if (check_break (init_ms, last_gui_check) != brNONE)
      {
       return false;
      }

     child->addfvar (params.svar, vfrom);
     float__t fvx = child->evaluate_f (params.sexpr);
     if (isnan (fvx) && child->errt () == teSyntax)
      {
       errorf (pos, "%s", child->err);
       return false;
      }

     if (pass == 0)
      {
       if (!isnan (fvx) && isChildResReal (child))
        {
         if (fvx < ymin) ymin = fvx;
         if (fvx > ymax) ymax = fvx;
        }
      }
     else
      {
       if (!isnan (fvx) && isChildResReal (child))
        {
         float__t x = padding + ((vfrom - save_vfrom) / (vto - save_vfrom)) * (width - 2 * padding);
         float__t y = height - padding - ((fvx - ymin) / (ymax - ymin)) * (height - 2 * padding);
         if (has_valid_points)
          {
           bmp->lineTo ((int)x, (int)y, 2, fgc);
          }
         else
          {
           bmp->moveTo ((int)x, (int)y);
           has_valid_points = true;
          }
        }
       else
        has_valid_points = false;
      }
     vfrom += step;
    }
   while (vfrom <= vto);
   vfrom = save_vfrom;

   if (pass == 0)
    {
     if (PlotFunc == pl_plot || PlotFunc == pl_plotdata || PlotFunc == pl_plotdatal)
      {
       // For non-parametric plots, allow user to set Y range via plot_ymin/plot_ymax variables
       if (!isnan (Plot_Ymin)) ymin = Plot_Ymin; // Use previous values if set
       if (!isnan (Plot_Ymax)) ymax = Plot_Ymax;
      }
     float__t plot_ymax = getfvar ("plot_ymax");
     float__t plot_ymin = getfvar ("plot_ymin");
     if (!isnan(plot_ymax) && plot_ymax != 0) ymax = plot_ymax;
     if (!isnan(plot_ymin) && plot_ymin != 0) ymin = plot_ymin;
     Plot_Ymax = ymax; // Store for use in next drawing
     Plot_Ymin = ymin; // Store for use in next drawing

     // Include zero in the range
     if (ymin > 0.0) ymin = 0.0;
     if (ymax < 0.0) ymax = 0.0;

     // Add padding
     if (ymin == ymax)
      {
       ymin -= (float__t)1.0L;
       ymax += (float__t)1.0L;
      }
     else
      {
       float__t ypad = (ymax - ymin) * (float__t)0.1L;
       ymin -= ypad;
       ymax += ypad;
      }
    }
  }

 // Update parameters for drawing axes and grid
 params.ymin = ymin;
 params.ymax = ymax;
 return true;
}

// Draw axes and grid (Cartesian coordinates)
void calculator::PlotDrawAxesCartesian (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 float__t ymin       = params.ymin;
 float__t ymax       = params.ymax;
 float__t vfrom      = params.vfrom;
 float__t vto        = params.vto;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 float__t x_range = vto - vfrom;
 float__t y_range = ymax - ymin;

 int plot_width       = width - 2 * padding;
 int plot_height      = height - 2 * padding;
 int grid_step_pixels = (plot_width > plot_height ? plot_width : plot_height) / 10;

 int y_axis_pixel = height - padding - ((0.0 - ymin) / y_range) * plot_height;

 int x_axis_pixel = -1;
 if (vfrom <= 0.0 && vto >= 0.0)
  {
   x_axis_pixel = padding + ((0.0 - vfrom) / x_range) * plot_width;
  }
 
 // Horizontal grid lines
 for (int offset = 0; offset <= plot_height; offset += grid_step_pixels)
  {
   int y_up   = y_axis_pixel - offset;
   int y_down = y_axis_pixel + offset;

   if (y_up >= padding && y_up < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);

     // NEW: Label first division above zero
     if (offset == grid_step_pixels && y_up > padding + 15)
      {
       float__t y_val = ymin + ((height - padding - y_up) / (float__t)plot_height) * y_range;
       char grid_label[32];
       d2scistr (grid_label, (double)y_val);
       bmp->drawString (5, y_up - 5, grid_label, text_color, 0, 1);
      }
    }

   if (offset > 0 && y_down >= padding && y_down < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);

     // NEW: Label first division below zero
     if (offset == grid_step_pixels && y_down < height - padding - 15)
      {
       float__t y_val = ymin + ((height - padding - y_down) / (float__t)plot_height) * y_range;
       char grid_label[32];
       d2scistr (grid_label, (double)y_val);
       bmp->drawString (5, y_down - 5, grid_label, text_color, 0, 1);
      }
    }
  }

 // Vertical grid lines (when x_axis exists)
 if (x_axis_pixel >= 0)
  {
   for (int offset = 0; offset <= plot_width; offset += grid_step_pixels)
    {
     int x_left  = x_axis_pixel - offset;
     int x_right = x_axis_pixel + offset;

     if (x_left >= padding && x_left < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
      }

     if (offset > 0 && x_right >= padding && x_right < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);

       // NEW: Label first division right of zero
       if (offset == grid_step_pixels && x_right < width - padding - 30)
        {
         float__t x_val = vfrom + ((x_right - padding) / (float__t)plot_width) * (vto - vfrom);
         char grid_label[32];
         d2scistr (grid_label, (double)x_val);
         bmp->drawString (x_right - 15, height - padding + 5, grid_label, text_color, 0, 1);
        }
      }
    }
  }
 else
  {
   // No zero axis - grid every N pixels
   for (int x = padding; x < width - padding; x += grid_step_pixels)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x, y, grid_color);

     // NEW: Label first grid line
     if (x == padding + grid_step_pixels && x < width - padding - 30)
      {
       float__t x_val = vfrom + ((x - padding) / (float__t)plot_width) * (vto - vfrom);
       char grid_label[32];
       d2scistr (grid_label, (double)x_val);
       bmp->drawString (x - 15, height - padding + 5, grid_label, text_color, 0, 1);
      }
    }
  }
 // Axes
 if (x_axis_pixel >= 0)
  {
   bmp->drawLine (x_axis_pixel, padding, x_axis_pixel, height - padding, 1, axis_color);
  }
 bmp->drawLine (padding, y_axis_pixel, width - padding, y_axis_pixel, 1, axis_color);

 // Axis labels
 char label[64];

 d2scistr (label, (double)vfrom);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)vto);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 char title[128];
 snprintf (title, sizeof (title), "y=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (5, 5, title, text_color, 0, 2);
}

bool calculator::PlotParametric (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t t_from   = params.vfrom;
 float__t t_to     = params.vto;

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
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

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

 if (PlotFunc == pl_xyplot)
  {
   // For parametric plots, allow user to set X/Y range via plot_xmin/plot_xmax and
   // plot_ymin/plot_ymax variables
   if (!isnan (Plot_Xmax)) xmax = Plot_Xmax; // Use previous values if set
   if (!isnan (Plot_Xmin)) xmin = Plot_Xmin;
   if (!isnan (Plot_Ymax)) ymax = Plot_Ymax;
   if (!isnan (Plot_Ymin)) ymin = Plot_Ymin;
  }

 Plot_Xmax = xmax; // For use in next drawing
 Plot_Xmin = xmin;
 Plot_Ymax = ymax;
 Plot_Ymin = ymin;


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

 // Calculate UNIFORM scale (like in polar plots)
 float__t x_range = xmax - xmin;
 float__t y_range = ymax - ymin;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;

 float__t scale_x = plot_width / x_range;
 float__t scale_y = plot_height / y_range;

 // Use smaller scale to fit both dimensions
 float__t scale = (scale_x < scale_y) ? scale_x : scale_y;

 // Center the plot
 int center_x = width / 2;
 int center_y = height / 2;

 float__t x_center = (xmin + xmax) / 2.0;
 float__t y_center = (ymin + ymax) / 2.0;

 // Second pass: draw the curve
 t                     = t_from;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, t);

   float__t x   = child->evaluate_f (params.sexpr);
   bool x_valid = true;
   if (isnan (x) && child->errt () == teMath)
    x = 0;
   else if (isnan (x) || !isChildResReal (child))
    x_valid = false;

   float__t y   = child->evaluate_f (params.sexpr_y);
   bool y_valid = true;
   if (isnan (y) && child->errt () == teMath)
    y = 0;
   else if (isnan (y) || !isChildResReal (child))
    y_valid = false;

   if (x_valid && y_valid)
    {
     // Use uniform scale and center
     int x_screen = center_x + (int)((x - x_center) * scale);
     int y_screen = center_y - (int)((y - y_center) * scale);

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
 params.xmin  = xmin;
 params.xmax  = xmax;
 params.ymin  = ymin;
 params.ymax  = ymax;
 params.scale = scale;

 return true;
}

void calculator::PlotDrawAxesParametric (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 float__t ymin       = params.ymin;
 float__t ymax       = params.ymax;
 float__t xmin       = params.xmin;
 float__t xmax       = params.xmax;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int plot_width       = width - 2 * padding;
 int plot_height      = height - 2 * padding;
 int grid_step_pixels = (plot_width > plot_height ? plot_width : plot_height) / 10;

 int center_x = width / 2;
 int center_y = height / 2;

 float__t x_center = (xmin + xmax) / 2.0;
 float__t y_center = (ymin + ymax) / 2.0;
 float__t scale    = params.scale;

 // Calculate axis positions using same formula as point drawing
 int y_axis_pixel = center_x + (int)((0.0 - x_center) * scale); // X=0 position
 int x_axis_pixel = center_y - (int)((0.0 - y_center) * scale); // Y=0 position

 // FIX: Draw grid centered on data center (not canvas center)
 // Horizontal grid lines (centered on Y=0 axis, not canvas center)
 for (int offset = 0; offset <= plot_height / 2; offset += grid_step_pixels)
  {
   int y_up   = x_axis_pixel - offset;
   int y_down = x_axis_pixel + offset;

   if (y_up >= padding && y_up < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);
    }

   if (offset > 0 && y_down >= padding && y_down < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);
    }
  }

 // Vertical grid lines (centered on X=0 axis, not canvas center)
 for (int offset = 0; offset <= plot_width / 2; offset += grid_step_pixels)
  {
   int x_left  = y_axis_pixel - offset;
   int x_right = y_axis_pixel + offset;

   if (x_left >= padding && x_left < width - padding)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
    }

   if (offset > 0 && x_right >= padding && x_right < width - padding)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);
    }
  }

 // Draw axes (at zero crossings)
 if (x_axis_pixel >= padding && x_axis_pixel < height - padding)
  {
   bmp->drawLine (padding, x_axis_pixel, width - padding, x_axis_pixel, 1, axis_color);
  }
 if (y_axis_pixel >= padding && y_axis_pixel < width - padding)
  {
   bmp->drawLine (y_axis_pixel, padding, y_axis_pixel, height - padding, 1, axis_color);
  }

 // Axis labels
 char label[64];

 d2scistr (label, (double)xmin);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)xmax);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 // Draw parameter name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 // Draw titles for both functions (on two lines or shortened)
 char title_x[128];
 char title_y[128];

 // Shorten expressions if too long
 int max_len = 30;
 if ((int)strlen (params.sexpr) > max_len)
  snprintf (title_x, sizeof (title_x), "x=%.27s...", params.sexpr);
 else
  snprintf (title_x, sizeof (title_x), "x=%s", params.sexpr);

 if ((int)strlen (params.sexpr_y) > max_len)
  snprintf (title_y, sizeof (title_y), "y=%.27s...", params.sexpr_y);
 else
  snprintf (title_y, sizeof (title_y), "y=%s", params.sexpr_y);

 title_x[sizeof (title_x) - 1] = '\0';
 title_y[sizeof (title_y) - 1] = '\0';

 // Draw on two lines
 bmp->drawString (5, 5, title_x, text_color, 0, 1);
 bmp->drawString (5, 17, title_y, text_color, 0, 1);
}

// PlotLogarithmic:
bool calculator::PlotLogarithmic (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 bool log_x = params.log_x;
 bool log_y = params.log_y;

 // Calculate step
 float__t step = (vto - vfrom) / ((width - 2 * padding) * 10);

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find min/max, skipping invalid values
 float__t x = vfrom;
 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, x);
   float__t y = child->evaluate_f (params.sexpr);

   // Skip NaN and complex results
   if (!(isnan (y) && child->errt () == teMath))
    {
     if (isnan (y) || !isChildResReal (child))
      {
       x += step;
       continue;
      }
    }
   else
    y = 0;

   // Skip invalid values for logarithmic axes
   if ((log_x && x <= 0) || (log_y && y <= 0))
    {
     x += step;
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

   x += step;
  }
 while (x <= vto);

 if (first_point)
  {
   errorf (pos, "No valid points to plot");
   return false;
  }

 // Add padding (in linear or log space)
 if (log_x)
  {
   float__t log_range = Lg (xmax) - Lg (xmin);
   float__t pad       = log_range * 0.1;
   xmin               = Pow (10.0, Lg (xmin) - pad);
   xmax               = Pow (10.0, Lg (xmax) + pad);
  }
 else
  {
   if (xmin == xmax)
    {
     xmin -= 1.0;
     xmax += 1.0;
    }
   else
    {
     float__t x_pad = (xmax - xmin) * 0.1;
     xmin -= x_pad;
     xmax += x_pad;
    }
   // Include zero in range for linear axis
   if (xmin > 0.0) xmin = 0.0;
   if (xmax < 0.0) xmax = 0.0;
  }

 if (log_y)
  {
   float__t log_range = Lg (ymax) - Lg (ymin);
   float__t pad       = log_range * 0.1;
   ymin               = Pow (10.0, Lg (ymin) - pad);
   ymax               = Pow (10.0, Lg (ymax) + pad);
  }
 else
  {
   if (ymin == ymax)
    {
     ymin -= 1.0;
     ymax += 1.0;
    }
   else
    {
     float__t y_pad = (ymax - ymin) * 0.1;
     ymin -= y_pad;
     ymax += y_pad;
    }
   // Include zero in range for linear axis
   if (ymin > 0.0) ymin = 0.0;
   if (ymax < 0.0) ymax = 0.0;
  }

 if (PlotFunc == pl_plotlgx || PlotFunc == pl_plotlgy || PlotFunc == pl_plotlgxy)
  {
   // For non-parametric plots, allow user to set Y range via plot_ymin/plot_ymax variables
   if (!isnan (Plot_Ymin)) ymin = Plot_Ymin; // Use previous values if set
   if (!isnan (Plot_Ymax)) ymax = Plot_Ymax;
  }
 float__t plot_ymax = getfvar ("plot_ymax");
 float__t plot_ymin = getfvar ("plot_ymin");
 if (!isnan (plot_ymax) && plot_ymax != 0) ymax = plot_ymax;
 if (!isnan (plot_ymin) && plot_ymin != 0) ymin = plot_ymin;
 Plot_Ymin = ymin; // Store for use in next drawing
 Plot_Ymax = ymax;


 // Second pass: draw the curve
 x = vfrom;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, x);
   float__t y = child->evaluate_f (params.sexpr);

   bool valid = true;
   if (isnan (y) && child->errt () == teMath)
    y = 0;
   else if (isnan (y) || !isChildResReal (child))
    valid = false;

   if ((log_x && x <= 0) || (log_y && y <= 0)) valid = false;

   if (valid)
    {
     int x_screen, y_screen;

     if (log_x)
      {
       float__t log_x_norm = (Lg (x) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
       x_screen            = padding + (int)(log_x_norm * (width - 2 * padding));
      }
     else
      {
       x_screen = padding + (int)(((x - xmin) / (xmax - xmin)) * (width - 2 * padding));
      }

     if (log_y)
      {
       float__t log_y_norm = (Lg (y) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
       y_screen            = height - padding - (int)(log_y_norm * (height - 2 * padding));
      }
     else
      {
       y_screen = height - padding - (int)(((y - ymin) / (ymax - ymin)) * (height - 2 * padding));
      }

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

   x += step;
  }
 while (x <= vto);

 // Store parameters for axis drawing
 params.xmin = xmin;
 params.xmax = xmax;
 params.ymin = ymin;
 params.ymax = ymax;

 return true;
}

// PlotDrawAxesLog:
// Replace PlotDrawAxesLog function - remove lambda functions
void calculator::PlotDrawAxesLog (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 float__t ymin       = params.ymin;
 float__t ymax       = params.ymax;
 float__t xmin       = params.xmin;
 float__t xmax       = params.xmax;
 bool log_x          = params.log_x;
 bool log_y          = params.log_y;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;

 // Draw Y axis grid
 if (log_y)
  {
   // Logarithmic Y grid
   int decade_min = (int)Floor (Lg (ymin));
   int decade_max = (int)Ceil (Lg (ymax));

   for (int decade = decade_min; decade <= decade_max; decade++)
    {
     float__t base = Pow (10.0, decade);

     // Major grid line at each decade (1, 10, 100, ...)
     if (base >= ymin && base <= ymax)
      {
       // Convert Y to screen coordinate (logarithmic)
       float__t log_norm = (Lg (base) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
       int y_screen      = height - padding - (int)(log_norm * plot_height);

        // Draw major grid line
       if (y_screen >= padding && y_screen < height - padding)
        {
         for (int x = padding; x < width - padding; x++) bmp->drawPixel (x, y_screen, axis_color);
        }
        // NEW: Add label on the left side (skip if too close to edges)
        if (y_screen > padding + 15 && y_screen < height - padding - 15)
         {
          char grid_label[32];
          d2scistr (grid_label, (double)base);
          bmp->drawString (5, y_screen - 5, grid_label, text_color, 0, 1);
         }
       }

     // Minor grid lines (2, 3, 4, 5, 6, 7, 8, 9)
     for (int minor = 2; minor <= 9; minor++)
      {
       float__t value = base * minor;
       if (value >= ymin && value <= ymax)
        {
         // Convert Y to screen coordinate (logarithmic)
         float__t log_norm = (Lg (value) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
         int y_screen      = height - padding - (int)(log_norm * plot_height);

         if (y_screen >= padding && y_screen < height - padding)
          {
           for (int x = padding; x < width - padding; x += 4)
            bmp->drawPixel (x, y_screen, grid_color);
          }
        }
      }
    }
  }
 else
  {
   // Linear Y grid
   int grid_step_pixels = plot_height / 10;

   // Convert Y=0 to screen coordinate (linear)
   int y_axis_pixel = height - padding - (int)(((0.0 - ymin) / (ymax - ymin)) * plot_height);

   for (int offset = 0; offset <= plot_height; offset += grid_step_pixels)
    {
     int y_up   = y_axis_pixel - offset;
     int y_down = y_axis_pixel + offset;

     if (y_up >= padding && y_up < height - padding)
      {
       for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);
      }

     if (offset > 0 && y_down >= padding && y_down < height - padding)
      {
       for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);
      }
    }
  }

 // Draw X axis grid
 if (log_x)
  {
   // Logarithmic X grid
   int decade_min = (int)Floor (Lg (xmin));
   int decade_max = (int)Ceil (Lg (xmax));

   for (int decade = decade_min; decade <= decade_max; decade++)
    {
     float__t base = Pow (10.0, decade);

     // Major grid line at each decade
     if (base >= xmin && base <= xmax)
      {
       // Convert X to screen coordinate (logarithmic)
       float__t log_norm = (Lg (base) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
       int x_screen      = padding + (int)(log_norm * plot_width);

       if (x_screen >= padding && x_screen < width - padding)
        {
         // Draw major grid line
         for (int y = padding; y < height - padding; y++) bmp->drawPixel (x_screen, y, axis_color);

         // NEW: Add label on the bottom (skip if too close to edges)
         if (x_screen > padding + 30 && x_screen < width - padding - 30)
          {
           char grid_label[32];
           d2scistr (grid_label, (double)base);
           bmp->drawString (x_screen - 15, height - padding + 5, grid_label, text_color, 0, 1);
          }
        }

      }

     // Minor grid lines
     for (int minor = 2; minor <= 9; minor++)
      {
       float__t value = base * minor;
       if (value >= xmin && value <= xmax)
        {
         // Convert X to screen coordinate (logarithmic)
         float__t log_norm = (Lg (value) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
         int x_screen      = padding + (int)(log_norm * plot_width);

         if (x_screen >= padding && x_screen < width - padding)
          {
           for (int y = padding; y < height - padding; y += 4)
            bmp->drawPixel (x_screen, y, grid_color);
          }
        }
      }
    }
  }
 else
  {
   // Linear X grid
   int grid_step_pixels = plot_width / 10;

   // Convert X=0 to screen coordinate (linear)
   int x_axis_pixel = padding + (int)(((0.0 - xmin) / (xmax - xmin)) * plot_width);

   for (int offset = 0; offset <= plot_width; offset += grid_step_pixels)
    {
     int x_left  = x_axis_pixel - offset;
     int x_right = x_axis_pixel + offset;

     if (x_left >= padding && x_left < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
      }

     if (offset > 0 && x_right >= padding && x_right < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);
      }
    }
  }

 // Draw main axes
 // Calculate axis positions
 int x_axis_pixel;
 int y_axis_pixel;

 if (log_y)
  {
   float__t log_norm = (Lg (0.0) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
   x_axis_pixel      = height - padding - (int)(log_norm * plot_height);
  }
 else
  {
   x_axis_pixel = height - padding - (int)(((0.0 - ymin) / (ymax - ymin)) * plot_height);
  }

 if (log_x)
  {
   float__t log_norm = (Lg (0.0) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
   y_axis_pixel      = padding + (int)(log_norm * plot_width);
  }
 else
  {
   y_axis_pixel = padding + (int)(((0.0 - xmin) / (xmax - xmin)) * plot_width);
  }

 if (x_axis_pixel >= padding && x_axis_pixel < height - padding)
  {
   bmp->drawLine (padding, x_axis_pixel, width - padding, x_axis_pixel, 1, axis_color);
  }
 if (y_axis_pixel >= padding && y_axis_pixel < width - padding)
  {
   bmp->drawLine (y_axis_pixel, padding, y_axis_pixel, height - padding, 1, axis_color);
  }

 // Axis labels
 char label[64];

 d2scistr (label, (double)xmin);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)xmax);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 // Variable name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 // Title
 char title[128];
 snprintf (title, sizeof (title), "y=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 //bmp->drawString (width / 2 - 50, 5, title, text_color, 0, 2);
 bmp->drawString (10, 5, title, text_color, 0, 2);
}

// PlotSmith with frequency labels:
bool calculator::PlotSmith (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;
 float__t z0       = params.z0;

 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 uint32_t fgc        = params.fgc;
 uint32_t text_color = params.text_color;

 // Center of Smith chart
 int center_x = width / 2;
 int center_y = height / 2;

 // Calculate radius (use smaller dimension)
 int plot_size = (width < height ? width : height) - 2 * padding;
 int radius    = plot_size / 2;

 // Calculate step for smooth curve
 float__t step           = (vto - vfrom) / (radius * 50);
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // Storage for axis crossings
 struct AxisCrossing
 {
  float__t freq;
  int x, y;
  bool is_real_axis; // true = real axis, false = imaginary axis
 };

 AxisCrossing crossings[100];
 int crossing_count = 0;

 // Previous gamma values for detecting crossings
 float__t prev_gamma_re = 0, prev_gamma_im = 0;
 bool has_prev = false;

 // Draw the impedance trace
 float__t param        = vfrom;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, param);

   // Evaluate impedance (can be complex)
   child->evaluate_f (params.sexpr);

   float__t z_re = child->get_re_res ();
   float__t z_im = child->get_im_res ();

   // Check for valid result
   bool valid = true;
   if (isnan (z_re) && child->errt () == teMath)
    {
     param += step;
     continue;
    }
   else if (isnan (z_re))
    {
     valid = false;
    }

   if (valid)
    {
     // Calculate reflection coefficient: Γ = (Z - Z0) / (Z + Z0)
     float__t z_norm_re = z_re / z0;
     float__t z_norm_im = z_im / z0;

     // Γ = (Z/Z0 - 1) / (Z/Z0 + 1)
     float__t num_re = z_norm_re - 1.0;
     float__t num_im = z_norm_im;
     float__t den_re = z_norm_re + 1.0;
     float__t den_im = z_norm_im;

     // Complex division: (num_re + i*num_im) / (den_re + i*den_im)
     float__t den_mag2 = den_re * den_re + den_im * den_im;

     if (den_mag2 < 1e-15)
      {
       param += step;
       continue;
      }

     float__t gamma_re = (num_re * den_re + num_im * den_im) / den_mag2;
     float__t gamma_im = (num_im * den_re - num_re * den_im) / den_mag2;

     // Check if Γ is within unit circle
     float__t gamma_mag2 = gamma_re * gamma_re + gamma_im * gamma_im;
     if (gamma_mag2 > 1.0)
      {
       param += step;
       continue;
      }

     // Convert Γ to screen coordinates
     int x_screen = center_x + (int)(gamma_re * radius);
     int y_screen = center_y - (int)(gamma_im * radius);

     // Detect axis crossings
     if (has_prev && crossing_count < 100)
      {
       // Real axis crossing (gamma_im changes sign)
       if ((prev_gamma_im < 0 && gamma_im >= 0) || (prev_gamma_im > 0 && gamma_im <= 0))
        {
         // Linear interpolation to find exact crossing point
         float__t t              = Abs (prev_gamma_im) / (Abs (prev_gamma_im) + Abs (gamma_im));
         float__t cross_freq     = param - step + t * step;
         float__t cross_gamma_re = prev_gamma_re + t * (gamma_re - prev_gamma_re);

         crossings[crossing_count].freq         = cross_freq;
         crossings[crossing_count].x            = center_x + (int)(cross_gamma_re * radius);
         crossings[crossing_count].y            = center_y;
         crossings[crossing_count].is_real_axis = true;
         crossing_count++;
        }

       // Imaginary axis crossing (gamma_re changes sign)
       if ((prev_gamma_re < 0 && gamma_re >= 0) || (prev_gamma_re > 0 && gamma_re <= 0))
        {
         // Linear interpolation to find exact crossing point
         float__t t              = Abs (prev_gamma_re) / (Abs (prev_gamma_re) + Abs (gamma_re));
         float__t cross_freq     = param - step + t * step;
         float__t cross_gamma_im = prev_gamma_im + t * (gamma_im - prev_gamma_im);

         crossings[crossing_count].freq         = cross_freq;
         crossings[crossing_count].x            = center_x;
         crossings[crossing_count].y            = center_y - (int)(cross_gamma_im * radius);
         crossings[crossing_count].is_real_axis = false;
         crossing_count++;
        }
      }

     prev_gamma_re = gamma_re;
     prev_gamma_im = gamma_im;
     has_prev      = true;

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
     has_prev         = false;
    }

   param += step;
  }
 while (param <= vto);

 // Draw markers at axis crossings
 for (int i = 0; i < crossing_count; i++)
  {
   int mx = crossings[i].x;
   int my = crossings[i].y;

   // Draw small circle marker
   for (int angle = 0; angle < 360; angle += 30)
    {
     float__t rad = angle * M_PI / 180.0;
     int dx       = (int)(4 * Cos (rad));
     int dy       = (int)(4 * Sin (rad));
     bmp->drawPixel (mx + dx, my + dy, text_color);
    }

   // Format frequency label
   char label[64];
   float__t freq = crossings[i].freq;

   if (freq >= 1e9)
    sprintf (label, "%.2fG", (double)(freq / 1e9));
   else if (freq >= 1e6)
    sprintf (label, "%.1fM", (double)(freq / 1e6));
   else if (freq >= 1e3)
    sprintf (label, "%.1fk", (double)(freq / 1e3));
   else
    sprintf (label, "%.0f", (double)freq);

   // Position label (offset from marker)
   int label_x = mx + 8;
   int label_y = my - 8;

   // Adjust label position to avoid going off-screen
   if (label_x > width - 60) label_x = mx - 50;
   if (label_y < 20) label_y = my + 15;

   // NEW: Avoid Z0 label area (top-right corner)
   if (mx > center_x + radius - 70 && my > center_y - 40 && my < center_y - 5)
    {
     label_y = my + 15; // Move label below marker instead of above
    }

   bmp->drawString (label_x, label_y, label, text_color, 0, 1);
  }

 // Store parameters for grid drawing
 params.scale = (float__t)radius;

 return true;
}

//PlotDrawAxesSmith - drawing Smith chart grid:
void calculator::PlotDrawAxesSmith (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;
 float__t z0         = params.z0;

 int center_x = width / 2;
 int center_y = height / 2;
 int radius   = (int)params.scale;

 // Draw outer circle (|Γ| = 1)
 for (int angle = 0; angle < 360; angle++)
  {
   float__t rad = angle * M_PI / 180.0;
   int x        = center_x + (int)(radius * Cos (rad));
   int y        = center_y - (int)(radius * Sin (rad));
   bmp->drawPixel (x, y, axis_color);
  }

 // Draw horizontal axis (real axis of Γ)
 bmp->drawLine (center_x - radius, center_y, center_x + radius, center_y, 1, axis_color);

 // Constant resistance circles (normalized r = R/Z0)
 // Draw circles for r = 0.2, 0.5, 1.0, 2.0, 5.0
 float__t r_values[] = { 0.2, 0.5, 1.0, 2.0, 5.0 };
 for (int i = 0; i < 5; i++)
  {
   float__t r = r_values[i];

   // Circle center and radius for constant resistance
   // Center: (r/(1+r), 0) in Γ plane
   // Radius: 1/(1+r)
   float__t circle_center_x = r / (1.0 + r);
   float__t circle_radius   = 1.0 / (1.0 + r);

   int screen_center_x = center_x + (int)(circle_center_x * radius);
   int screen_radius   = (int)(circle_radius * radius);

   // Draw circle (upper and lower halves)
   for (int angle = 0; angle < 360; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x        = screen_center_x + (int)(screen_radius * Cos (rad));
     int y        = center_y - (int)(screen_radius * Sin (rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      bmp->drawPixel (x, y, grid_color);
    }
  }

 // Constant reactance arcs (normalized x = X/Z0)
 // Draw arcs for x = ±0.2, ±0.5, ±1.0, ±2.0, ±5.0
 float__t x_values[] = { 0.2, 0.5, 1.0, 2.0, 5.0 };
 for (int i = 0; i < 5; i++)
  {
   float__t x = x_values[i];

   // Arc center and radius for constant reactance
   // Center: (1, 1/x) in Γ plane
   // Radius: 1/x
   float__t arc_center_x     = 1.0;
   float__t arc_center_y_pos = 1.0 / x;
   float__t arc_center_y_neg = -1.0 / x;
   float__t arc_radius       = 1.0 / x;

   int screen_center_x = center_x + (int)(arc_center_x * radius);

   // Positive reactance (inductive, upper half)
   int screen_center_y_pos = center_y - (int)(arc_center_y_pos * radius);
   int screen_arc_radius   = (int)(arc_radius * radius);

   // Draw arc from left intersection to right edge
   for (int angle = 90; angle <= 270; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x_pos    = screen_center_x + (int)(screen_arc_radius * Cos (rad));
     int y_pos    = screen_center_y_pos - (int)(screen_arc_radius * Sin (rad));

     // Check if inside outer circle
     int dx = x_pos - center_x;
     int dy = y_pos - center_y;
     if (dx * dx + dy * dy <= radius * radius)
      {
       if (x_pos >= padding && x_pos < width - padding && y_pos >= padding
           && y_pos < height - padding)
        bmp->drawPixel (x_pos, y_pos, grid_color);
      }
    }

   // Negative reactance (capacitive, lower half)
   int screen_center_y_neg = center_y - (int)(arc_center_y_neg * radius);

   for (int angle = 90; angle <= 270; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x_neg    = screen_center_x + (int)(screen_arc_radius * Cos (rad));
     int y_neg    = screen_center_y_neg - (int)(screen_arc_radius * Sin (rad));

     // Check if inside outer circle
     int dx = x_neg - center_x;
     int dy = y_neg - center_y;
     if (dx * dx + dy * dy <= radius * radius)
      {
       if (x_neg >= padding && x_neg < width - padding && y_neg >= padding
           && y_neg < height - padding)
        bmp->drawPixel (x_neg, y_neg, grid_color);
      }
    }
  }

 // Add labels
 char label[64];

 // Z0 label
 sprintf (label, "Z0=%.0fΩ", (double)z0);
 bmp->drawString (center_x + radius + 5, center_y - 30, label, text_color, 0, 1);

 // Open circuit (right edge)
 bmp->drawString (center_x + radius - 20, center_y + 5, "∞", text_color, 0, 1);

 // Short circuit (left edge)
 bmp->drawString (center_x - radius + 5, center_y + 5, "0", text_color, 0, 1);

 // Matched load (center)
 bmp->drawString (center_x - 10, center_y + 15, "Z0", text_color, 0, 1);

 // Title
 char title[128];
 snprintf (title, sizeof (title), "Z=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (5, 5, title, text_color, 0, 2);

 // Parameter name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}

// Plot data from file (two-pass like PlotCartesian)
bool calculator::PlotData (bmpdraw *bmp, PlotParams &params)
{
 const char *datafile = params.sexpr; // data file name
 const char *mask     = params.svar;  // mask (can be empty or nullptr)

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;
 bool dots    = params.dot; // points or lines

 // Determine the number of parameters in the mask
 int param_count = scanmasknum (mask);
 if (param_count > 2)
  {
   errorf (pos, "Mask has more than 2 parameters (max is 2: x and y)");
   return false;
  }
 if (param_count == 0) param_count = 1; // If no mask - one parameter

 // Normalize file path
 char fnamebuf[STRBUF] = { 0 };
 NormalizePath (datafile, fnamebuf, STRBUF);

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point = true;
 int line_number  = 0;

 // First pass: find min/max
 FILE *f = fopen (fnamebuf, "r");
 if (!f)
  {
   errorf (pos, "Cannot open data file: %s", fnamebuf);
   return false;
  }

 char line[1024];
 while (fgets (line, sizeof (line), f))
  {
   line_number++;

   float__t x, y;

   if (param_count == 1)
    {
     // One parameter: X = line number, Y = value
     double dy   = qnan;
     int scanned = strscan (line, mask, 1, &dy);

     if (scanned < 1) continue; // Not enough data in the line
     x = (float__t)line_number;
     y = (float__t)dy;
    }
   else // param_count == 2
    {
     // Two parameters: X = first value, Y = second value
     double dx = qnan, dy = qnan;
     int scanned = strscan (line, mask, 2, &dx, &dy);

     if (scanned < 2) continue; // Not enough data in the line
     x = (float__t)dx;
     y = (float__t)dy;
    }

   // Update boundaries
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
  }

 fclose (f);


 if (PlotFunc == pl_plotdatal || PlotFunc == pl_plotdata || PlotFunc == pl_plot)
  {
   if (!isnan (Plot_Ymin)) ymin = Plot_Ymin; // Use previous values if set
   if (!isnan (Plot_Ymax)) ymax = Plot_Ymax;
  }
 float__t plot_ymax = getfvar ("plot_ymax");
 float__t plot_ymin = getfvar ("plot_ymin");
 if (!isnan(plot_ymax) && plot_ymax != 0) ymax = plot_ymax;
 if (!isnan(plot_ymin) && plot_ymin != 0) ymin = plot_ymin;
 Plot_Xmax = xmax; // For use in next drawing
 Plot_Xmin = xmin;
 Plot_Ymax = ymax;
 Plot_Ymin = ymin;


 if (first_point)
  {
   errorf (pos, "No valid data points found in file");
   return false;
  }

 // Add padding (as in PlotCartesian)
 if (xmin == xmax)
  {
   xmin -= (float__t)1.0L;
   xmax += (float__t)1.0L;
  }
 else
  {
   float__t xpad = (xmax - xmin) * (float__t)0.1L;
   xmin -= xpad;
   xmax += xpad;
  }

 if (ymin == ymax)
  {
   ymin -= (float__t)1.0L;
   ymax += (float__t)1.0L;
  }
 else
  {
   float__t ypad = (ymax - ymin) * (float__t)0.1L;
   ymin -= ypad;
   ymax += ypad;
  }

 // Include zero in Y range
 if (ymin > 0.0) ymin = 0.0;
 if (ymax < 0.0) ymax = 0.0;

 // Calculate ranges for screen mapping
 float__t x_range = xmax - xmin;
 float__t y_range = ymax - ymin;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;

 // Second pass: draw points/lines
 f = fopen (fnamebuf, "r");
 if (!f)
  {
   errorf (pos, "Cannot reopen data file");
   return false;
  }

 line_number           = 0;
 bool has_valid_points = false;

 while (fgets (line, sizeof (line), f))
  {
   line_number++;

   float__t x, y;

   if (param_count == 1)
    {
     double dy   = qnan;
     int scanned = strscan (line, mask, 1, &dy);

     if (scanned < 1) continue;

     x = (float__t)line_number;
     y = (float__t)dy;
    }
   else // param_count == 2
    {
     double dx = qnan, dy = qnan;
     int scanned = strscan (line, mask, 2, &dx, &dy);

     if (scanned < 2) continue;

     x = (float__t)dx;
     y = (float__t)dy;
    }

   // Convert to screen coordinates
   int x_screen = padding + (int)(((x - xmin) / x_range) * plot_width);
   int y_screen = height - padding - (int)(((y - ymin) / y_range) * plot_height);

   if (dots)
    {
     // Draw dot (circle)
     int point_size = params.pxsize;
     for (int dy = -point_size; dy <= point_size; dy++)
      {
       for (int dx = -point_size; dx <= point_size; dx++)
        {
         if (dx * dx + dy * dy <= point_size * point_size) // Circle
          {
           int px = x_screen + dx;
           int py = y_screen + dy;
           if (px >= padding && px < width - padding && py >= padding && py < height - padding)
            {
             bmp->drawPixel (px, py, fgc);
            }
          }
        }
      }
    }
   else
    {
     // Draw line
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
  }

 fclose (f);

 // Store parameters for grid drawing
 params.xmin  = xmin;
 params.xmax  = xmax;
 params.ymin  = ymin;
 params.ymax  = ymax;
 params.vfrom = xmin;
 params.vto   = xmax;

 return true;
}


// Main plotting function
bool calculator::Plot (const char *expr, v_func fidx, value &res)
{
 res.sval = nullptr;
 PlotParams params;
 memset (&params, 0, sizeof (params));

 // 1. Prepare data
 if (!PlotPrepare (expr, fidx, params))
  {
   return false;
  }


  // 2. Create or load bitmap
 bmpdraw *bmp = new bmpdraw ();
 bool is_polar = (fidx == pl_plotpol);
 bool is_parametric = (fidx == pl_xyplot);
 bool is_logarithmic = (fidx == pl_plotlgx ||  fidx == pl_plotlgy || fidx == pl_plotlgxy);
 bool is_smith = (fidx == pl_plotsmith || fidx == pl_plotsmithz);
 bool is_data = (fidx == pl_plotdata || fidx == pl_plotdatal);

 // Normal plot or fplot — create new bitmap
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

 bmp->top = params.top;
 bmp->left = params.left;

 // 3. Draw plot (Cartesian or Polar coordinates)
 bool plot_success;
 if (is_data)
  {
   plot_success = PlotData (bmp, params);
  }
 else
 if (is_smith)
  {
   plot_success = PlotSmith (bmp, params);
  }
 else
 if (is_logarithmic)
  {
   plot_success = PlotLogarithmic (bmp, params);
  }
 else
 if (is_parametric)
  {
   plot_success = PlotParametric (bmp, params);
  }
 else
 if (is_polar)
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
   if (params.sexpr) free (params.sexpr);
   if (params.sexpr_y) free (params.sexpr_y);
   if (params.svar) free (params.svar);
   result_fval = qnan;
   return false;
  }

 // 4. Draw axes and grid (skip for overlay mode)
 // 
  {
   if (is_smith)
    {
     PlotDrawAxesSmith (bmp, params);
    }
   else 
   if (is_logarithmic)
    {
     PlotDrawAxesLog (bmp, params);
    }
   else
   if (is_parametric)
    {
     PlotDrawAxesParametric (bmp, params);
    }
   else 
   if (is_polar)
    {
     PlotDrawAxesPolar (bmp, params);
    }
   else
    {
     PlotDrawAxesCartesian (bmp, params);
    }
  }

// 5. Save or display
// Show in GUI
 {
  res.tag = tvBMP;
  res.sval = (char *)bmp; // Pass bitmap pointer to GUI for display
  register_mem (res.sval, ptBMP);
 }

 // 6. Cleanup
 if (!res.sval) delete bmp;
 fflags |= params.child->isfflags ();
 delete params.child;
 if (params.sexpr) free (params.sexpr);
 if (params.sexpr_y) free (params.sexpr_y);
 if (params.svar) free (params.svar);

 if (PlotFunc == vf_num) PlotFunc = fidx;

 return true;
}


void calculator::PlotRegion(float__t x_min, float__t x_max, float__t y_min,
    float__t y_max) // Set the plotting region for the plot operator
{
 Plot_Xmin = x_min;
 Plot_Xmax = x_max;
 Plot_Ymin = y_min;
 Plot_Ymax = y_max;
 Plot_Tmin = x_min;
 Plot_Tmax = x_max;
 PlotFunc  = pl_any; // Reset plot function to allow new plot type
 addfvar ("plot_ymax", y_max);
 addfvar ("plot_ymin", y_min);
}

void calculator::PlotReset() // Reset plot settings to defaults
{
 Plot_Ymax = qnan;   // Reset the maximum Y value for plotting
 Plot_Ymin = qnan;   // Reset the minimum Y value for plotting
 Plot_Xmax = qnan;   // Reset the maximum X value for plotting
 Plot_Xmin = qnan;   // Reset the minimum X value for plotting
 Plot_Tmax = qnan;   // Reset the maximum T value for plotting
 Plot_Tmin = qnan;   // Reset the minimum T value for plotting
 Plot_Rmax = qnan;   // Reset the maximum R value for plotting
 PlotFunc  = vf_num; // Reset the plotting function
}

// Overlay two bitmaps: copy only specified color from bmp2 to bmp1
// This preserves grid and axes from bmp1, overlaying only the plot curve
bool calculator::AddBmp (bmpdraw *bmp1, bmpdraw *bmp2, uint32_t fg_color)
{
 if (!bmp1 || !bmp2)
  {
   errorf (pos, "AddBmp: null bitmap pointer");
   return false;
  }

 if (!bmp1->data || !bmp2->data)
  {
   errorf (pos, "AddBmp: bitmap data is null");
   return false;
  }

 // Check dimensions
 if (bmp1->width != bmp2->width || bmp1->height != bmp2->height)
  {
   errorf (pos, "AddBmp: bitmap dimensions mismatch (%dx%d vs %dx%d)", bmp1->width, bmp1->height,
           bmp2->width, bmp2->height);
   return false;
  }

 // Check row sizes
 if (bmp1->rowSize != bmp2->rowSize)
  {
   errorf (pos, "AddBmp: bitmap row sizes mismatch");
   return false;
  }

 // Extract RGB components from fg_color
 uint8_t fg_r = (fg_color >> 16) & 0xFF;
 uint8_t fg_g = (fg_color >> 8) & 0xFF;
 uint8_t fg_b = fg_color & 0xFF;

 // Overlay only pixels matching fg_color from bmp2 to bmp1
 for (int y = 0; y < bmp1->height; y++)
  {
   for (int x = 0; x < bmp1->width; x++)
    {
     int offset = y * bmp1->rowSize + x * 3;

     // Read pixel from bmp2
     uint8_t b2 = bmp2->data[offset];
     uint8_t g2 = bmp2->data[offset + 1];
     uint8_t r2 = bmp2->data[offset + 2];

     // If pixel matches fg_color, copy it to bmp1
     if (r2 == fg_r && g2 == fg_g && b2 == fg_b)
      {
       bmp1->data[offset]     = b2;
       bmp1->data[offset + 1] = g2;
       bmp1->data[offset + 2] = r2;
      }
    }
  }

 return true;
}

#pragma endregion

#pragma region WAV

// Create WAV file in memory from expression
bool calculator::CreateWav (char *sexpr, char *svar, float__t vfrom, float__t vto,
                            calculator *child, value &res)
{
 const uint32_t SAMPLE_RATE     = 44100;
 const uint16_t BITS_PER_SAMPLE = 16;
 const uint16_t NUM_CHANNELS    = 1; // mono

 // Calculate number of samples
 float__t duration = vto - vfrom; // duration in seconds
 if (duration <= 0)
  {
   errorf (pos, "Invalid time range for WAV generation");
   return false;
  }

 uint32_t numSamples = (uint32_t)(duration * SAMPLE_RATE);
 if (numSamples == 0)
  {
   errorf (pos, "Duration too short for WAV generation");
   return false;
  }

 // Allocate memory for WAV file
 uint32_t dataSize = numSamples * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
 uint32_t fileSize = sizeof (WavHeader) + dataSize;

 char *wavData = (char *)malloc (fileSize);
 if (!wavData)
  {
   errorf (pos, "Out of memory for WAV generation");
   return false;
  }

 // Initialize WAV header
 WavHeader *header = (WavHeader *)wavData;
 memcpy (header->riff, "RIFF", 4);
 header->fileSize = fileSize - 8;
 memcpy (header->wave, "WAVE", 4);
 memcpy (header->fmt, "fmt ", 4);
 header->fmtSize       = 16;
 header->audioFormat   = 1; // PCM
 header->numChannels   = NUM_CHANNELS;
 header->sampleRate    = SAMPLE_RATE;
 header->bitsPerSample = BITS_PER_SAMPLE;
 header->byteRate      = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
 header->blockAlign    = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
 memcpy (header->data, "data", 4);
 header->dataSize = dataSize;

 int16_t *samples = (int16_t *)(wavData + sizeof (WavHeader));

 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 float__t maxAmplitude = 0.0;
 float__t save_vfrom   = vfrom;

 // First pass: find maximum amplitude (sample every 2nd point for speed - 22kHz)
 float__t step_pass1 = duration / (numSamples / 2);
 float__t t          = vfrom;

 for (uint32_t i = 0; i < numSamples / 2; i++)
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    {
     free (wavData);
     return false;
    }

   child->addfvar (svar, t);
   float__t fvx = child->evaluate_f (sexpr);

   if (isnan (fvx) && child->errt () == teSyntax)
    {
     errorf (pos, "%s", child->err);
     free (wavData);
     return false;
    }

   // Check if result is real (not complex)
   if (!isnan (fvx) && isChildResReal (child))
    {
     float__t absVal = Abs (fvx);
     if (absVal > maxAmplitude) maxAmplitude = absVal;
    }
   // TODO: Check if result goes into complex plane for future stereo WAV support

   t += step_pass1;
  }

 // Avoid division by zero
 if (maxAmplitude == 0.0) maxAmplitude = 1.0;

 // Second pass: generate samples at full 44100 Hz
 float__t step_pass2 = duration / numSamples;
 t                   = save_vfrom;

 for (uint32_t i = 0; i < numSamples; i++)
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    {
     free (wavData);
     return false;
    }

   child->addfvar (svar, t);
   float__t fvx = child->evaluate_f (sexpr);

   if (isnan (fvx) && child->errt () == teSyntax)
    {
     errorf (pos, "%s", child->err);
     free (wavData);
     return false;
    }

   // Normalize and convert to 16-bit PCM
   int16_t sample = 0;
   if (!isnan (fvx) && isChildResReal (child))
    {
     // Normalize to [-1, 1] range, then scale to 16-bit range
     float__t normalized = fvx / maxAmplitude;
     // Clamp to [-1, 1]
     if (normalized > 1.0) normalized = 1.0;
     if (normalized < -1.0) normalized = -1.0;

     sample = (int16_t)(normalized * 32767.0);
    }

   samples[i] = sample;
   t += step_pass2;
  }

 // Set result
 res.tag  = tvWAV;
 res.sval = wavData;
 res.imval = (float__t)0.0L;
 res.fval  = (float__t)0.0L;
 res.ival  = 0;
 register_mem (res.sval, ptMALLOC);

 return true;
}

// Utility function to get WAV file size from header
inline uint32_t GetWavFileSize (const char *wavData)
{
 WavHeader *header = (WavHeader *)wavData;
 return header->fileSize + 8; // fileSize not includes the first 8 bytes (RIFF + size)
}

// Utility function to get number of samples from WAV header
inline uint32_t GetWavNumSamples (const char *wavData)
{
 WavHeader *header = (WavHeader *)wavData;
 return header->dataSize / (header->numChannels * header->bitsPerSample / 8);
}

// Play function
bool calculator::Play (const char *expr, v_func fidx, value &res)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return false;
  }

 char sexpr[STRBUF]   = { '\0' };
 char sfrom[MAXOP]    = { '\0' };
 char sto[MAXOP]      = { '\0' };
 char svar[STRBUF]    = { '\0' };

 bool split_ok     = false;

 split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
 if (!split_ok)
  {
   result_fval = qnan;
   return false;
  }
 calculator *child = new calculator (scfg | SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return false;
  }
 float__t vfrom = qnan, vto = qnan;
 float__t fvx = qnan;
 if (sfrom[0]) vfrom = child->evaluate_f (sfrom);
 if (isnan (vfrom) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 if (sto[0]) vto = child->evaluate_f (sto);
 if (isnan (vto) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }
 if (!isname (svar))
  {
   errorf (pos, "Invalid variable name");
   delete child;
   result_fval = qnan;
   return false;
  }
 child->addfvar (svar, vfrom);
 fvx = child->evaluate_f (sexpr); // evaluate the function for
                                           // the syntax check before starting the integration
 if ((isnan (fvx) || isinf (fvx)) && child->errt () == teMath)
  {
   fvx = 0; 
  }
 else if (isnan (fvx) || child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   result_fval = qnan;
   return false;
  }

 return CreateWav (sexpr, svar, vfrom, vto, child, res);
}

// WAV operations helper functions

// Mix two WAV samples with clipping
int16_t MixSamples (int16_t a, int16_t b)
{
 int32_t mixed = (int32_t)a + (int32_t)b;
 if (mixed > 32767) mixed = 32767;
 if (mixed < -32768) mixed = -32768;
 return (int16_t)mixed;
}

// Subtract WAV samples with clipping
int16_t SubSamples (int16_t a, int16_t b)
{
 int32_t result = (int32_t)a - (int32_t)b;
 if (result > 32767) result = 32767;
 if (result < -32768) result = -32768;
 return (int16_t)result;
}

// Multiply WAV samples (normalized) with clipping
int16_t MulSamples (int16_t a, int16_t b)
{
 int32_t result = ((int32_t)a * (int32_t)b) / 32768;
 if (result > 32767) result = 32767;
 if (result < -32768) result = -32768;
 return (int16_t)result;
}

// Concatenate two WAV files
char *ConcatenateWav (const char *wav1, const char *wav2)
{
 WavHeader *h1 = (WavHeader *)wav1;
 WavHeader *h2 = (WavHeader *)wav2;

 // Check compatibility
 if (h1->sampleRate != h2->sampleRate || h1->numChannels != h2->numChannels
     || h1->bitsPerSample != h2->bitsPerSample)
  return nullptr; // Incompatible WAV files

 uint32_t samples1     = h1->dataSize / (h1->numChannels * h1->bitsPerSample / 8);
 uint32_t samples2     = h2->dataSize / (h2->numChannels * h2->bitsPerSample / 8);
 uint32_t totalSamples = samples1 + samples2;

 // Allocate new WAV
 uint32_t newDataSize = totalSamples * h1->numChannels * (h1->bitsPerSample / 8);
 uint32_t newFileSize = sizeof (WavHeader) + newDataSize;

 char *newWav = (char *)malloc (newFileSize);
 if (!newWav) return nullptr;

 // Copy header from first WAV
 memcpy (newWav, wav1, (int_t)sizeof (WavHeader));
 WavHeader *newHeader = (WavHeader *)newWav;
 newHeader->fileSize  = newFileSize - 8;
 newHeader->dataSize  = newDataSize;

 // Copy samples from both WAVs
 int16_t *newSamples  = (int16_t *)(newWav + sizeof (WavHeader));
 int16_t *samples1ptr = (int16_t *)(wav1 + sizeof (WavHeader));
 int16_t *samples2ptr = (int16_t *)(wav2 + sizeof (WavHeader));

 memcpy (newSamples, samples1ptr, h1->dataSize);
 memcpy (newSamples + samples1, samples2ptr, h2->dataSize);

 return newWav;
}

// Add normalized scalar to WAV sample with clipping
int16_t AddScalar (int16_t sample, float__t value)
{
 // value is in range [-1.0, 1.0], scale to [-32768, 32767]
 int32_t offset = (int32_t)(value * 32767.0);
 int32_t result = (int32_t)sample + offset;
 if (result > 32767) result = 32767;
 if (result < -32768) result = -32768;
 return (int16_t)result;
}

// Scale WAV sample by scalar with clipping
int16_t ScaleSample (int16_t sample, float__t scale)
{
 int32_t result = (int32_t)(sample * scale);
 if (result > 32767) result = 32767;
 if (result < -32768) result = -32768;
 return (int16_t)result;
}

// Main function to perform WAV operations based on the operator
bool calculator::WavOp (value &left, value &right, t_operator cop)
{
 if (left.tag != tvWAV && right.tag != tvWAV) return false; // At least one must be WAV

 // WAV | WAV - concatenation
 if (cop == toOR && left.tag == tvWAV && right.tag == tvWAV)
  {
   char *result = ConcatenateWav (left.sval, right.sval);
   if (!result)
    {
     errorf (left.pos, "Incompatible WAV files for concatenation");
     return false;
    }

   // Free old left value and assign new
   sf_free (left.sval, ptMALLOC);
   left.sval = result;
   register_mem (left.sval, ptMALLOC);
   left.tag = tvWAV;
   left.imval = (float__t)0.0L;
   left.fval  = (float__t)0.0L;
   left.ival  = 0;
   return true;
  }

 // WAV op WAV - element-wise operations
 if (left.tag == tvWAV && right.tag == tvWAV)
  {
   WavHeader *h1 = (WavHeader *)left.sval;
   WavHeader *h2 = (WavHeader *)right.sval;

   // Check compatibility
   if (h1->sampleRate != h2->sampleRate || h1->numChannels != h2->numChannels
       || h1->bitsPerSample != h2->bitsPerSample)
    {
     errorf (left.pos, "Incompatible WAV files for operation");
     return false;
    }

   uint32_t samples1   = h1->dataSize / (h1->numChannels * h1->bitsPerSample / 8);
   uint32_t samples2   = h2->dataSize / (h2->numChannels * h2->bitsPerSample / 8);
   uint32_t minSamples = samples1 < samples2 ? samples1 : samples2;

   // Use shorter length for result
   uint32_t newDataSize = minSamples * h1->numChannels * (h1->bitsPerSample / 8);
   uint32_t newFileSize = sizeof (WavHeader) + newDataSize;

   char *newWav = (char *)malloc (newFileSize);
   if (!newWav)
    {
     errorf (left.pos, "Out of memory for WAV operation");
     return false;
    }

   // Copy header
   memcpy (newWav, left.sval, (int_t)sizeof (WavHeader));
   WavHeader *newHeader = (WavHeader *)newWav;
   newHeader->fileSize  = newFileSize - 8;
   newHeader->dataSize  = newDataSize;

   int16_t *newSamples  = (int16_t *)(newWav + sizeof (WavHeader));
   int16_t *samples1ptr = (int16_t *)(left.sval + sizeof (WavHeader));
   int16_t *samples2ptr = (int16_t *)(right.sval + sizeof (WavHeader));

   // Perform operation
   for (uint32_t i = 0; i < minSamples; i++)
    {
     switch (cop)
      {
      case toADD:
       newSamples[i] = MixSamples (samples1ptr[i], samples2ptr[i]);
       break;
      case toSUB:
       newSamples[i] = SubSamples (samples1ptr[i], samples2ptr[i]);
       break;
      case toMUL:
       newSamples[i] = MulSamples (samples1ptr[i], samples2ptr[i]);
       break;
      default:
       free (newWav);
       return false;
      }
    }

   // Free old and assign new
   sf_free (left.sval, ptMALLOC);
   left.sval = newWav;
   register_mem (left.sval, ptMALLOC);
   left.tag = tvWAV;
   left.imval = (float__t)0.0L;
   left.fval  = (float__t)0.0L;
   left.ival  = 0;
   return true;
  }

 // WAV op scalar
 if (left.tag == tvWAV && ((right.tag & (t_value)~MSK_SCALAR) == 0))
  {
   WavHeader *h1    = (WavHeader *)left.sval;
   uint32_t samples = h1->dataSize / (h1->numChannels * h1->bitsPerSample / 8);

   float__t scale  = 0.0;
   float__t offset = 0.0;

   if (cop == toMUL)
    scale = right.get ();
   else if (cop == toDIV)
    {
     if (right.get () == 0.0)
      {
       errorf (left.pos, "Division by zero in WAV operation");
       return false;
      }
     scale = 1.0 / right.get ();
    }
   else if (cop == toADD)
    offset = right.get ();
   else if (cop == toSUB)
    offset = -right.get ();
   else
    return false; // Unsupported operator

   // Create new WAV
   uint32_t fileSize = h1->fileSize + 8;
   char *newWav      = (char *)malloc (fileSize);
   if (!newWav)
    {
     errorf (left.pos, "Out of memory for WAV operation");
     return false;
    }

   // Copy header
   memcpy (newWav, left.sval, (int_t)sizeof (WavHeader));

   int16_t *newSamples  = (int16_t *)(newWav + sizeof (WavHeader));
   int16_t *samples1ptr = (int16_t *)(left.sval + sizeof (WavHeader));

   // Apply operation to all samples
   for (uint32_t i = 0; i < samples; i++)
    {
     if (cop == toMUL || cop == toDIV)
      newSamples[i] = ScaleSample (samples1ptr[i], scale);
     else // toADD or toSUB
      newSamples[i] = AddScalar (samples1ptr[i], offset);
    }

   // Free old and assign new
   sf_free (left.sval, ptMALLOC);
   left.sval = newWav;
   register_mem (left.sval, ptMALLOC);
   left.tag = tvWAV;
   left.imval = (float__t)0.0L;
   left.fval  = (float__t)0.0L;
   left.ival  = 0;
   return true;
  }

 // scalar op WAV
 if (right.tag == tvWAV && ((left.tag & (t_value)~MSK_SCALAR) == 0))
  {
   WavHeader *h1    = (WavHeader *)right.sval;
   uint32_t samples = h1->dataSize / (h1->numChannels * h1->bitsPerSample / 8);

   float__t scale  = 0.0;
   float__t offset = 0.0;

   if (cop == toMUL)
    {
     // Commutative: scalar * WAV = WAV * scalar
     value temp = left;
     left       = right;
     right      = temp;
     return WavOp (left, right, cop);
    }
   else if (cop == toADD)
    {
     // Commutative: scalar + WAV = WAV + scalar
     value temp = left;
     left       = right;
     right      = temp;
     return WavOp (left, right, cop);
    }
   else if (cop == toSUB)
    {
     // scalar - WAV: invert and add
     offset = left.get ();
     scale  = -1.0;
    }
   else if (cop == toDIV)
    {
     // scalar / WAV: invert samples (reciprocal operation - experimental)
     // This is mathematically questionable for audio, but we can implement it
     scale = left.get ();
    }
   else
    return false;

   // Create new WAV
   uint32_t fileSize = h1->fileSize + 8;
   char *newWav      = (char *)malloc (fileSize);
   if (!newWav)
    {
     errorf (left.pos, "Out of memory for WAV operation");
     return false;
    }

   // Copy header
   memcpy (newWav, right.sval, (int_t)sizeof (WavHeader));

   int16_t *newSamples  = (int16_t *)(newWav + sizeof (WavHeader));
   int16_t *samples1ptr = (int16_t *)(right.sval + sizeof (WavHeader));

   // Apply operation to all samples
   for (uint32_t i = 0; i < samples; i++)
    {
     if (cop == toSUB)
      {
       // scalar - WAV[i] = scalar + (-WAV[i])
       int16_t inverted = ScaleSample (samples1ptr[i], scale);
       newSamples[i]    = AddScalar (inverted, offset);
      }
     else if (cop == toDIV)
      {
       // scalar / WAV[i]: treat each sample as divisor
       if (samples1ptr[i] != 0)
        {
         float__t reciprocal = scale / (float__t)samples1ptr[i] * 32768.0;
         newSamples[i]       = ScaleSample (32767, reciprocal / 32767.0);
        }
       else
        newSamples[i] = 0; // Avoid division by zero
      }
    }

   // Assign to left
   left.sval = newWav;
   register_mem (left.sval, ptMALLOC);
   left.tag = tvWAV;
   left.imval = (float__t)0.0L;
   left.fval  = (float__t)0.0L;
   left.ival  = 0;
   return true;
  }

 return false;
}

// Simple FFT implementation (Cooley-Tukey algorithm)
// real and imag arrays must have size n (power of 2)
// inverse = false for forward FFT, true for inverse FFT
void PerformFFT (float__t *real, float__t *imag, int n, bool inverse)
{
 if (n <= 1) return;

 // Bit-reversal permutation
 int j = 0;
 for (int i = 0; i < n - 1; i++)
  {
   if (i < j)
    {
     float__t temp = real[i];
     real[i]       = real[j];
     real[j]       = temp;
     temp          = imag[i];
     imag[i]       = imag[j];
     imag[j]       = temp;
    }
   int k = n / 2;
   while (k <= j)
    {
     j -= k;
     k /= 2;
    }
   j += k;
  }

 // Cooley-Tukey FFT
 for (int len = 2; len <= n; len *= 2)
  {
   float__t angle  = (inverse ? 2.0L : -2.0L) * M_PI / len;
   float__t wlen_r = Cos (angle);
   float__t wlen_i = Sin (angle);

   for (int i = 0; i < n; i += len)
    {
     float__t w_r = 1.0L;
     float__t w_i = 0.0L;

     for (int j = 0; j < len / 2; j++)
      {
       float__t u_r = real[i + j];
       float__t u_i = imag[i + j];
       float__t v_r = real[i + j + len / 2] * w_r - imag[i + j + len / 2] * w_i;
       float__t v_i = real[i + j + len / 2] * w_i + imag[i + j + len / 2] * w_r;

       real[i + j]           = u_r + v_r;
       imag[i + j]           = u_i + v_i;
       real[i + j + len / 2] = u_r - v_r;
       imag[i + j + len / 2] = u_i - v_i;

       float__t temp = w_r;
       w_r           = w_r * wlen_r - w_i * wlen_i;
       w_i           = temp * wlen_i + w_i * wlen_r;
      }
    }
  }

 if (inverse)
  {
   for (int i = 0; i < n; i++)
    {
     real[i] /= n;
     imag[i] /= n;
    }
  }
}

// Find next power of 2
uint32_t NextPowerOf2 (uint32_t n)
{
 uint32_t power = 1;
 while (power < n) power *= 2;
 return power;
}

// Analyze WAV and extract top harmonics into matrix [frequency, amplitude]
bool calculator::WavFFT (value &wavVal, value &res)
{
 if (wavVal.tag != tvWAV || !wavVal.sval)
  {
   errorf (pos, "Expected WAV object for FFT");
   return false;
  }

 WavHeader *header   = (WavHeader *)wavVal.sval;
 uint32_t numSamples = header->dataSize / (header->numChannels * header->bitsPerSample / 8);
 uint32_t sampleRate = header->sampleRate;
 int16_t *samples    = (int16_t *)(wavVal.sval + sizeof (WavHeader));

 // Find FFT size (power of 2, limit to 32768 for reasonable performance)
 uint32_t fftSize = NextPowerOf2 (numSamples);
 if (fftSize > 32768) fftSize = 32768;
 if (fftSize < 64) fftSize = 64;

 // Allocate FFT buffers
 float__t *real = (float__t *)malloc (fftSize * sizeof (float__t));
 float__t *imag = (float__t *)malloc (fftSize * sizeof (float__t));

 if (!real || !imag)
  {
   if (real) free (real);
   if (imag) free (imag);
   errorf (pos, "Out of memory for FFT");
   return false;
  }

 // Copy samples to real part, normalize to [-1, 1]
 uint32_t copySize = numSamples < fftSize ? numSamples : fftSize;
 for (uint32_t i = 0; i < copySize; i++)
  {
   real[i] = (float__t)samples[i] / 32768.0L;
   imag[i] = 0.0L;
  }

 // Zero-pad if needed
 for (uint32_t i = copySize; i < fftSize; i++)
  {
   real[i] = 0.0L;
   imag[i] = 0.0L;
  }

 // Perform FFT
 PerformFFT (real, imag, fftSize, false);

 // Find top harmonics (peaks in magnitude spectrum)
 struct Peak
 {
  uint32_t index;
  float__t frequency;
  float__t magnitude;
 };

 Peak peaks[7];
 for (int i = 0; i < 7; i++)
  {
   peaks[i].index     = 0;
   peaks[i].frequency = 0.0L;
   peaks[i].magnitude = 0.0L;
  }

 // Only analyze positive frequencies (first half of spectrum)
 // Skip DC component (index 0)
 for (uint32_t i = 1; i < fftSize / 2; i++)
  {
   float__t mag = Sqrt (real[i] * real[i] + imag[i] * imag[i]);

   // Check if this is a local peak
   bool isPeak = false;
   if (i == 1)
    {
     float__t next = Sqrt (real[i + 1] * real[i + 1] + imag[i + 1] * imag[i + 1]);
     isPeak        = (mag > next);
    }
   else if (i == fftSize / 2 - 1)
    {
     float__t prev = Sqrt (real[i - 1] * real[i - 1] + imag[i - 1] * imag[i - 1]);
     isPeak        = (mag > prev);
    }
   else
    {
     float__t prev = Sqrt (real[i - 1] * real[i - 1] + imag[i - 1] * imag[i - 1]);
     float__t next = Sqrt (real[i + 1] * real[i + 1] + imag[i + 1] * imag[i + 1]);
     isPeak        = (mag > prev && mag > next);
    }

   if (isPeak && mag > 0.01L) // Threshold to ignore noise
    {
     // Calculate frequency
     float__t frequency = (float__t)i * sampleRate / fftSize;

     // Try to insert into top 7
     for (int j = 0; j < 7; j++)
      {
       if (mag > peaks[j].magnitude)
        {
         // Shift down
         for (int k = 6; k > j; k--) peaks[k] = peaks[k - 1];

         peaks[j].index     = i;
         peaks[j].frequency = frequency;
         peaks[j].magnitude = mag;
         break;
        }
      }
    }
  }

 free (real);
 free (imag);

 // Count actual peaks found
 int peakCount = 0;
 for (int i = 0; i < 7; i++)
  {
   if (peaks[i].magnitude > 0.0L) peakCount++;
  }

 if (peakCount == 0)
  {
   errorf (pos, "No significant harmonics found in WAV");
   return false;
  }

 // Create result matrix [frequency, amplitude]
 res.tag   = tvMATRIX;
 res.mrows = peakCount;
 res.mcols = 2;
 res.mval  = (float__t *)malloc (peakCount * 2 * sizeof (float__t));

 if (!res.mval)
  {
   errorf (pos, "Out of memory for result matrix");
   return false;
  }

 register_mem (res.mval, ptMALLOC);

 for (int i = 0; i < peakCount; i++)
  {
   // Normalize amplitude (multiply by 2 because we only look at positive frequencies)
   float__t amplitude = peaks[i].magnitude * 2.0L / fftSize;

   res.mval[i * 2 + 0] = peaks[i].frequency;
   res.mval[i * 2 + 1] = amplitude;
  }

 res.fval = (float__t)peakCount;
 res.ival = peakCount;

 return true;
}

// Synthesize WAV from harmonics matrix [frequency, amplitude] or [frequency, amplitude, phase]
// Duration in seconds
bool calculator::HarmonicsToWav (value &harmonics, float__t duration, value &res)
{
 if (harmonics.tag != tvMATRIX)
  {
   errorf (pos, "Expected matrix with harmonics");
   return false;
  }

 if (harmonics.mcols < 2 || harmonics.mcols > 3)
  {
   errorf (pos, "Matrix must have 2 or 3 columns");
   return false;
  }

 if (harmonics.mrows < 1 || harmonics.mrows > 7)
  {
   errorf (pos, "Matrix must have 1 to 7 rows");
   return false;
  }

 if (duration <= 0.0L)
  {
   errorf (pos, "Duration must be positive");
   return false;
  }

 const uint32_t SAMPLE_RATE     = 44100;
 const uint16_t BITS_PER_SAMPLE = 16;
 const uint16_t NUM_CHANNELS    = 1;

 uint32_t numSamples = (uint32_t)(duration * SAMPLE_RATE);

 if (numSamples == 0)
  {
   errorf (pos, "Duration too short");
   return false;
  }

 bool hasPhase = (harmonics.mcols == 3);

 // First pass: find maximum absolute value
 float__t maxAbs = 0.0L;

 for (uint32_t i = 0; i < numSamples; i++)
  {
   float__t t     = (float__t)i / SAMPLE_RATE;
   float__t value = 0.0L;

   // Sum all harmonics
   for (int h = 0; h < harmonics.mrows; h++)
    {
     float__t freq  = harmonics.mval[h * harmonics.mcols + 0];
     float__t amp   = harmonics.mval[h * harmonics.mcols + 1];
     float__t phase = hasPhase ? harmonics.mval[h * harmonics.mcols + 2] : 0.0L;

     value += amp * Sin (2.0L * M_PI * freq * t + phase);
    }

   float__t absVal = Abs (value);
   if (absVal > maxAbs) maxAbs = absVal;
  }

 // Determine normalization scale
 float__t scale = (maxAbs > 1.0L) ? (1.0L / maxAbs) : 1.0L;

 // Allocate WAV
 uint32_t dataSize = numSamples * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
 uint32_t fileSize = sizeof (WavHeader) + dataSize;

 char *wavData = (char *)malloc (fileSize);
 if (!wavData)
  {
   errorf (pos, "Out of memory for WAV synthesis");
   return false;
  }

 // Setup WAV header
 WavHeader *header = (WavHeader *)wavData;
 memcpy (header->riff, "RIFF", 4);
 header->fileSize = fileSize - 8;
 memcpy (header->wave, "WAVE", 4);
 memcpy (header->fmt, "fmt ", 4);
 header->fmtSize       = 16;
 header->audioFormat   = 1;
 header->numChannels   = NUM_CHANNELS;
 header->sampleRate    = SAMPLE_RATE;
 header->bitsPerSample = BITS_PER_SAMPLE;
 header->byteRate      = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
 header->blockAlign    = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
 memcpy (header->data, "data", 4);
 header->dataSize = dataSize;

 int16_t *samples = (int16_t *)(wavData + sizeof (WavHeader));

 // Second pass: compute and write normalized samples
 for (uint32_t i = 0; i < numSamples; i++)
  {
   float__t t     = (float__t)i / SAMPLE_RATE;
   float__t value = 0.0L;

   for (int h = 0; h < harmonics.mrows; h++)
    {
     float__t freq  = harmonics.mval[h * harmonics.mcols + 0];
     float__t amp   = harmonics.mval[h * harmonics.mcols + 1];
     float__t phase = hasPhase ? harmonics.mval[h * harmonics.mcols + 2] : 0.0L;

     value += amp * Sin (2.0L * M_PI * freq * t + phase);
    }

   value *= scale;

   // Clamp to [-1, 1]
   if (value > 1.0L) value = 1.0L;
   if (value < -1.0L) value = -1.0L;

   samples[i] = (int16_t)(value * 32767.0L);
  }

 res.tag  = tvWAV;
 res.sval = wavData;
 res.fval  = (float__t)0.0L;
 res.imval = (float__t)0.0L;
 res.ival  = 0;
 register_mem (res.sval, ptMALLOC);

 return true;
}

// Evaluate harmonic sum at given time t (in seconds)
// harmonics - matrix [frequency, amplitude] or [frequency, amplitude, phase]
// t - time in seconds
// Returns signal value at time t
float__t calculator::EvalHarmonics (value &harmonics, float__t t)
{
 if (harmonics.tag != tvMATRIX)
  {
   errorf (pos, "Expected matrix with harmonics");
   return qnan;
  }

 if (harmonics.mcols < 2 || harmonics.mcols > 3)
  {
   errorf (pos, "Harmonics matrix must have 2 or 3 columns [frequency, amplitude] or [frequency, "
                "amplitude, phase]");
   return qnan;
  }

 if (isnan (t) || isinf (t)) return qnan;

 bool hasPhase  = (harmonics.mcols == 3);
 float__t value = 0.0L;

 // Sum all harmonics: value = Σ(amplitude * sin(2π * frequency * t + phase))
 for (int h = 0; h < harmonics.mrows; h++)
  {
   float__t freq  = harmonics.mval[h * harmonics.mcols + 0];
   float__t amp   = harmonics.mval[h * harmonics.mcols + 1];
   float__t phase = hasPhase ? harmonics.mval[h * harmonics.mcols + 2] : 0.0L;

   value += amp * Sin (2.0L * M_PI * freq * t + phase);
  }

 return value;
}

// Evaluate harmonics at complex time t
// If t is real (imval == 0), uses fast real path
// Returns false on error
bool calculator::EvalHarmonics (value &harmonics, value &t, value &res)
{
 if (harmonics.tag != tvMATRIX)
  {
   errorf (pos, "Expected matrix with harmonics");
   return false;
  }

 if (harmonics.mcols < 2 || harmonics.mcols > 3)
  {
   errorf (pos, "Harmonics matrix must have 2 or 3 columns");
   return false;
  }

 // Check if t has imaginary component first
 if (t.imval != (float__t)0.0L || t.tag == tvCOMPLEX)
  {
   // Complex path: sin(2π·f·t + φ) with complex t
   // sin(z) where z = 2π·f·t + φ (complex)

   bool hasPhase   = (harmonics.mcols == 3);
   float__t sum_re = 0.0L;
   float__t sum_im = 0.0L;

   for (int h = 0; h < harmonics.mrows; h++)
    {
     float__t freq  = harmonics.mval[h * harmonics.mcols + 0];
     float__t amp   = harmonics.mval[h * harmonics.mcols + 1];
     float__t phase = hasPhase ? harmonics.mval[h * harmonics.mcols + 2] : 0.0L;

     // Calculate z = 2π·f·t + φ
     // t is complex: t = fval + i·imval
     // 2π·f·t = 2π·f·(fval + i·imval) = 2π·f·fval + i·2π·f·imval
     float__t z_re = 2.0L * M_PI * freq * t.fval + phase;
     float__t z_im = 2.0L * M_PI * freq * t.imval;

     // sin(z) for complex z
     float__t sin_re, sin_im;
     SinC (z_re, z_im, sin_re, sin_im);

     // Multiply by amplitude (real scalar)
     sum_re += amp * sin_re;
     sum_im += amp * sin_im;
    }

   res.tag   = tvCOMPLEX;
   res.fval  = sum_re;
   res.imval = sum_im;
   res.ival  = (int_t)sum_re;
   return true;
  }
 else
  {
   // Fast real path
   if (isnan (t.fval) || isinf (t.fval))
    {
     res.tag   = tvFLOAT;
     res.fval  = qnan;
     res.imval = (float__t)0.0L;
     res.ival  = 0;
     return true;
    }

   float__t result = EvalHarmonics (harmonics, t.fval);
   res.tag         = tvFLOAT;
   res.fval        = result;
   res.imval       = (float__t)0.0L;
   res.ival        = (int_t)result;
   return true;
  }
}


// Get normalized sample value from WAV at time t (in seconds)
// Returns value in range [-1, 1], or 0 if t is out of bounds
float__t calculator::EvalWav (value &wavVal, float__t t)
{
 if (wavVal.tag != tvWAV || !wavVal.sval)
  {
   errorf (pos, "Expected WAV object");
   return qnan;
  }

 if (isnan (t) || isinf (t)) return qnan;

 WavHeader *header   = (WavHeader *)wavVal.sval;
 uint32_t numSamples = header->dataSize / (header->numChannels * header->bitsPerSample / 8);
 uint32_t sampleRate = header->sampleRate;
 int16_t *samples    = (int16_t *)(wavVal.sval + sizeof (WavHeader));

 // Calculate sample index from time
 float__t sampleIndexF = t * sampleRate;

 // Check bounds
 if (sampleIndexF < 0.0L || sampleIndexF >= (float__t)numSamples)
  return 0.0L; // Out of bounds - return silence

 // Get sample index (with linear interpolation for better quality)
 uint32_t index    = (uint32_t)sampleIndexF;
 float__t fraction = sampleIndexF - index;

 // Get current sample, normalized to [-1, 1]
 float__t value = (float__t)samples[index] / 32768.0L;

 // Linear interpolation with next sample if available
 if (fraction > 0.0L && index + 1 < numSamples)
  {
   float__t nextValue = (float__t)samples[index + 1] / 32768.0L;
   value              = value * (1.0L - fraction) + nextValue * fraction;
  }

 return value;
}

// FFT Plot: displays waveform (top) and spectrum (bottom) on single BMP
bool calculator::FFTPlot (value &wavVal, value &res)
{
 if (wavVal.tag != tvWAV || !wavVal.sval)
  {
   errorf (pos, "Expected WAV object for FFT plot");
   return false;
  }

 WavHeader *header   = (WavHeader *)wavVal.sval;
 uint32_t numSamples = header->dataSize / (header->numChannels * header->bitsPerSample / 8);
 uint32_t sampleRate = header->sampleRate;
 float__t duration   = (float__t)numSamples / sampleRate;

 // Perform FFT to get harmonics
 value harmonics;
 if (!WavFFT (wavVal, harmonics)) return false;

 // Get fundamental frequency (first harmonic)
 float__t fundamental = 0.0L;
 if (harmonics.mrows > 0) fundamental = harmonics.mval[0]; // First column of first row

 if (fundamental <= 0.0L) fundamental = 440.0L; // Default if can't determine

 // Calculate period and display duration (3-4 periods)
 float__t period      = 1.0L / fundamental;
 float__t displayTime = period * 3.5L; // 3.5 periods
 if (displayTime > duration) displayTime = duration;
 if (displayTime < 0.001L) displayTime = 0.01L; // At least 10ms

 // Get plot settings from variables
 int bgc   = (int)getivar ("plot_bgc");
 int fgc   = (int)getivar ("plot_fgc");
 int width = (int)getivar ("plot_width");
 if ((width <= 100) || (width > 2000)) width = 800;
 int height = (int)getivar ("plot_height");
 if ((height <= 100) || (height > 2000)) height = 600;
 int top = (int)getivar ("plot_top");
 if (top < 0 || top > 2000) top = 0;
 int left = (int)getivar ("plot_left");
 if (left < 0 || left > 2000) left = 0;

 int padding         = 40;
 uint32_t grid_color = 0xC0C0C0;
 uint32_t axis_color = 0x808080;
 uint32_t text_color = ~bgc;

 // Create bitmap
 bmpdraw *bmp = new bmpdraw ();
 if (!bmp || !bmp->newbmp (width, height, bgc))
  {
   if (bmp) delete bmp;
   freevar (harmonics);
   errorf (pos, "Failed to create bitmap");
   return false;
  }

 bmp->top  = top;
 bmp->left = left;

 int topHeight = height / 2; // Upper half for waveform

 // === Draw Waveform (Top) ===
 {
  int plotTop    = padding;
  int plotBottom = topHeight - padding / 2;
  int plotLeft   = padding;
  int plotRight  = width - padding;
  int plotHeight = plotBottom - plotTop;
  int plotWidth  = plotRight - plotLeft;

  // Draw grid (dotted lines)
  for (int i = 0; i <= 10; i++)
   {
    int y = plotTop + (plotHeight * i) / 10;
    for (int x = plotLeft; x < plotRight; x += 4) bmp->drawPixel (x, y, grid_color);
   }
  for (int i = 0; i <= 10; i++)
   {
    int x = plotLeft + (plotWidth * i) / 10;
    for (int y = plotTop; y < plotBottom; y += 4) bmp->drawPixel (x, y, grid_color);
   }

  // Draw axes
  int centerY = (plotTop + plotBottom) / 2;
  bmp->drawLine (plotLeft, centerY, plotRight, centerY, 1, axis_color);
  bmp->drawLine (plotLeft, plotTop, plotLeft, plotBottom, 1, axis_color);

  // Draw waveform
  bool hasValidPoint = false;

  for (int x = 0; x <= plotWidth; x++)
   {
    float__t t     = (float__t)x / plotWidth * displayTime;
    float__t value = EvalWav (wavVal, t);

    // Map value [-1, 1] to y coordinate
    int yPos = centerY - (int)(value * plotHeight / 2);
    if (yPos < plotTop) yPos = plotTop;
    if (yPos > plotBottom) yPos = plotBottom;

    int xPos = plotLeft + x;

    if (hasValidPoint)
     bmp->lineTo (xPos, yPos, 2, fgc);
    else
     bmp->moveTo (xPos, yPos);

    hasValidPoint = true;
   }

  // Draw title
  char title[128];
  sprintf (title, "Waveform (%.3f ms, %.1f Hz)", (double)(displayTime * 1000.0L),
           (double)fundamental);
  bmp->drawString (plotLeft + 10, plotTop + 10, title, text_color, 0, 1);
 }

 // === Draw Spectrum (Bottom) ===
 {
  int plotTop    = topHeight + padding / 2;
  int plotBottom = height - padding;
  int plotLeft   = padding;
  int plotRight  = width - padding;
  int plotHeight = plotBottom - plotTop;
  int plotWidth  = plotRight - plotLeft;

  // Find max frequency and amplitude for scaling
  float__t maxFreq = 0.0L;
  float__t maxAmp  = 0.0L;

  for (int i = 0; i < harmonics.mrows; i++)
   {
    float__t freq = harmonics.mval[i * harmonics.mcols + 0];
    float__t amp  = harmonics.mval[i * harmonics.mcols + 1];
    if (freq > maxFreq) maxFreq = freq;
    if (amp > maxAmp) maxAmp = amp;
   }

  if (maxFreq == 0.0L) maxFreq = sampleRate / 2.0L;
  if (maxAmp == 0.0L) maxAmp = 1.0L;

  // Limit frequency range to reasonable values
  if (maxFreq > 15000.0L) maxFreq = 15000.0L;

  // Draw grid (dotted lines)
  for (int i = 0; i <= 10; i++)
   {
    int y = plotTop + (plotHeight * i) / 10;
    for (int x = plotLeft; x < plotRight; x += 4) bmp->drawPixel (x, y, grid_color);
   }
  for (int i = 0; i <= 10; i++)
   {
    int x = plotLeft + (plotWidth * i) / 10;
    for (int y = plotTop; y < plotBottom; y += 4) bmp->drawPixel (x, y, grid_color);
   }

  // Draw axes
  bmp->drawLine (plotLeft, plotBottom, plotRight, plotBottom, 1, axis_color);
  bmp->drawLine (plotLeft, plotTop, plotLeft, plotBottom, 1, axis_color);

  // Draw spectrum bars
  for (int i = 0; i < harmonics.mrows; i++)
   {
    float__t freq = harmonics.mval[i * harmonics.mcols + 0];
    float__t amp  = harmonics.mval[i * harmonics.mcols + 1];

    if (freq > maxFreq) continue;

    // Map frequency to x coordinate
    int xPos = plotLeft + (int)((freq / maxFreq) * plotWidth);

    // Map amplitude to height
    int barHeight = (int)((amp / maxAmp) * plotHeight);
    int yTop      = plotBottom - barHeight;

    // Draw bar
    bmp->drawLine (xPos, plotBottom, xPos, yTop, 3, fgc);

    // Draw frequency label for significant peaks
    if (amp > maxAmp * 0.01L) // Only label peaks > 1% of max
     {
      char label[32];
      if (freq < 1000.0L)
       sprintf (label, "%.0f", (double)freq);
      else
       sprintf (label, "%.1fk", (double)(freq / 1000.0L));

      bmp->drawString (xPos - 10, yTop - 15, label, text_color, 0, 1);
     }
   }

  // Draw title
  char title[128];
  sprintf (title, "Spectrum (0 - %.0f Hz)", (double)maxFreq);
  bmp->drawString (plotLeft + 10, plotTop + 10, title, text_color, 0, 1);

  // Draw amplitude scale
  char scale[32];
  sprintf (scale, "%.2f", (double)maxAmp);
  bmp->drawString (plotLeft - 35, plotTop + 5, scale, text_color, 0, 1);
  bmp->drawString (plotLeft - 20, plotBottom - 5, "0", text_color, 0, 1);
 }

 // Clean up harmonics matrix
 freevar (harmonics);

 // Set result
 res.tag  = tvBMP;
 res.sval = (char *)bmp;
 register_mem (res.sval, ptBMP);
 res.ival = 1;
 res.fval = 1.0L;
 res.imval = 0.0L;

 return true;
}
#pragma endregion

#pragma region Chebyshev
// Evaluate Chebyshev polynomial at point x
// T_n(x) = cos(n * arccos(x)) for |x| <= 1
// For |x| > 1, use recurrence relation
// coeffs is a vector (row or column) with polynomial coefficients [a0, a1, a2, ..., an]
// Result = a0*T0(x) + a1*T1(x) + a2*T2(x) + ... + an*Tn(x)
float__t calculator::EvalChebyshev (value &coeffs, float__t x)
{
 if (coeffs.tag != tvMATRIX)
  {
   errorf (pos, "Expected matrix with Chebyshev coefficients");
   return qnan;
  }

 // Accept both row vector (1 row, N cols) and column vector (N rows, 1 col)
 int n;
 if (coeffs.mrows == 1)
  {
   // Row vector: [(a0, a1, a2, ...)]
   n = coeffs.mcols;
  }
 else if (coeffs.mcols == 1)
  {
   // Column vector: [(a0); (a1); (a2); ...]
   n = coeffs.mrows;
  }
 else
  {
   errorf (pos, "Chebyshev coefficients must be a vector (row or column matrix)");
   return qnan;
  }

 if (isnan (x) || isinf (x)) return qnan;

 if (n == 0) return 0.0L;

 // Use Clenshaw algorithm for stable evaluation
 // b_{n+1} = b_{n+2} = 0
 // b_k = a_k + 2*x*b_{k+1} - b_{k+2}  for k = n-1, n-2, ..., 0
 // Result = a_0 + x*b_1 - b_2

 float__t b_k  = 0.0L; // b_{k+1}
 float__t b_k1 = 0.0L; // b_{k+2}
 float__t x2   = 2.0L * x;

 // Access coefficients sequentially (same for row or column vector)
 for (int k = n - 1; k >= 1; k--)
  {
   float__t b_new = coeffs.mval[k] + x2 * b_k - b_k1;
   b_k1           = b_k;
   b_k            = b_new;
  }

 // Final step
 return coeffs.mval[0] + x * b_k - b_k1;
}

// Complex Chebyshev evaluation
bool calculator::EvalChebyshev (value &coeffs, value &x, value &res)
{
 if (coeffs.tag != tvMATRIX)
  {
   errorf (pos, "Expected matrix with Chebyshev coefficients");
   return false;
  }

 // Accept both row vector (1 row, N cols) and column vector (N rows, 1 col)
 int n;
 if (coeffs.mrows == 1)
  {
   n = coeffs.mcols;
  }
 else if (coeffs.mcols == 1)
  {
   n = coeffs.mrows;
  }
 else
  {
   errorf (pos, "Chebyshev coefficients must be a vector (row or column matrix)");
   return false;
  }

 // Check if x has imaginary component
 if (x.imval != (float__t)0.0L || x.tag == tvCOMPLEX)
  {
   // Complex path using Clenshaw algorithm
   if (n == 0)
    {
     res.tag   = tvCOMPLEX;
     res.fval  = 0.0L;
     res.imval = 0.0L;
     res.ival  = 0;
     return true;
    }

   float__t b_re = 0.0L, b_im = 0.0L;   // b_{k+1}
   float__t b1_re = 0.0L, b1_im = 0.0L; // b_{k+2}
   float__t x2_re = 2.0L * x.fval;
   float__t x2_im = 2.0L * x.imval;

   for (int k = n - 1; k >= 1; k--)
    {
     // b_new = coeffs[k] + 2*x*b_k - b_k1
     // 2*x*b_k (complex multiplication)
     float__t prod_re = x2_re * b_re - x2_im * b_im;
     float__t prod_im = x2_re * b_im + x2_im * b_re;

     float__t b_new_re = coeffs.mval[k] + prod_re - b1_re;
     float__t b_new_im = prod_im - b1_im;

     b1_re = b_re;
     b1_im = b_im;
     b_re  = b_new_re;
     b_im  = b_new_im;
    }

   // Final: a_0 + x*b_1 - b_2
   // x*b_1
   float__t prod_re = x.fval * b_re - x.imval * b_im;
   float__t prod_im = x.fval * b_im + x.imval * b_re;

   res.tag   = tvCOMPLEX;
   res.fval  = coeffs.mval[0] + prod_re - b1_re;
   res.imval = prod_im - b1_im;
   res.ival  = (int_t)res.fval;
   return true;
  }
 else
  {
   // Fast real path
   if (isnan (x.fval) || isinf (x.fval))
    {
     res.tag   = tvFLOAT;
     res.fval  = qnan;
     res.imval = (float__t)0.0L;
     res.ival  = 0;
     return true;
    }

   float__t result = EvalChebyshev (coeffs, x.fval);
   res.tag         = tvFLOAT;
   res.fval        = result;
   res.imval       = (float__t)0.0L;
   res.ival        = (int_t)result;
   return true;
  }
}

// WAV Chebyshev evaluation with normalization
bool calculator::EvalChebyshevWav (value &coeffs, value &wavVal, value &res)
{
 if (coeffs.tag != tvMATRIX)
  {
   errorf (pos, "Expected matrix with Chebyshev coefficients");
   return false;
  }

 // Accept both row vector and column vector
 int n;
 if (coeffs.mrows == 1)
  n = coeffs.mcols;
 else if (coeffs.mcols == 1)
  n = coeffs.mrows;
 else
  {
   errorf (pos, "Chebyshev coefficients must be a vector");
   return false;
  }

 if (wavVal.tag != tvWAV || !wavVal.sval)
  {
   errorf (pos, "Expected WAV object");
   return false;
  }

 WavHeader *header   = (WavHeader *)wavVal.sval;
 uint32_t numSamples = header->dataSize / (header->numChannels * header->bitsPerSample / 8);
 int16_t *inSamples  = (int16_t *)(wavVal.sval + sizeof (WavHeader));

 // First pass: compute and find maximum
 float__t maxAbs = 0.0L;

 for (uint32_t i = 0; i < numSamples; i++)
  {
   // Normalize input to [-1, 1]
   float__t x = (float__t)inSamples[i] / 32768.0L;

   // Evaluate Chebyshev polynomial
   float__t y = EvalChebyshev (coeffs, x);

   float__t absY = Abs (y);
   if (absY > maxAbs) maxAbs = absY;
  }

 // Determine normalization scale
 float__t scale = (maxAbs > 1.0L) ? (1.0L / maxAbs) : 1.0L;

 // Create output WAV
 char *wavBuf
     = CreateWavBuffer (header->sampleRate, header->numChannels, header->bitsPerSample, numSamples);
 if (!wavBuf)
  {
   errorf (pos, "Failed to create WAV buffer");
   return false;
  }

 int16_t *outSamples = (int16_t *)(wavBuf + sizeof (WavHeader));

 // Second pass: compute and write normalized samples
 for (uint32_t i = 0; i < numSamples; i++)
  {
   float__t x = (float__t)inSamples[i] / 32768.0L;
   float__t y = EvalChebyshev (coeffs, x) * scale;

   // Clamp to [-1, 1]
   if (y > 1.0L) y = 1.0L;
   if (y < -1.0L) y = -1.0L;

   outSamples[i] = (int16_t)(y * 32767.0L);
  }

 res.tag  = tvWAV;
 res.sval = wavBuf;
 res.fval  = (float__t)0.0L;
 res.imval = (float__t)0.0L;
 res.ival  = 0;
 register_mem (wavBuf, ptMALLOC);

 return true;
}

// Helper to create WAV buffer
char *calculator::CreateWavBuffer (uint32_t sampleRate, uint16_t numChannels,
                                   uint16_t bitsPerSample, uint32_t numSamples)
{
 uint32_t dataSize = numSamples * numChannels * (bitsPerSample / 8);
 uint32_t fileSize = sizeof (WavHeader) + dataSize;

 char *buffer = (char *)malloc (fileSize);
 if (!buffer) return nullptr;

 WavHeader *header = (WavHeader *)buffer;
 memcpy (header->riff, "RIFF", 4);
 header->fileSize = fileSize - 8;
 memcpy (header->wave, "WAVE", 4);
 memcpy (header->fmt, "fmt ", 4);
 header->fmtSize       = 16;
 header->audioFormat   = 1; // PCM
 header->numChannels   = numChannels;
 header->sampleRate    = sampleRate;
 header->bitsPerSample = bitsPerSample;
 header->byteRate      = sampleRate * numChannels * (bitsPerSample / 8);
 header->blockAlign    = numChannels * (bitsPerSample / 8);
 memcpy (header->data, "data", 4);
 header->dataSize = dataSize;

 return buffer;
}
#pragma endregion

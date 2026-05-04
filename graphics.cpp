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
 return true;
}

bool calculator::isChildResReal (calculator *child)
{
 float__t re = child->get_re_res ();
 float__t im = child->get_im_res ();
 if (isnan (re) || Abs (im) / AbsC (re, im) > (float__t)1e-12L)
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
  case pl_plot:     // expr, from, to, var
  case pl_plotpol:
  case pl_plotlgx:
  case pl_plotlgy:
  case pl_plotlgxy:
  case pl_plotsmith:
   if (!ShowImageFn) return false; // ShowImageFn must be set for non-file plotting (NULL for CLI)
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_xyplot:  // x_expr, y_expr, from, to, var
   if (!ShowImageFn) return false; // ShowImageFn must be set for non-file plotting (NULL for CLI)
   split_ok = Split (expr, sexpr, STRBUF, sexpr_y, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF,
                     nullptr, 0);
  break;

  case pl_fplot:    // fname, expr, from, to, var
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

  case pl_fxyplot: // fname, x_expr, y_expr, from, to, var
  case pl_oxyplot:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sexpr_y, STRBUF, sfrom, MAXOP, sto, MAXOP,
                     svar, STRBUF, nullptr, 0);
  break;

  case pl_plotsmithz: // expr, from, to, var, z0
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, sz0, MAXOP,
                     nullptr, 0);
   break;

  case pl_fplotsmithz: // file, expr, from, to, var, z0
  case pl_oplotsmithz:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF,
                     sz0, MAXOP, nullptr, 0);
   break;

  case pl_plotdata: // datafile, mask
  case pl_plotdatal:
   if (!ShowImageFn) return false;
   split_ok = Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0);    // try get mask
   if (!split_ok) split_ok = Split (expr, sexpr, STRBUF, nullptr, 0);    // get datafile
   //if (split_ok) Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0); //try get mask
  break;

  case pl_fplotdata: // bmpfile, datafile, mask
  case pl_oplotdata:
  case pl_fplotdatal:
  case pl_oplotdatal:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, nullptr, 0); //get bmp and datafile
   if (split_ok) Split (expr, fname, STRBUF, sexpr, STRBUF, svar, STRBUF, nullptr, 0); //try get mask
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

 calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return false;
  }

// Process filename for file-based functions
 switch (fidx)
 {
  case pl_fplot:
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
  case pl_fxyplot:     
  case pl_oxyplot: 
  case pl_fplotlgx:
  case pl_oplotlgx:
  case pl_fplotlgy:
  case pl_oplotlgy:
  case pl_fplotlgxy:
  case pl_oplotlgxy:
  case pl_fplotsmith: 
  case pl_oplotsmith: 
  case pl_fplotsmithz:
  case pl_oplotsmithz: 
  case pl_fplotdata: // bmpfile, datafile, mask
  case pl_oplotdata:
  case pl_fplotdatal:
  case pl_oplotdatal:
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
  break;
  }

 if (fidx < pl_plotdata)
  {
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
   if (fidx == pl_plotsmithz || fidx == pl_fplotsmithz || fidx == pl_oplotsmithz)
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
   else fvx = 0;

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
     else fvx = 0;
    }

   // Fill in the parameters
   params.sexpr   = strdup (sexpr);
   params.sexpr_y = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot)
                        ? strdup (sexpr_y)
                        : nullptr;
   params.svar    = strdup (svar);
   params.vfrom   = vfrom;
   params.vto     = vto;
   params.ymin    = fvx;
   params.ymax    = fvx;

   params.width = getivar ("plot_width");
   if ((params.width <= 100) || (params.width > 2000)) params.width = 800;

   params.height = getivar ("plot_height");
   if ((params.height <= 100) || (params.height > 2000)) params.height = 600;

   params.top = getivar ("plot_top");
   if (params.top < 0 || params.top > 2000) params.top = 0;

   params.left = getivar ("plot_left");
   if (params.left < 0 || params.left > 2000) params.left = 0;

   params.pxsize = getivar ("plot_dotsz");
   if (params.pxsize < 1 || params.pxsize > 4) params.pxsize = 4;

   params.dot = false;

   params.bgc     = getivar ("plot_bgc");
   params.fgc     = getivar ("plot_fgc");
   params.padding = 40;

   params.grid_color = 0xC0C0C0;
   params.axis_color = 0x808080;
   params.text_color = ~params.bgc;

   // Set logarithmic flags
   params.log_x = (fidx == pl_plotlgx || fidx == pl_fplotlgx || fidx == pl_oplotlgx
                   || fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);
   params.log_y = (fidx == pl_plotlgy || fidx == pl_fplotlgy || fidx == pl_oplotlgy
                   || fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);

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

   params.width = getivar ("plot_width");
   if ((params.width <= 100) || (params.width > 2000)) params.width = 800;

   params.height = getivar ("plot_height");
   if ((params.height <= 100) || (params.height > 2000)) params.height = 600;

   params.pxsize = getivar ("plot_dotsz");
   if (params.pxsize < 1 || params.pxsize > 4) params.pxsize = 4;

   params.dot = (fidx == pl_plotdata || fidx == pl_fplotdata || fidx == pl_oplotdata);

   params.bgc     = getivar ("plot_bgc");
   params.fgc     = getivar ("plot_fgc");
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

 // Calculate scale: how many pixels per unit radius
 int plot_size  = (width < height ? width : height) - 2 * padding;
 float__t scale = plot_size / (2.0 * rmax);

 // Second pass: draw the curve
 angle                 = angle_from;
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

 // Second pass: draw the curve
 x                     = vfrom;
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
 char fnamebuf[STRBUF];
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
 bool is_overlay     = ((fidx == pl_oplot || fidx == pl_oplotpol || fidx == pl_oxyplot
                     || fidx == pl_oplotlgx || fidx == pl_oplotlgy || fidx == pl_oplotlgxy
                     || fidx == pl_oplotsmith || fidx == pl_oplotsmithz 
                     || fidx == pl_oplotdata || fidx == pl_oplotdatal)
                    && fname[0]);

 bool is_polar       = (fidx == pl_plotpol || fidx == pl_fplotpol || fidx == pl_oplotpol);

 bool is_parametric = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot);

 bool is_logarithmic = (fidx == pl_plotlgx || fidx == pl_fplotlgx || fidx == pl_oplotlgx ||
                        fidx == pl_plotlgy || fidx == pl_fplotlgy || fidx == pl_oplotlgy ||
                        fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);

 bool is_smith       = (fidx == pl_plotsmith || fidx == pl_fplotsmith || fidx == pl_oplotsmith
                  || fidx == pl_plotsmithz || fidx == pl_fplotsmithz || fidx == pl_oplotsmithz);

 bool is_data = (fidx >= pl_plotdata && fidx <= pl_oplotdatal);

 if (is_overlay)
  {
   // Try to load existing image for overlay
   if (bmp->load (fname))
    {
     // Successfully loaded — get dimensions and dominant background color
     params.width      = bmp->getWidth ();
     params.height     = bmp->getHeight ();
     params.bgc        = bmp->getDominantColor ();
     params.text_color = ~params.bgc;
    }
   else
    {
     // Failed to load — create new bitmap (fallback to fplot behavior)
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
 if (!is_overlay)
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
 switch (fidx)
 {
  case pl_fplot:
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
  case pl_fxyplot:
  case pl_oxyplot:
  case pl_fplotlgx:
  case pl_oplotlgx:
  case pl_fplotlgy:
  case pl_oplotlgy:
  case pl_fplotlgxy:
  case pl_oplotlgxy:
  case pl_fplotsmith:
  case pl_oplotsmith:
  case pl_fplotsmithz:
  case pl_oplotsmithz:
  case pl_fplotdata:
  case pl_oplotdata:
   // Already handled above with is_overlay logic
   bmp->save (fname);
  break;
  case pl_plot:
  case pl_plotpol:
  case pl_xyplot:
  case pl_plotlgx:
  case pl_plotlgy:
  case pl_plotlgxy:
  case pl_plotsmith:
  case pl_plotsmithz:
  case pl_plotdata:
  case pl_plotdatal:
   // Show in GUI
   if (ShowImageFn)
    {
     ShowImageFn ((void *)bmp);
    }
  break;
  }

 // 6. Cleanup
 delete bmp;
 fflags |= params.child->isfflags ();
 delete params.child;
 if (params.sexpr) free (params.sexpr);
 if (params.sexpr_y) free (params.sexpr_y);
 if (params.svar) free (params.svar);

 return true;
}
#pragma endregion

// Исправить PlotParametric для единого масштаба:
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
 t = t_from;
 bool has_valid_points = false;

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
 params.xmin = xmin;
 params.xmax = xmax;
 params.ymin = ymin;
 params.ymax = ymax;
 params.scale = scale;

 return true;
}

// Модифицировать PlotDrawAxesParametric для лучшего отображения заголовка:
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

 float__t x_range = xmax - xmin;
 float__t y_range = ymax - ymin;

 int plot_width       = width - 2 * padding;
 int plot_height      = height - 2 * padding;
 int grid_step_pixels = (plot_width > plot_height ? plot_width : plot_height) / 10;

 int center_x = width / 2;
 int center_y = height / 2;
 
 float__t x_center = (xmin + xmax) / 2.0;
 float__t y_center = (ymin + ymax) / 2.0;
 float__t scale = params.scale;

 // Calculate axis positions
 int x_axis_pixel = center_y - (int)((0.0 - y_center) * scale);
 int y_axis_pixel = center_x + (int)((0.0 - x_center) * scale);

 // Horizontal grid lines
 for (int offset = 0; offset <= plot_height / 2; offset += grid_step_pixels)
  {
   int y_up   = center_y - offset;
   int y_down = center_y + offset;

   if (y_up >= padding && y_up < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);
    }

   if (offset > 0 && y_down >= padding && y_down < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);
    }
  }

 // Vertical grid lines
 for (int offset = 0; offset <= plot_width / 2; offset += grid_step_pixels)
  {
   int x_left  = center_x - offset;
   int x_right = center_x + offset;

   if (x_left >= padding && x_left < width - padding)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
    }

   if (offset > 0 && x_right >= padding && x_right < width - padding)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);
    }
  }

 // Axes
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
 if (strlen(params.sexpr) > max_len)
  snprintf (title_x, sizeof (title_x), "x=%.27s...", params.sexpr);
 else
  snprintf (title_x, sizeof (title_x), "x=%s", params.sexpr);
  
 if (strlen(params.sexpr_y) > max_len)
  snprintf (title_y, sizeof (title_y), "y=%.27s...", params.sexpr_y);
 else
  snprintf (title_y, sizeof (title_y), "y=%s", params.sexpr_y);

 title_x[sizeof (title_x) - 1] = '\0';
 title_y[sizeof (title_y) - 1] = '\0';

 // Draw on two lines
 bmp->drawString (5, 5, title_x, text_color, 0, 1);
 bmp->drawString (5, 17, title_y, text_color, 0, 1);
}
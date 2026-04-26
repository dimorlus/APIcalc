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

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find bounding box in Cartesian coordinates
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
     float__t x = r * Cos (angle);
     float__t y = r * Sin (angle);

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
   angle += step;
  }
 while (angle <= angle_to);

 // Add padding to ranges
 float__t x_range = xmax - xmin;
 float__t y_range = ymax - ymin;
 if (x_range < 1e-10) x_range = 1.0;
 if (y_range < 1e-10) y_range = 1.0;

 float__t x_pad = x_range * 0.1;
 float__t y_pad = y_range * 0.1;
 xmin -= x_pad;
 xmax += x_pad;
 ymin -= y_pad;
 ymax += y_pad;

 x_range = xmax - xmin;
 y_range = ymax - ymin;

 // Calculate scale to fit in available space
 int plot_width   = width - 2 * padding;
 int plot_height  = height - 2 * padding;
 float__t scale_x = plot_width / x_range;
 float__t scale_y = plot_height / y_range;
 float__t scale   = (scale_x < scale_y) ? scale_x : scale_y;

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
     float__t x_cart = r * Cos (angle);
     float__t y_cart = r * Sin (angle);

     // Convert to screen coordinates (centered)
     int x_screen = center_x + (int)((x_cart - (xmin + xmax) / 2) * scale);
     int y_screen = center_y - (int)((y_cart - (ymin + ymax) / 2) * scale);

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
 params.xmin  = xmin;
 params.xmax  = xmax;
 params.ymin  = ymin;
 params.ymax  = ymax;
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

 float__t xmin  = params.xmin;
 float__t xmax  = params.xmax;
 float__t ymin  = params.ymin;
 float__t ymax  = params.ymax;
 float__t scale = params.scale;

 // Calculate maximum radius in data coordinates
 float__t x_range    = xmax - xmin;
 float__t y_range    = ymax - ymin;
 float__t max_radius = (x_range > y_range ? x_range : y_range) / 2.0;

 // Draw radial grid (10 circles)
 for (int i = 1; i <= 10; i++)
  {
   float__t r_data = max_radius * i / 10.0;
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

   // Draw radius label
   if (i == 10)
    {
     char label[32];
     d2scistr (label, (double)r_data);
     bmp->drawString (center_x + r_pixels + 5, center_y - 10, label, text_color, 0, 1);
    }
  }

 // Draw angular grid (every 30 degrees)
 for (int angle_deg = 0; angle_deg < 360; angle_deg += 30)
  {
   float__t angle_rad = angle_deg * M_PI / 180.0;
   int max_r_pixels   = (int)(max_radius * scale);

   for (int r = 0; r <= max_r_pixels; r += 4)
    {
     int x = center_x + (int)(r * Cos (angle_rad));
     int y = center_y - (int)(r * Sin (angle_rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      {
       bmp->drawPixel (x, y, grid_color);
      }
    }

   // Draw angle labels at outermost circle
   int label_r = (int)(max_radius * scale * 1.1);
   int label_x = center_x + (int)(label_r * Cos (angle_rad));
   int label_y = center_y - (int)(label_r * Sin (angle_rad));

   char angle_label[16];
   sprintf (angle_label, "%d°", angle_deg);
   bmp->drawString (label_x - 10, label_y - 5, angle_label, text_color, 0, 1);
  }

 // Draw main axes (0° and 90°)
 bmp->drawLine (padding, center_y, width - padding, center_y, 1, axis_color);
 bmp->drawLine (center_x, padding, center_x, height - padding, 1, axis_color);

 // Draw title
 char title[128];
 snprintf (title, sizeof (title), "r=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (width / 2 - 50, 5, title, text_color, 0, 2);

 // Draw variable name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}

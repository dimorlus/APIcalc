// Plot Polar graph (specific part)
bool calculator::PlotPolar (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t angle_from = params.vfrom;
 float__t angle_to   = params.vto;

 int width   = params.width;
 int height  = params.height;
 int padding = params.padding;
 uint32_t fgc = params.fgc;

 // Center of polar plot is always in the middle
 int center_x = width / 2;
 int center_y = height / 2;

 // Calculate appropriate step for smooth circle
 int available_size = (width < height ? width : height) - 2 * padding;
 float__t circumference = 2.0 * M_PI * (available_size / 2);
 float__t step = (angle_to - angle_from) / (circumference / 4.0); // ~4 pixels per step

 float__t rmax = 0;
 bool first_point = true;
 uint64_t init_ms = GetTickCount64 ();
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
       rmax = abs_r;
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
 int plot_size = (width < height ? width : height) - 2 * padding;
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
 params.ymin = -rmax;
 params.ymax = rmax;
 params.xmin = -rmax;
 params.xmax = rmax;
 params.scale = scale;

 return true;
}

// Draw polar grid and axes
void calculator::PlotDrawAxesPolar (bmpdraw *bmp, PlotParams &params)
{
 int width = params.width;
 int height = params.height;
 int padding = params.padding;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int center_x = width / 2;
 int center_y = height / 2;

 float__t rmax = params.ymax; // Maximum radius in data units
 float__t scale = params.scale;

 // Draw radial grid (10 circles)
 for (int i = 1; i <= 10; i++)
  {
   float__t r_data = rmax * i / 10.0;
   int r_pixels = (int)(r_data * scale);

   // Draw circle
   for (int angle_deg = 0; angle_deg < 360; angle_deg += 2)
    {
     float__t angle_rad = angle_deg * M_PI / 180.0;
     int x = center_x + (int)(r_pixels * Cos (angle_rad));
     int y = center_y - (int)(r_pixels * Sin (angle_rad));

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
 bmp->drawLine (center_x - max_r_pixels, center_y, center_x + max_r_pixels, center_y, 1, axis_color);
 bmp->drawLine (center_x, center_y - max_r_pixels, center_x, center_y + max_r_pixels, 1, axis_color);

 // Draw title
 char title[128];
 snprintf (title, sizeof (title), "r=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (width / 2 - 50, 5, title, text_color, 0, 2);

 // Draw variable name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}

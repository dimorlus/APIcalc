// Добавить PlotDrawAxesSmith - рисование сетки Смита:
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
     int x = screen_center_x + (int)(screen_radius * Cos (rad));
     int y = center_y - (int)(screen_radius * Sin (rad));
     
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
   float__t arc_center_x = 1.0;
   float__t arc_center_y_pos = 1.0 / x;
   float__t arc_center_y_neg = -1.0 / x;
   float__t arc_radius = 1.0 / x;

   int screen_center_x = center_x + (int)(arc_center_x * radius);
   
   // Positive reactance (inductive, upper half)
   int screen_center_y_pos = center_y - (int)(arc_center_y_pos * radius);
   int screen_arc_radius = (int)(arc_radius * radius);

   // Draw arc from left intersection to right edge
   for (int angle = 90; angle <= 270; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x_pos = screen_center_x + (int)(screen_arc_radius * Cos (rad));
     int y_pos = screen_center_y_pos - (int)(screen_arc_radius * Sin (rad));
     
     // Check if inside outer circle
     int dx = x_pos - center_x;
     int dy = y_pos - center_y;
     if (dx * dx + dy * dy <= radius * radius)
      {
       if (x_pos >= padding && x_pos < width - padding && 
           y_pos >= padding && y_pos < height - padding)
        bmp->drawPixel (x_pos, y_pos, grid_color);
      }
    }

   // Negative reactance (capacitive, lower half)
   int screen_center_y_neg = center_y - (int)(arc_center_y_neg * radius);

   for (int angle = 90; angle <= 270; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x_neg = screen_center_x + (int)(screen_arc_radius * Cos (rad));
     int y_neg = screen_center_y_neg - (int)(screen_arc_radius * Sin (rad));
     
     // Check if inside outer circle
     int dx = x_neg - center_x;
     int dy = y_neg - center_y;
     if (dx * dx + dy * dy <= radius * radius)
      {
       if (x_neg >= padding && x_neg < width - padding && 
           y_neg >= padding && y_neg < height - padding)
        bmp->drawPixel (x_neg, y_neg, grid_color);
      }
    }
  }

 // Add labels
 char label[64];

 // Z0 label
 sprintf (label, "Z0=%.0fΩ", (double)z0);
 bmp->drawString (center_x + radius + 5, center_y - 10, label, text_color, 0, 1);

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
 bmp->drawString (width / 2 - 50, 5, title, text_color, 0, 2);

 // Parameter name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}
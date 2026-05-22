// Add this function after the PlotDrawAxesCartesian function (around line 900)

// Overlay two bitmaps: bmp1 = bmp1 + bmp2
// Returns true on success, false if bitmaps are incompatible
bool calculator::AddBmp (bmpdraw *bmp1, bmpdraw *bmp2)
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
   errorf (pos, "AddBmp: bitmap dimensions mismatch (%dx%d vs %dx%d)", 
           bmp1->width, bmp1->height, bmp2->width, bmp2->height);
   return false;
  }

 // Check row sizes
 if (bmp1->rowSize != bmp2->rowSize)
  {
   errorf (pos, "AddBmp: bitmap row sizes mismatch");
   return false;
  }

 // Overlay pixels: for each non-background pixel in bmp2, copy it to bmp1
 // Background is assumed to be white (0xFFFFFF) or the dominant color
 uint32_t bg_color = bmp1->getDominantColor ();

 for (int y = 0; y < bmp1->height; y++)
  {
   for (int x = 0; x < bmp1->width; x++)
    {
     int offset = y * bmp1->rowSize + x * 3;

     // Read pixel from bmp2
     uint8_t b2 = bmp2->data[offset];
     uint8_t g2 = bmp2->data[offset + 1];
     uint8_t r2 = bmp2->data[offset + 2];

     uint32_t color2 = (r2 << 16) | (g2 << 8) | b2;

     // If pixel is not background, overlay it on bmp1
     if (color2 != bg_color)
      {
       bmp1->data[offset]     = b2;
       bmp1->data[offset + 1] = g2;
       bmp1->data[offset + 2] = r2;
      }
    }
  }

 return true;
}
#ifndef bmpH
#define bmpH

#ifdef __BORLANDC__
#include <stdint.h>

#pragma warn -8027
#define nullptr NULL
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <cstdint>
#endif

#ifdef __BORLANDC__
#pragma pack(push, 1)
struct BMPHeader
{
 uint16_t bfType; // 'BM'
 uint32_t bfSize;
 uint16_t bfReserved1, bfReserved2;
 uint32_t bfOffBits;
 // Info Header
 uint32_t biSize;
 int32_t biWidth, biHeight;
 uint16_t biPlanes, biBitCount;
 uint32_t biCompression, biSizeImage;
 int32_t biXPelsPerMeter, biYPelsPerMeter;
 uint32_t biClrUsed, biClrImportant;
};
#pragma pack(pop)
#else
#pragma pack(push, 1)
struct BMPHeader
{
 uint16_t bfType { 0x4D42 }; // 'BM'
 uint32_t bfSize;
 uint16_t bfReserved1 { 0 }, bfReserved2{ 0 };
 uint32_t bfOffBits { 54 };
 // Info Header
 uint32_t biSize{ 40 };
 int32_t biWidth, biHeight;
 uint16_t biPlanes{ 1 }, biBitCount{ 24 };
 uint32_t biCompression{ 0 }, biSizeImage{ 0 };
 int32_t biXPelsPerMeter{ 0 }, biYPelsPerMeter{ 0 };
 uint32_t biClrUsed{ 0 }, biClrImportant{ 0 };
};
#pragma pack(pop)
#endif

extern const uint8_t font5x8[104][5];

union tColor
{
 uint32_t Color;
 struct
  {
   uint8_t B;
   uint8_t G;
   uint8_t R;
   uint8_t A;
  };
};

class bmpdraw
{
 private:
 int width;
 int height;
 int rowSize; // Size of the row with alignment
 uint8_t *data;
 int curX, curY; // current position for MoveTo/LineTo

 void setPixel (int x, int y, uint32_t color);
 void drawThickPoint (int x, int y, int thickness, uint32_t color);

 public:
 bmpdraw (void);

 bool load (const char *fname);
 bool save (const char *fname);
 bool newbmp (int w, int h, uint32_t color = 0xFFFFFF);

 void clear (uint32_t color = 0xFFFFFF);
 
 // Drawing a pixel
 void drawPixel (int x, int y, uint32_t color);
 
 // Drawing lines
 void drawLine (int x0, int y0, int x1, int y1, uint32_t color);
 void drawLine (int x0, int y0, int x1, int y1, int thickness, uint32_t color);
 
 // MoveTo/LineTo
 void moveTo (int x, int y);
 void lineTo (int x, int y, uint32_t color);
 void lineTo (int x, int y, int thickness, uint32_t color);
 
 // Drawing text
 void drawChar (int x, int y, char c, uint32_t color, int angle = 0, int scale = 1);
 void drawString (int x, int y, const char *s, uint32_t color, int angle = 0, int scale = 1);
 
 // angle: 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
 
 // Getting image information
 int getWidth () const { return width; }
 int getHeight () const { return height; }
 uint32_t getDominantColor () const; // get the dominant color

 ~bmpdraw (void);
};

void test_bmp ();

#endif

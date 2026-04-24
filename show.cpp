#include "bmp.h" // Теперь можем работать с bmpdraw напрямую

bool WinApiCalc::ShowImageWindowFromBMP (void *bmpObject)
{
 if (!bmpObject) return false;

 bmpdraw *bmp = (bmpdraw *)bmpObject;
 if (!bmp->data || bmp->width <= 0 || bmp->height <= 0) return false;

 int width  = bmp->width;
 int height = bmp->height;

 const char *pClassName = "WinApiCalcImageWindow";

 // ... регистрация класса ...

 // Создаём DIB
 BITMAPINFO bmi              = { 0 };
 bmi.bmiHeader.biSize        = sizeof (BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth       = width;
 bmi.bmiHeader.biHeight      = -height;
 bmi.bmiHeader.biPlanes      = 1;
 bmi.bmiHeader.biBitCount    = 32;
 bmi.bmiHeader.biCompression = BI_RGB;

 HDC hdcScreen   = GetDC (nullptr);
 void *pBits     = nullptr;
 HBITMAP hBitmap = CreateDIBSection (hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
 ReleaseDC (nullptr, hdcScreen);

 if (!hBitmap || !pBits) return false;

 // --- ПРЯМОЕ ПРЕОБРАЗОВАНИЕ ИЗ bmp->data (BGR, 3 байта) В BGRA (4 байта) ---
 uint32_t *dest = (uint32_t *)pBits;

 for (int y = 0; y < height; y++)
  {
   for (int x = 0; x < width; x++)
    {
     int srcIdx = y * bmp->rowSize + x * 3; // BGR: учитываем выравнивание rowSize
     int dstIdx = y * width + x;

     uint8_t b = bmp->data[srcIdx + 0];
     uint8_t g = bmp->data[srcIdx + 1];
     uint8_t r = bmp->data[srcIdx + 2];

     // Windows DIB: BGRA (little-endian)
     dest[dstIdx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
  }

 // --- Создание окна (как раньше) ---
 RECT rc;
 if (GetWindowRect (m_hWnd, &rc))
  {
   rc.left += 10;
   rc.top += 10;
  }
 else
  {
   rc.left = 100;
   rc.top  = 100;
  }

 RECT clientRect = { 0, 0, width, height };
 DWORD dwStyle   = WS_POPUP | WS_VISIBLE | WS_BORDER | WS_CAPTION | WS_SYSMENU;
 DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;

 AdjustWindowRectEx (&clientRect, dwStyle, FALSE, dwExStyle);

 int windowWidth  = clientRect.right - clientRect.left;
 int windowHeight = clientRect.bottom - clientRect.top;

 HWND hImageWnd = CreateWindowExA (dwExStyle, pClassName, "Image Preview", dwStyle,
                                   rc.left, rc.top, windowWidth, windowHeight,
                                   m_hWnd, nullptr, GetModuleHandle (nullptr), nullptr);

 if (!hImageWnd)
  {
   DeleteObject (hBitmap);
   return false;
  }

 SetWindowLongPtrA (hImageWnd, GWLP_USERDATA, (LONG_PTR)hBitmap);
 SetFocus (hImageWnd);

 return true;
}
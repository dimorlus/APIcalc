//---------------------------------------------------------------------------
#include "pch.h"

#ifdef __BORLANDC__
#if defined(_WIN64)||defined(_WIN32)
#define _WIN_
#include <windows.h>
#endif
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#pragma hdrstop
#else
#if defined(_WIN64) || defined(_WIN32)
#define _WIN_
#include <windows.h>
#endif
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

#include "scalc.h"
#include "sfmts.h"

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma package(smart_init)
#pragma warn -8008 // Condition is always true
#pragma warn -8066 // Unreachable code
#pragma warn -8004 // assigned a value that is never used

#else
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)

#define M_PI    3.1415926535897932384626433832795
#define M_PI_2l 1.5707963267948966192313216916398
#define M_E     2.7182818284590452353602874713527
#endif
//----------------------------------
int ones (unsigned char *cp, int from, int to)
{
 int res = 0;
 for (int i = from; i < to; i++)
  if (cp[i / 8] & (1 << (i % 8))) res++;
 return res;
}
//---------------------------------------------------------------------------

bool isNan (float__t d)
{
 int fr, ex;
 if (d < 0) d = -d;
 if (sizeof (d) == 80 / 8) // IEEE 754 80 bits
  {
   fr = ones ((unsigned char *)&d, 0, 64);
   ex = ones ((unsigned char *)&d, 64, 80);
   return ((ex >= 15) && fr);
  }
 else if (sizeof (d) == 64 / 8) // IEEE 754 64 bits
  {
   fr = ones ((unsigned char *)&d, 0, 52);
   ex = ones ((unsigned char *)&d, 52, 64);
   return ((ex >= 11) && fr);
  }
 else if (sizeof (d) == 32 / 8) // IEEE 754 32 bits
  {
   fr = ones ((unsigned char *)&d, 0, 23);
   ex = ones ((unsigned char *)&d, 23, 32);
   return ((ex >= 8) && fr);
  }
 return false;
}
//---------------------------------------------------------------------------

const uint64_t dms[] = { (60ull * 60 * 60 * 24 * 36525ull),
                         (6ull * 6 * 24 * 36525),
                         (60ull * 60 * 24),
                         (60ull * 60),
                         60ull,
                         1ull };

int t2str (char *str, uint64_t sec)
{
 const char *fmt[] = { "%lld:c ", "%lld:y ", "%lld:d ", "%lld:h ", "%lld:m ", "%lld:s " };
 uint64_t pt[6];
 int i, j, k;
 char *pc = str;

 for (i = 0, j = -1, k = 0; i < 6; i++)
  {
   pt[i] = (uint64_t)(sec / dms[i]);
   sec %= dms[i];
   if ((j == -1) && (pt[i] != 0)) j = i;
   if ((j != -1) && (pt[i] != 0)) k = i;
  }
 *str = '\0';
 if (j == -1)
  str += sprintf (str, "0s");
 else
  for (i = j; i <= k; i++) str += sprintf (str, fmt[i], pt[i]);
 return str - pc;
}
//---------------------------------------------------------------------------

/* convert int to bin according format %10b */
int b2str (char *str, const char *fmt, uint64_t b)
{
 char c;
 bool flag      = false;
 unsigned int j = 0;
 char *ws       = nullptr;
 unsigned int wide;
 unsigned int i, w = 0;
 uint64_t mask;

 while (0 != (c = *fmt++))
  {
   if (!flag && (c == '%'))
    {
     flag = true;
     ws   = (char *)fmt;
     continue;
    }
   if (flag && (c == '%'))
    {
     flag = false;
     continue;
    }
   if (flag && (c == 'b'))
    {
     for (wide = 0; ws < fmt; ws++)
      {
       c = *ws;
       if (c == '-') continue;
       if ((c >= '0') && (c <= '9'))
        wide = wide * 10 + (c - '0');
       else
        break;
      }

     mask = 1ull << 63;
     for (w = wide; ((w > 0) && (mask > 1) && ((b & mask) == 0)); w--) mask >>= 1;
     if (wide <= w) wide = w;

     mask = 1ull << (wide - 1);
     for (i = 0; i < wide; i++)
      {
       if (b & mask)
        str[j++] = '1';
       else
        str[j++] = '0';
       mask >>= 1;
      }
     flag = false;
     continue;
    }
   if (!flag) str[j++] = c;
  }
 str[j] = '\0';
 return j;
}
//---------------------------------------------------------------------------

int d2scistr (char *str, float__t d)
{
 const char csci[]
     = { 'q', 'r', 'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm', ' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'R', 'Q' };
 enum
 {
  quecto,
  ronto,
  yocto,
  zepto,
  atto,
  femto,
  pico,
  nano,
  micro,
  milli,
  empty,
  kilo,
  mega,
  giga,
  tera,
  peta,
  exa,
  zetta,
  yotta,
  ronna,
  quetta
 };

 if (isNan (d))
  return sprintf (str, "%e", d);
 else
  {
#ifdef _long_double_
   float__t dd = fabsl (d);
#else  /*_long_double_*/
   float__t dd = fabs (d);
#endif /*_long_double_*/
   int rng = empty;
   if (dd > 0)
    {
     while (dd >= 1000)
      {
       rng++;
       dd /= 1000;
       if (rng > quetta) break;
      }
     while (dd < 1)
      {
       rng--;
       dd *= 1000;
       if (rng < quecto) break;
      }
    }
   if (d < 0) dd = -dd;
#ifdef _long_double_
   if (rng == empty)
    return sprintf (str, "%.4Lg", d);
   else if ((rng >= quecto) && (rng <= quetta))
    return sprintf (str, "%.4Lg%c", dd, csci[rng]);
   else
    return sprintf (str, "%.2Le", d);
#else  /*_long_double_*/
   if (rng == empty)
    return sprintf (str, "%.4g", d);
   else if ((rng >= yocto) && (rng <= yotta))
    return sprintf (str, "%.4g%c", dd, csci[rng]);
   else
    return sprintf (str, "%.2e", d);
#endif /*_long_double_*/
  }
}
//---------------------------------------------------------------------------

int normz (float__t &re, float__t &im)
 {
  float__t d = sqrt (re * re + im * im);
  if (isNan (d)) return 0;
  if (d == 0) return 0;
  if (re != 0 && (d / fabs(re) > 1000.0)) re = 0.0;
  if (im != 0 && (d / fabs(im) > 1000.0)) im = 0.0;
  return 1;
 }


//---------------------------------------------------------------------------
int d2nrmstr (char *str, float__t d)
{
 if (isNan (d))
  return sprintf (str, "%e", d);
 else
  {
#ifdef _long_double_
   float__t dd = fabsl (d);
#else  /*_long_double_*/
   float__t dd = fabs (d);
#endif /*_long_double_*/
   int rng = 0;
   if (dd > 0)
    {
     while (dd >= 1000)
      {
       rng++;
       dd /= 1000;
       if (rng > 308) break;
      }
     while (dd < 1)
      {
       rng--;
       dd *= 1000;
       if (rng < -308) break;
      }
    }
   if (d < 0) dd = -dd;
#ifdef _long_double_
   if (rng == 0)
    return sprintf (str, "%.4Lg", d);
   else
    return sprintf (str, "%.4Lge%i", dd, rng * 3);
#else  /*_long_double_*/
   if (rng == 0)
    return sprintf (str, "%.4g", d);
   else
    return sprintf (str, "%.4ge%i", dd, rng * 3);
#endif /*_long_double_*/
  }
}
//---------------------------------------------------------------------------

int b2scistr (char *str, float__t d)
{
 const char csci_plus[] = { ' ', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'R', 'Q'};
 enum
 {
  Empty,
  Kilo,
  Mega,
  Giga,
  Tera,
  Peta,
  Exa,
  Zetta,
  Yotta,
  Ronna,
  Quetta
 };

 if (isNan (d))
  return sprintf (str, "%e", d);
 else
  {
#ifdef _long_double_
   float__t dd = fabsl (d);
#else  /*_long_double_*/
   float__t dd = fabs (d);
#endif /*_long_double_*/
   int rng = Empty;
   if (dd > 0)
    {
     while ((rng <= Quetta) && (dd >= 1024))
      {
       rng++;
       dd /= 1024;
      }
    }
   if (d < 0) dd = -dd;

#ifdef _long_double_
   if ((rng > Empty) && (rng <= Quetta))
    return sprintf (str, "%.4Lg%ciB", dd, csci_plus[rng]);
   else
    return sprintf (str, "%.4Lg", d);
#else  /*_long_double_*/
   if ((rng > Empty) && (rng <= Quetta))
    return sprintf (str, "%.4g%ciB", dd, csci_plus[rng]);
   else
    return sprintf (str, "%.4g", d);
#endif /*_long_double_*/
  }
}
//---------------------------------------------------------------------------
int dgr2str (char *str, float__t radians)
{
 const char cdeg[] = { 96, 39, 34 }; // ` ' "
 double degrees    = radians * 180.0 / M_PI;
 int deg           = (int)degrees;
 double min_full   = (degrees - deg) * 60.0;
 int min           = (int)min_full;
 double sec_full   = (min_full - min) * 60.0;
 int sec           = (int)(sec_full + 0.5); // Rounding seconds

 if (fabs (degrees) > 36000)
  return sprintf (str, "--%c--%c--%c", cdeg[0], cdeg[1], cdeg[2]);
 // Seconds and minutes overflow correction
 if (sec == 60)
  {
   sec = 0;
   min += 1;
  }
 if (min == 60)
  {
   min = 0;
   deg += 1;
  }

 // Format the string
 if (deg != 0)
  return sprintf (str, "%d%c%d%c%d%c", deg, cdeg[0], min, cdeg[1], sec, cdeg[2]);
 else if (min != 0)
  return sprintf (str, "%d%c%d%c", min, cdeg[1], sec, cdeg[2]);
 else
  return sprintf (str, "%d%c", sec, cdeg[2]);
}

//---------------------------------------------------------------------------

int chr2str (char *str, unsigned char c)
{
 const char *const ch[] =
     // 0      1      2      3      4      5      6      7
     { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",  // 0
       "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",   // 1
       "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",  // 2
       "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US" }; // 3

 const char *const chc[] =
     // 0        1        2        3        4        5        6       7
     { "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\a",     // 0
       "\\b",   "\\t",   "\\n",   "\\v",   "\\004", "\\r",   "\\016", "\\017",   // 1
       "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",   // 2
       "\\030", "\\031", "\\032", "\\e",   "\\034", "\\035", "\\036", "\\037" }; // 3

 if (c < 32)
  return sprintf (str, "%s '%s'", ch[c], chc[c]);
 else if (c == '\\')
  return sprintf (str, "'\\' '%c'", c);
 else if (c == 255U)
  return sprintf (str, "'\\f' '%c'", c);
 else
  return sprintf (str, "'%c'", c);
}
//---------------------------------------------------------------------------
int wchr2str1 (char *str, int i)
{
 wchar_t wbuf[2];
 char cbuf[8];

 wbuf[0] = (wchar_t)i & 0xffff;
 wbuf[1] = L'\0';

 // Replace WideCharToMultiByte with standard function
 wcstombs (cbuf, wbuf, sizeof (cbuf));
 return sprintf (str, "'%s'W", cbuf);
}

int wchr2str (char *str, int i)
{
#ifdef _WIN_
 wchar_t wbuf[2];
 char cbuf[8];

 wbuf[0] = *(wchar_t *)&i;
 wbuf[1] = L'\0';

 if (i == '\t')
  cbuf[0] = ' ', cbuf[1] = '\0';
 else
  WideCharToMultiByte (CP_OEMCP, 0, wbuf, -1, (LPSTR)cbuf, 4, nullptr, nullptr);
 return sprintf (str, "'%s'W", cbuf);
#else  /*_WIN_*/
 *str = '\0';
 return 0;
#endif /*_WIN_*/
}


#ifdef __BORLANDC__
int nx_time2str (char *str, uint64_t time)
{
  struct tm t;

  if (time > 0x7FFFFFFF) // 2147483647 signed 32-bit (19 Jan 2038)
  {

    strcpy(str, "Date overflow (>2038)");
    return (int)strlen(str);
  }
  
  t = *gmtime((time_t*)&time);
  return (int)strftime(str, 80, "%a, %b %d %H:%M:%S %Y", &t);
}

#else
int nx_time2str (char *str, uint64_t time)
{
 struct tm t;

 // Checking that the time is within the valid range for time_t (32-bit unix time)
 // time_t is usually 32-bit and supports dates from 1970 to 2038
 if (time > 0x7FFFFFFF) // 2147483647 = maximum value for signed 32-bit (19 Jan 2038)
  {
   // For larger values, return an error message
   strcpy (str, "Date overflow (>2038)");
   return (int)strlen (str);
  }

 time_t safe_time = (time_t)time;
 gmtime_s (&t, &safe_time); // Using the safe version of the function with a checked value
 return (int)strftime (str, 80, "%a, %b %d %H:%M:%S %Y", &t);
}
#endif
void fraction (double val, double eps, int &num, int &denum)
{
 int a = 1, b = 1;
 int mn = 2;

 if (val < eps)
  {
   num = denum = 0;
   return;
  }

 double c = 1;
 do
  {
   b++;
   c = (double)a / b;
  }
 while ((val - c) < 0);

 if ((val - c) < eps)
  {
   num   = a;
   denum = b;
   return;
  }

 b--;
 c = (double)a / b;
 if ((val - c) > -eps)
  {
   num   = a;
   denum = b;
   return;
  }

 for (int iter = 0; iter < 20000; iter++)
  {
   int cc = a * mn, zz = b * mn;
   do
    {
     zz++;
     c = (double)cc / zz;
    }
   while ((val - c) < 0);
   if ((val - c) < eps)
    {
     num   = cc;
     denum = zz;
     return;
    }
   zz--;
   c = (double)cc / zz;
   if ((val - c) > -eps)
    {
     num   = cc;
     denum = zz;
     return;
    }
   mn++;
  }
}
//---------------------------------------------------------------------------

int d2frcstr (char *str, float__t d, int eps_order)
{
 int num, denum;
 double val;
 double eps;
 if ((eps_order > 0) && (eps_order < 7))
  eps = exp (-eps_order * log (10.0));
 else
  eps = 0.001;
 if (d > 0)
  val = d;
 else
  val = -d;
 double intpart = floor (val);
 if (intpart > 0)
  {
   fraction (d - intpart, eps, num, denum);
   if (denum)
    {
     if (d > 0)
      return sprintf (str, "%.0f+%d/%d", intpart, num, denum);
     else
      return sprintf (str, "-%.0f-%d/%d", intpart, num, denum);
    }
   else
    return sprintf (str, "%f", d);
  }
 else
  {
   fraction (val, eps, num, denum);
   if (denum)
    {
     if (d > 0)
      return sprintf (str, "%d/%d", num, denum);
     else
      return sprintf (str, "-%d/%d", num, denum);
    }
   else
    return sprintf (str, "%f", d);
  }
}
//---------------------------------------------------------------------------

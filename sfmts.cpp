//---------------------------------------------------------------------------
#include "pch.h"
#if defined(_WIN64)||defined(_WIN32)
#define _WIN_
#include <windows.h>
#endif
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <float.h>

#pragma hdrstop
#include "scalc.h"
#include "sfmts.h"

//---------------------------------------------------------------------------
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)



#define M_PI	3.14159265358979323846
#define M_E		2.7182818284590452354

//int _matherr(struct _exception *e)
//{
//  return 1;             /* error has been handled */
//}

//----------------------------------
int ones(unsigned char *cp, int from, int to)
{
 int res = 0;
 for(int i = from; i < to; i++)
   if (cp[i/8] & (1 << (i%8))) res++;
 return res;
}
//---------------------------------------------------------------------------

bool isNan(float__t d)
{
 int fr, ex;
 if (d < 0) d = -d;
 if (sizeof(d) == 80/8) //IEEE 754 80 bits
  {
   fr = ones((unsigned char *)&d, 0, 64);
   ex = ones((unsigned char *)&d, 64, 80);
   return ((ex >= 15) && fr);
  }
 else
 if (sizeof(d) == 64/8) //IEEE 754 64 bits
  {
   fr = ones((unsigned char *)&d, 0, 52);
   ex = ones((unsigned char *)&d, 52, 64);
   return ((ex >= 11) && fr);
  }
 else
 if (sizeof(d) == 32/8) //IEEE 754 32 bits
  {
   fr = ones((unsigned char *)&d, 0, 23);
   ex = ones((unsigned char *)&d, 23, 32);
   return ((ex >= 8) && fr);
  }
 return false;
}
//---------------------------------------------------------------------------

const uint64_t dms[] =
   {(60ull*60*60*24*36525ull),(6ull*6*24*36525),
    (60ull*60*24), (60ull*60), 60ull, 1ull};

int t2str(char *str, uint64_t sec)
{
 const char * fmt[] =
   {"%lld:c ", "%lld:y ", "%lld:d ", "%lld:h ", "%lld:m ", "%lld:s "};
 uint64_t pt[6];
 int i, j, k;
 char *pc = str;

 for(i = 0, j = -1, k = 0; i < 6; i++)
  {
    pt[i] = (uint64_t)(sec / dms[i]);
    sec %= dms[i];
    if ((j == -1) && (pt[i] != 0)) j = i;
    if ((j != -1) && (pt[i] != 0)) k = i;
  }
 *str = '\0';
 if (j == -1) str += sprintf(str, "0s");
 else
 for(i = j; i <= k; i++)
   str += sprintf(str, fmt[i], pt[i]);
 return str-pc;
}
//---------------------------------------------------------------------------

/* convert int to bin according format %10b */
int b2str(char *str, const char *fmt, uint64_t b)
{
  char c;
  bool flag = false;
  unsigned int j = 0;
  char *ws = NULL;
  unsigned int wide;
  unsigned int i,w=0;
  uint64_t mask;

  while(0 != (c = *fmt++))
   {
    if (!flag && (c == '%')) {flag = true; ws = (char*)fmt; continue;}
    if (flag && (c == '%')) {flag = false; continue;}
    if (flag && (c == 'b'))
     {
      for(wide = 0; ws < fmt; ws++)
       {
        c = *ws;
        if (c == '-') continue;
        if ((c >= '0') && (c <= '9')) wide = wide * 10 + (c - '0');
        else break;
       }

      mask = 1ull << 63;
      for(w = wide; ((w > 0) && (mask > 1) && ((b & mask) == 0)); w--) mask >>=1;
      if (wide <= w) wide = w;

      mask = 1ull << (wide-1);
      for(i = 0; i < wide; i++)
       {
        if (b & mask) str[j++] = '1';
        else str[j++] = '0';
        mask >>=1;
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

int d2scistr(char *str, float__t d)
{
 const char csci[] = {'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm', ' ',
                      'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
 enum {yocto, zepto, atto, femto, pico, nano, micro, milli, empty,
      kilo, mega, giga, tera, peta, exa, zetta, yotta};

 if (isNan(d)) return sprintf(str, "%e", d);
 else
  {
   #ifdef _long_double_
   float__t dd = fabsl(d);
   #else /*_long_double_*/
   float__t dd = fabs(d);
   #endif /*_long_double_*/
   int rng = empty;
   if (dd > 0)
    {
     while (dd >= 1000)
      {
       rng++;
       dd /= 1000;
       if (rng > yotta) break;
      }
     while (dd < 1)
      {
       rng--;
       dd *= 1000;
       if (rng < yocto) break;
      }
    }
   if (d < 0) dd = -dd;
   #ifdef _long_double_
   if (rng == empty)
    return sprintf(str, "%.4Lg", d);
   else
   if ((rng >= yocto) && (rng <= yotta))
    return sprintf(str, "%.4Lg%c", dd, csci[rng]);
   else
    return sprintf(str, "%.2Le", d);
   #else /*_long_double_*/
   if (rng == empty)
    return sprintf(str, "%.4g", d);
   else
   if ((rng >= yocto) && (rng <= yotta))
    return sprintf(str, "%.4g%c", dd, csci[rng]);
   else
    return sprintf(str, "%.2e", d);
   #endif /*_long_double_*/
  }
}
//---------------------------------------------------------------------------

int d2nrmstr(char *str, float__t d)
{
 if (isNan(d)) return sprintf(str, "%e", d);
 else
  {
   #ifdef _long_double_
   float__t dd = fabsl(d);
   #else /*_long_double_*/
   float__t dd = fabs(d);
   #endif /*_long_double_*/
   int rng = 0;
   if (dd > 0)
    {
     while (dd >= 1000)
      {
       rng++;
       dd /= 1000;
      }
     while (dd < 1)
      {
       rng--;
       dd *= 1000;
      }
    }
   if (d < 0) dd = -dd;
   #ifdef _long_double_
   if (rng == 0) return sprintf(str, "%.4Lg", d);
   else return sprintf(str, "%.4Lge%i", dd, rng*3);
   #else /*_long_double_*/
   if (rng == 0) return sprintf(str, "%.4g", d);
   else return sprintf(str, "%.4ge%i", dd, rng*3);
   #endif /*_long_double_*/
  }
}
//---------------------------------------------------------------------------

int b2scistr(char *str, float__t d)
{
 const char csci_plus[] =  {' ', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
 enum {Empty, Kilo, Mega, Giga, Tera, Peta, Exa, Zetta, Yotta};

 if (isNan(d)) return sprintf(str, "%e", d);
 else
  {
   #ifdef _long_double_
   float__t dd = fabsl(d);
   #else /*_long_double_*/
   float__t dd = fabs(d);
   #endif /*_long_double_*/
   int rng = Empty;
   if (dd > 0)
    {
     while (dd >= 1024)
      {
       rng++;
       dd /= 1024;
      }
    }
   if (d < 0) dd = -dd;

   #ifdef _long_double_
   if ((rng > Empty) && (rng <= Yotta))
    return sprintf(str, "%.4Lg%cB", dd, csci_plus[rng]);
   else
    return sprintf(str, "%.4Lg", d);
   #else /*_long_double_*/
   if ((rng > Empty) && (rng <= Yotta))
    return sprintf(str, "%.4g%cB", dd, csci_plus[rng]);
   else
    return sprintf(str, "%.4g", d);
   #endif /*_long_double_*/
  }
}
//---------------------------------------------------------------------------

int dgr2str(char *str, float__t d)
{
 const char cdeg[] = {96, 39, 34}; //` - degrees, ' - minutes, " - seconds
 const float__t mdeg[] = {M_PI/180.0, M_PI/(180.0*60), M_PI/(180.0*60*60)};
 unsigned int pt[6];
 int i, j, k;
 char *pc = str;

 for(i = 0, j = -1, k = 0; i < 3; i++)
  {
    pt[i] = (unsigned int)(d / mdeg[i]);
    d = fmod(d, mdeg[i]);
    if ((j == -1) && (pt[i] != 0)) j = i;
    if ((j != -1) && (pt[i] != 0)) k = i;
  }
 *str = '\0';
 if (j == -1) str += sprintf(str, "0%c",cdeg[2]);
 else
 for(i = j; i <= k; i++)
   str += sprintf(str, "%d%c", pt[i], cdeg[i]);
 return str-pc;
}
//---------------------------------------------------------------------------

int chr2str(char *str, unsigned char c)
{
 const char* const ch[] =
 // 0        1        2        3        4        5        6        7
  {"NUL",   "SOH",   "STX",   "ETX",   "EOT",   "ENQ",   "ACK",   "BEL",  //0
   "BS",    "HT",    "LF",    "VT",    "FF",    "CR",    "SO",    "SI",   //1
   "DLE",   "DC1",   "DC2",   "DC3",   "DC4",   "NAK",   "SYN",   "ETB",  //2
   "CAN",   "EM",    "SUB",   "ESC",   "FS",    "GS",    "RS",    "US"};  //3

 const char* const chc[] =
 // 0        1        2        3        4        5        6       7
  {"\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\a",    //0
   "\\b",   "\\t",   "\\n",   "\\v",   "\\004", "\\r",   "\\016", "\\017",  //1
   "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",  //2
   "\\030", "\\031", "\\032", "\\e",   "\\034", "\\035", "\\036", "\\037"}; //3

 if (c < 32) return sprintf(str, "%s '%s'", ch[c], chc[c]);
 else
 if (c == '\\') return sprintf(str, "'\\' '%c'", c);
 else
 if (c == 255U) return sprintf(str, "'\\f' '%c'", c);
 else return sprintf(str, "'%c'", c);
}
//---------------------------------------------------------------------------
int wchr2str1(char *str, int i)
{
 wchar_t wbuf[2];
 char cbuf[8];

 wbuf[0] = (wchar_t)i&0xffff;
 wbuf[1] = L'\0';

 // Заменяем WideCharToMultiByte на стандартную функцию
 wcstombs(cbuf, wbuf, sizeof(cbuf));
 return sprintf(str, "'%s'W", cbuf);
}

int wchr2str(char *str, int i)
{
#ifdef _WIN_
  wchar_t wbuf[2];
  char cbuf[8];

  wbuf[0] = *(wchar_t *)&i;
  wbuf[1] = L'\0';

  if (i == '\t') cbuf[0] = ' ', cbuf[1] = '\0'; 
  else WideCharToMultiByte(CP_OEMCP, 0, wbuf, -1, (LPSTR)cbuf, 4, NULL, NULL);
  return sprintf(str, "'%s'W", cbuf);
#else /*_WIN_*/
 *str = '\0';
 return 0;
#endif  /*_WIN_*/
}
//---------------------------------------------------------------------------

//   https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
//   #include <winbase.h>
//   #include <winnt.h>
//   #include <time.h>
//
//   void UnixTimeToFileTime(time_t t, LPFILETIME pft)
//   {
//     // Note that LONGLONG is a 64-bit value
//     LONGLONG ll;
//
//     ll = Int32x32To64(t, 10000000) + 116444736000000000;
//     pft->dwLowDateTime = (DWORD)ll;
//     pft->dwHighDateTime = ll >> 32;
//   }

int nx_time2str(char *str, uint64_t time)
{
  struct tm t;
  
  // Проверяем что время находится в допустимом диапазоне для time_t (32-bit unix time)
  // time_t обычно 32-битный и поддерживает даты от 1970 до 2038 года
  if (time > 0x7FFFFFFF) // 2147483647 = максимальное значение для signed 32-bit (19 Jan 2038)
  {
    // Для больших значений возвращаем сообщение об ошибке
    strcpy(str, "Date overflow (>2038)");
    return (int)strlen(str);
  }
  
  time_t safe_time = (time_t)time;
  gmtime_s(&t, &safe_time);  // Используем безопасную версию функции с проверенным значением
  return (int)strftime(str, 80, "%a, %b %d %H:%M:%S %Y", &t);
}
//int nx_time2str(char *str, __int64 time)
//{
// struct tm t = *gmtime((time_t*)&time);
// return strftime(str, 80, "%a, %b %d %H:%M:%S %Y", &t);
// //return sprintf(str, "%s", ctime((time_t*)&time));
//}
//---------------------------------------------------------------------------


void fraction(double val, double eps, int &num, int &denum)
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
    num = a; denum = b;
    return;
   }

  b--;
  c = (double)a / b;
  if ((val - c) > -eps)
   {
    num = a; denum = b;
    return;
   }

  for(int iter = 0; iter < 20000; iter++)
   {
    int cc = a*mn, zz = b*mn;
    do
     {
      zz++;
      c = (double)cc / zz;
     }
    while ((val - c) < 0);
    if ((val - c) < eps)
     {
      num = cc; denum = zz;
      return;
     }
    zz--;
    c = (double)cc / zz;
    if ((val - c) > -eps)
     {
      num = cc; denum = zz;
      return;
     }
    mn++;
   }
}
//---------------------------------------------------------------------------

int d2frcstr(char *str, float__t d, int eps_order)
{
 int num, denum;
 double val;
 double eps;
 if ((eps_order > 0) && (eps_order < 7)) eps = exp(-eps_order*log(10.0));
 else eps = 0.001;
 if (d > 0) val = d;
 else val = -d;
 double intpart = floor(val);
 if (intpart > 0)
  {
   fraction(d-intpart, eps, num, denum);
   if (denum)
    {
     if (d > 0) return sprintf(str, "%.0f+%d/%d", intpart, num, denum);
     else return sprintf(str, "-%.0f-%d/%d", intpart, num, denum);
    }
   else return sprintf(str, "%f", d);
  }
 else
  {
   fraction(val, eps, num, denum);
   if (denum)
    {
     if (d > 0) return sprintf(str, "%d/%d", num, denum);
     else return sprintf(str, "-%d/%d", num, denum);
    }
   else return sprintf(str, "%f", d);
  }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
int format_out (int Options, int scfg, int binwide, int n, float__t fVal,
				int64_t iVal, char *expr, char strings[20][80], calculator *ccalc)
{
 if (IsNaN(fVal))
  {
   if (ccalc->error()[0])
	{
	 if (ccalc->errps() < 64)
	  {
	   char binstr[80];
	   memset(binstr, '-', sizeof binstr);
	   binstr[ccalc->errps()] = '^';
	   binstr[ccalc->errps() + 1] = '\0';
	   sprintf(strings[n++], "%-64s", binstr);
	   sprintf(strings[n++], "  %66.66s  ", ccalc->error());
	  }
	 else
	  {
	   sprintf(strings[n++], "%66.66s ", ccalc->error());
	  }
	}
   else
	{
	 if (expr[0]) sprintf(strings[n++], "%66.66s ", "NaN");
	 else sprintf(strings[n++], "%66.66s ", " ");

	 // (RO) String format found
	 if ((Options & STR) || (scfg & STR))
	  {
	   if (Options & AUTO)
		{
		 if (ccalc->Sres()[0])
		  {
		   char strcstr[80];
		   sprintf(strcstr, "'%s'", ccalc->Sres());
		   if (strcstr[0]) sprintf(strings[n++], "%65.64s", strcstr);
		  }
		}
	   else
		{
		 if (ccalc->Sres()[0])
		  {
		   char strcstr[80];
		   sprintf(strcstr, "'%s'", ccalc->Sres());
		   sprintf(strings[n++], "%65.64s", strcstr);
		  }
		 else sprintf(strings[n++], "%65.64s S", "''");
		}
	  }
	}
  }
 else
  {
   // (WO) Forced float
   if (Options & FFLOAT)
	sprintf(strings[n++], "%65.16Lg f", (long double) fVal);

   // (RO) Scientific (6.8k) format found
   if ((Options & SCF) || (scfg & SCF))
	{
	 char scistr[80];
	 d2scistr(scistr, fVal);
	 sprintf(strings[n++], "%65.64s S", scistr);
	}

   // (UI) Normalized output
   if (Options & NRM)
	{
	 char nrmstr[80];
	 d2nrmstr(nrmstr, fVal);
	 sprintf(strings[n++], "%65.64s n", nrmstr);
	}

   // (RO) Computing format found
   if ((Options & CMP) || (scfg & CMP))
	{
	 char bscistr[80];
	 b2scistr(bscistr, fVal);
	 sprintf(strings[n++], "%65.64s c", bscistr);
	}

   // (UI) Integer output
   if (Options & IGR)
	{
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%65lld i", iVal);
	  }
	 else sprintf(strings[n++], "%65lld i", iVal);
	}

   // (UI) Unsigned output
   if (Options & UNS)
	{
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%65llu u", iVal);
	  }
	 else sprintf(strings[n++], "%65llu u", iVal);
	}

   // (UI) Fraction output
   if (Options & FRC)
	{
	 char frcstr[80];
	 int num, denum;
	 double val;
	 if (fVal > 0) val = fVal;
	 else val = -fVal;
	 double intpart = floor(val);
	 if (intpart > 0)
	  {
	   fraction(val - intpart, 0.001, num, denum);
	   if (fVal > 0) sprintf(frcstr, "%.0f+%d/%d", intpart, num, denum);
	   else	sprintf(frcstr, "-%.0f-%d/%d", intpart, num, denum);
	  }
	 else
	  {
	   fraction(val, 0.001, num, denum);
	   if (fVal > 0) sprintf(frcstr, "%d/%d", num, denum);
	   else sprintf(frcstr, "-%d/%d", num, denum);
	  }
	 if (denum) sprintf(strings[n++], "%65.64s F", frcstr);
	}

   // (UI) Fraction inch output
   if (Options & FRI)
	{
	 char frcstr[80];
	 int num, denum;
	 double val;
	 if (fVal > 0) val = fVal;
	 else val = -fVal;
	 val /= 25.4e-3;
	 double intpart = floor(val);
	 if (intpart > 0)
	  {
	   fraction(val - intpart, 0.001, num, denum);
	   if (num && denum)
		{
		 if (fVal > 0) sprintf(frcstr, "%.0f+%d/%d", intpart, num, denum);
		 else sprintf(frcstr, "-%.0f-%d/%d", intpart, num, denum);
		}
	   else
		{
		 sprintf(frcstr, "%.0f", intpart);
		}
	  }
	 else
	  {
	   fraction(val, 0.001, num, denum);
	   if (fVal > 0) sprintf(frcstr, "%d/%d", num, denum);
	   else sprintf(frcstr, "-%d/%d", num, denum);
	  }
	 sprintf(strings[n++], "%65.64s \"", frcstr);
	}

   // (RO) Hex format found
   if ((Options & HEX) || (scfg & HEX))
	{
	 char binfstr[16];
	 sprintf(binfstr, "%%64.%illxh  ", binwide / 4);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], binfstr, iVal);
	  }
	 else sprintf(strings[n++], binfstr, iVal);
	}

   // (RO) Octal format found
   if ((Options & OCT) || (scfg & OCT))
	{
	 char binfstr[16];
	 sprintf(binfstr, "%%64.%illoo  ", binwide / 3);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], binfstr, iVal);
	  }
	 else sprintf(strings[n++], binfstr, iVal);
	}

   // (RO) Binary format found
   if ((Options & fBIN) || (scfg & fBIN))
	{
	 char binfstr[16];
	 char binstr[80];
	 sprintf(binfstr, "%%%ib", binwide);
	 b2str(binstr, binfstr, iVal);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%64.64sb  ", binstr);
	  }
	 else sprintf(strings[n++], "%64.64sb  ", binstr);
	}

   // (RO) Char format found
   if ((Options & CHR) || (scfg & CHR))
	{
	 char chrstr[80];
	 chr2str(chrstr, iVal);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%64.64s c", chrstr);
	  }
	 else sprintf(strings[n++], "%64.64s c", chrstr);
	}

   // (RO) WChar format found
   if ((Options & WCH) || (scfg & WCH))
	{
	 char wchrstr[80];
	 int i = iVal & 0xffff;
	 wchr2str(wchrstr, i);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%64.64s c", wchrstr);
	  }
	 else sprintf(strings[n++], "%64.64s c", wchrstr);
	}

   // (RO) Date time format found
   if ((Options & DAT) || (scfg & DAT))
	{
	 char dtstr[80];
	 t2str(dtstr, iVal);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%65.64s ", dtstr);
	  }
	 else sprintf(strings[n++], "%65.64s ", dtstr);
	}

   // (RO) Unix time
   if ((Options & UTM) || (scfg & UTM))
	{
	 char dtstr[80];
	 nx_time2str(dtstr, iVal);
	 if (Options & AUTO)
	  {
	   if ((fVal - iVal) == 0) sprintf(strings[n++], "%65.64s  ", dtstr);
	  }
	 else sprintf(strings[n++], "%65.64s  ", dtstr);
	}

   // (RO) Degrees format found
   if ((Options & DEG) || (scfg & DEG))
	{
	 char dgrstr[80];
	 dgr2str(dgrstr, fVal);
	 sprintf(strings[n++], "%65.64s  ", dgrstr);
	}

   // (RO) String format found
   if ((Options & STR) || (scfg & STR))
	{
	 if (Options & AUTO)
	  {
	   if (ccalc->Sres()[0])
		{
		 char strcstr[80];
		 sprintf(strcstr, "'%s'", ccalc->Sres());
		 if (strcstr[0]) sprintf(strings[n++], "%65.64s S", strcstr);
		}
	   else sprintf(strings[n++], "%65.64s S", "''");
	  }
	 else
	  {
	   if (ccalc->Sres()[0])
		{
		 char strcstr[80];
		 sprintf(strcstr, "'%s'", ccalc->Sres());
		 sprintf(strings[n++], "%65.64s S", strcstr);
		}
	   else sprintf(strings[n++], "%65.64s S", "''");
	  }
	}
  }

 return n++;
}
//---------------------------------------------------------------------------

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
//---------------------------------------------------------------------------

#pragma region Output Formatting
  //---------------------------------------------------------------------------
// Print matrix result in a formatted way, with an option for a new line
// and an optional pointer to store the size of the output
int calculator::mxprint (int8_t res_rows, int8_t res_cols, float__t *res_mval, 
                         char *str, bool nl, int *size)
{
 int n     = 0;
 int bsize = 0;
 if (result_tag == tvMATRIX)
  {
   // compute Frobenius norm (RMS) for threshold
   double norm = 0.0L;
   int nm = res_rows * res_cols;
   for (int i = 0; i < nm; i++) norm += (double)res_mval[i] * (double)res_mval[i];
   norm = sqrt (norm);
   double threshold = norm * 1e-9L;

   char mstr[80];
   for (int i = 0; i < res_rows; i++)
    {
     char *cp = mstr;
     if (nl)
      {
       if (i > 0) cp += sprintf (cp, " (");
       else cp += sprintf (cp, "[(");
       for (int j = 0; j < res_cols; j++)
        {
         char elemstr[20];
         double elem = (double)res_mval[i * res_cols + j];
         if (fabs (elem) < threshold) elem = 0.0L; // suppress numerical noise
         d2scistr (elemstr, elem);
         if (j < res_cols - 1) cp += sprintf (cp, "%7.7s, ", elemstr);
         else
          {
           if (i == res_rows - 1) cp += sprintf (cp, "%7.7s)]", elemstr);
           else cp += sprintf (cp, "%7.7s); ", elemstr);
          }
        }
       if (i == res_rows - 1) cp += sprintf (cp, " ");
       bsize += sprintf (str + bsize, "%65.64s\r\n", mstr);
       n++;
      }
     else
      {
       if (i > 0) cp += sprintf (cp, "(");
       else cp += sprintf (cp, "[(");
       for (int j = 0; j < res_cols; j++)
        {
         char elemstr[20];
         double elem = (double)res_mval[i * res_cols + j];
         if (fabs (elem) < threshold) elem = 0.0L; // suppress numerical noise
         d2scistr (elemstr, elem);
         if (j < res_cols - 1)
          cp += sprintf (cp, "%s, ", elemstr);
         else
          {
           if (i == res_rows - 1) cp += sprintf (cp, "%s)]", elemstr);
           else cp += sprintf (cp, "%s); ", elemstr);
          }
        }
       bsize += sprintf (str + bsize, "%s", mstr);
       n++;
      }
    }
   if (size) *size += bsize;
   return n;
  }
 return n;
}

#define OPTS sizeof (all_options) / sizeof (option_def)
int32_t scan_opt (char *str, int &opts)
{
 struct option_def
 {
  char name[4]; // Option name
  int flag;     // Bit mask
 };

 // All supported options definitions
 static const option_def all_options[] = {
  { "DEG", DEG },  { "ENG", ENG }, { "STR", STR }, { "HEX", HEX }, { "OCT", OCT },
  { "BIN", FBIN},  { "DAT", DAT }, { "CHR", CHR }, { "WCH", WCH }, { "CMP", CMP },
  { "NRM", NRM },  { "IGR", IGR }, { "UNS", UNS }, { "FRC", FRC }, { "FRI", FRI }, 
  { "FRH", FRH },  { "FLT", FLT }, { "UTM", UTM }, { "FCT", FCTR},
  { "ALL", DEG + ENG + STR + HEX + OCT + FBIN + DAT + CHR + WCH + CMP + NRM + IGR 
         + UNS + FRC + FRI + FRH + UTM + FLT + FCTR},
  { ""   , 0 } // Sentinel
 };
 unsigned int i, j, k, l;
 char c, cc;

 l = 0;
 while (str[l])
  {
   // Skip whitespace
   while (str[l] && (str[l] == ' ' || str[l] == '\t' || str[l] == '\r' || str[l] == '\n')) l++;

   if (!str[l]) break;
   // Search for an option
   if (str[l] != '/')
    {
     l++;
     continue;
    }

   l++; // Skip '/'
   // Search for a matching option
   bool found = false;
   for (i = 0; i < OPTS - 1; i++) // -1 to avoid checking NULL sentinel
    {
     j = l;
     k = 0;

     // Compare option name
     while (all_options[i].name[k])
      {
       c = str[j];
       if (c >= 'a' && c <= 'z') c -= ('a' - 'A'); // To upper
       cc = all_options[i].name[k];

       if (c != cc) break;

       j++;
       k++;
      }

     // Check if the name matched completely
     if (all_options[i].name[k] == '\0')
      {
       c = str[j];
       if (c == '+' || c == '-')
        {
         // Found the option!
         if (c == '+')
          opts |= all_options[i].flag;
         else
          opts &= ~all_options[i].flag;

         l     = j + 1;
         found = true;
         break;
        }
      }
    }
  }
 return opts;
}

int qprint (char *str, float__t re, float__t im, int prec, char c_imaginary)
{
 #ifdef _float128_
 char imstr[80];
 char restr[80];
 sprintf (restr, "%.*g", prec, (double)re); // if quadmath_snprintf is not available, 
                                       //use sprintf as a fallback for compatibility 
 quadmath_snprintf (restr, 80, "%.*Qg", prec, re);
 if ((double)im != 0.0)
  {
   sprintf (imstr, "%+.*g", prec, (double)im);
   quadmath_snprintf (imstr, 80, "%+.*Qg", prec, im);
   return sprintf (str, "%s%s%c", restr, imstr, c_imaginary);
  }
 else return sprintf (str, "%s", restr);
 #else
 if ((double)im == 0.0)
  return sprintf (str, "%.*Lg", prec, re);
 else
  return sprintf (str, "%.*Lg%+.*Lg%c", prec, re, prec, im, c_imaginary);
#endif
}

bool is_integer (float__t val)
{
 if (isnan (val) || isinf (val)) return false;
 if (val > (float__t)INT64_MAX || val < (float__t)INT64_MIN) return false;
 return true;
}

// Print the result string to the input
int calculator::printres(char* str, int options, int binwide)
{
 if (isnan (result_fval) || err[0])
  {
   return sprintf (str, "%s", err[0] ? err : "NaN");
  }
 if (result_tag == tvMATRIX)
  {
    return mxprint (str, false);
  }
 else
  {
   if (options & AUTO)
    {
     if (sres[0])
      return sprintf (str, "%s", sres);
     else
      {
/*
  { "DEG", DEG },  { "ENG", ENG }, { "STR", STR }, { "HEX", HEX }, { "OCT", OCT },
  { "BIN", FBIN }, { "DAT", DAT }, { "CHR", CHR }, { "WCH", WCH }, { "CMP", CMP },
  { "NRM", NRM },  { "IGR", IGR }, { "UNS", UNS }, { "FRC", FRC }, { "FRI", FRI },
  { "FRH", FRH },  { "FLT", FLT }, { "UTM", UTM }, { "FCT", FCTR}, { ""   , 0 } // Sentinel
*/
       if (fflags & DEG) return printres (str, DEG, binwide);
       if (fflags & FCTR) return printres (str, FCTR, binwide);
       if (fflags & FLT) return printres (str, FFLOAT, binwide);
       if (fflags & ENG) return printres (str, SCI, binwide);
       if (fflags & NRM) return printres (str, NRM, binwide);
       if (fflags & FRH) return printres (str, FRH, binwide);
       if (fflags & CMP) return printres (str, CMP, binwide);
       if (fflags & DAT) return printres (str, DAT, binwide);
       if (fflags & IGR) return printres (str, IGR, binwide);
       if (fflags & UNS) return printres (str, UNS, binwide);
       if (fflags & FRC) return printres (str, FRC, binwide);
       if (fflags & FRI) return printres (str, FRI, binwide);
       if (fflags & UTM) return printres (str, UTM, binwide);
       if (fflags & CHR) return printres (str, CHR, binwide);
       if (fflags & WCH) return printres (str, WCH, binwide);

       if ((fflags & HEX) && ((float__t)result_ival == result_fval))
        return printres (str, HEX, binwide);
       if ((fflags & OCT) && ((float__t)result_ival == result_fval)) return printres (str, OCT, binwide);
       if ((fflags & fBIN) && ((float__t)result_ival == result_fval)) return printres (str, fBIN, binwide);
       if ((float__t)result_ival == result_fval) return printres (str, IGR, binwide);
       if (fflags & STR) return printres (str, STR, binwide);
       // Here quadmath_snprintf (buf, sizeof (buf), fmt, val)
       return qprint (str, result_fval, result_imval, fprec, c_imaginary);
      }
    }

   if (options & (FFLOAT|FLT))
    {
     // Here quadmath_snprintf (buf, sizeof (buf), fmt, val)
     return qprint (str, result_fval, result_imval, fprec, c_imaginary);
    }

   if (options & SCI)
    {
     char scistr[80];
     if (result_imval == 0)
      d2scistr (scistr, (double)result_fval);
     else
      {
       char cphi[24];
       char cr[24];
       char *cp  = scistr;
       double im = (double)result_imval;
       double re = (double)result_fval;
       double phi = atan2 (im, re);
       double r = hypot (re, im);
       d2scistr (cr, r);
       dgr2str (cphi, phi);
       cp += sprintf (cp, "|%s|(%s) ", cr, cphi);
       normz (re, im);
       cp += d2scistr (cp, re);
       if (im >= 0) *cp++ = '+';
       cp += d2scistr (cp, im);
       *cp++ = c_imaginary;
       *cp   = '\0';
      }
     return sprintf (str, "%s", scistr);
    }

   if (options & NRM)
    {
     char nrmstr[80];
     double im = (double)result_imval;
     double re = (double)result_fval;
     normz (re, im);
     if (im == 0.0) d2nrmstr (nrmstr, re);
     else
      {
       char cphi[24];
       char cr[24];
       double phi = atan2 (im, re);
       double r = hypot (re, im);
       d2nrmstr (cr, r);
       dgr2str (cphi, phi);
       sprintf (nrmstr, "|%s|(%s) %.*g%+.*g%c", cr, cphi, fprec, re, fprec, im, c_imaginary);
      }
     return sprintf (str, "%s", nrmstr);
    }

   if (options & CMP)
    {
     char bscistr[80];
     b2scistr (bscistr, (double)result_fval);
     return sprintf (str, "%s", bscistr);
    }
   if (options & IGR)
    {
     return sprintf (str, "%lld", result_ival);
    }
   if (options & UNS)
    {
     return sprintf (str, "%llu", result_ival); //%llu|%zu
    }
   if (options & FRC)
    {
     char frcstr[80];
     int num, denum;
     double val;
     if ((double)result_fval > 0.0)
      val = (double)result_fval;
     else
      val = -(double)result_fval;
     double intpart = floor (val);
     if (intpart < 1e15)
      {
       if (intpart > 0)
        {
         fraction (val - intpart, 0.001, num, denum);
         if ((double)result_fval > 0.0)
          sprintf (frcstr, "%.0f+%d/%d", intpart, num, denum);
         else
          sprintf (frcstr, "-%.0f-%d/%d", intpart, num, denum);
        }
       else
        {
         fraction (val, 0.001, num, denum);
         if ((double)result_fval > 0.0)
          sprintf (frcstr, "%d/%d", num, denum);
         else
          sprintf (frcstr, "-%d/%d", num, denum);
        }
       if (denum)
        {
         return sprintf (str, "%s", frcstr);
        }
      }
    }

   if (options & FRI)
    {
     char frcstr[80];
     int num, denum;
     double val;
     if (result_fval > 0)
      val = (double)result_fval;
     else
      val = -(double)result_fval;
     val /= 25.4e-3;
     double intpart = floor (val);
     if (intpart < 1e15)
      {
       if (intpart > 0)
        {
         fraction (val - intpart, 0.001, num, denum);
         if (num && denum)
          {
           if ((double)result_fval > 0.0)
            sprintf (frcstr, "%.0f+%d/%d", intpart, num, denum);
           else
            sprintf (frcstr, "-%.0f-%d/%d", intpart, num, denum);
          }
         else
          {
           sprintf (frcstr, "%.0f", intpart);
          }
        }
       else
        {
         fraction (val, 0.001, num, denum);
         if ((double)result_fval > 0.0)
          sprintf (frcstr, "%d/%d", num, denum);
         else
          sprintf (frcstr, "-%d/%d", num, denum);
        }
       return sprintf (str, "%s", frcstr);
      }
    }

   if ((options & HEX) && is_integer (result_fval))
    {
     char binfstr[16];
     sprintf (binfstr, "0x%%0.%illx", binwide / 4);
     return sprintf (str, binfstr, result_ival);
    }

   if ((options & OCT) && is_integer (result_fval))
    {
     char binfstr[16];
     sprintf (binfstr, "0o%%0.%illo", binwide / 3);
     return sprintf (str, binfstr, result_ival);
    }

   if ((options & fBIN) && is_integer (result_fval))
    {
     char binfstr[16];
     char binstr[80];
     sprintf (binfstr, "%%%ib", binwide);
     b2str (binstr, binfstr, result_ival);
     return sprintf (str, "%s", binstr);
    }

   if ((options & FCTR) && (result_imval == 0) && is_integer (result_fval))
    {
     char fctrstr[80];
     factorize_p (fctrstr, (scfg & PAS), result_ival);
     return sprintf (str, "%s", fctrstr);
    }

   if ((options & CHR) && is_integer (result_fval))
    {
     char chrstr[16];
     chr2str (chrstr, result_ival);
     return sprintf (str, "%s", chrstr);
    }

   if ((options & WCH) && is_integer (result_fval))
    {
     char wchrstr[16];
     int i = result_ival & 0xffff;
     wchr2str (wchrstr, i);
     return sprintf (str, "%s", wchrstr);
    }

   if ((options & DAT) && is_integer (result_fval))
    {
     char dtstr[80];
     t2str (dtstr, result_ival);
     return sprintf (str, "%s", dtstr);
    }

   if ((options & UTM) && is_integer (result_fval))
    {
     char dtstr[80];
     nx_time2str (dtstr, result_ival);
     return sprintf (str, "%s", dtstr);
    }

   if (options & DEG)
    {
     char dgrstr[80];
     char *cp = dgrstr;
     double angle = (double)result_fval;
     cp += sprintf (cp, "%.6g rad|", angle);
     cp += dgr2str (cp, angle);
     cp += sprintf (cp, " (%.6g`)", (double)(angle * 180.0 / (double)M_PId));
     cp += sprintf (cp, "|%.4g gon", (double)(angle * 200.0 / (double)M_PId));
     cp += sprintf (cp, "|%.4g turn", (double)(angle * 0.5 / (double)M_PId));
     return sprintf (str, "%s", dgrstr);
    }

   if ((options & FRH) && (result_fval > -273.15))
    {
     char frhstr[80];
     double temperature = (double)result_fval;
     sprintf (frhstr, "%.6g K|%.6g `C|%.6g `F", (double)(temperature + 273.15),
              temperature, (double)(temperature * 9.0 / 5.0 + 32.0));
     return sprintf (str, "%s", frhstr);
    }

   if (options & STR) return sprintf (str, "'%s'", sres);
  }
 return sprintf (str, "%s", "");
}

// Print the result of the calculation into the provided string buffer with formatting options
int calculator::print (char *str, int Options, int binwide, int *size)
{
 int n     = 0;
 int bsize = 0;
 if (!expr)
  {
   bsize += sprintf (str + bsize, "%66.66s \r\n", " ");
   if (size) *size = bsize;
   n++;
   return n;
  }

 if (isnan (result_fval)||result_tag == tvERR)
  {
   if (err[0])
    {
     int ep = errpos;
     if (ep < 0) ep = 0;
     if (ep > 0) ep--; // Move the error position to the character before it
     if ((ep < 64))
      {
       char binstr[80];
       memset (binstr, ' ', sizeof (binstr));
       memset (binstr, '-', ep);
       binstr[ep]                  = '^';
       binstr[sizeof (binstr) - 1] = '\0';
       bsize += sprintf (str + bsize, "%64.64s   \r\n", binstr);
       n++;
       bsize += sprintf (str + bsize, "%67.67s\r\n", err);
       n++;
      }
     else
      {
       bsize += sprintf (str + bsize, "%67.67s\r\n", err);
       n++;
      }
    }
   else
    {
     binwide = 8 * (binwide / 8);
     if (binwide < 8) binwide = 8;
     if (binwide > 64) binwide = 64;
     if (expr)
      bsize += sprintf (str + bsize, "%66.66s \r\n", "NaN");
     else
      bsize += sprintf (str + bsize, "%66.66s \r\n", " ");
     n++;

     // (RO) String format found
     if (((Options & STR) || (fflags & STR)) && (result_imval == 0))
      {
        {
         if (sres[0])
          {
           char strcstr[80];
           sprintf (strcstr, "'%s'", sres);
           bsize += sprintf (str + bsize, "%65.64s\r\n", strcstr);
           n++;
          }
         else
          {
           bsize += sprintf (str + bsize, "%65.64s S\r\n", "''");
           n++;
          }
        }
      }
    }
  }
 else
  {
   if (result_tag == tvMATRIX)
    {
     n += mxprint (res_rows, res_cols, res_mval, str, true, &bsize);
     if (size) *size = bsize;
     return n;
    }

   // (WO) Forced float
   if (Options & FFLOAT)
    {
     if ((double)result_imval == 0.0)
      {
       #ifdef _float128_
       char fbuf[80];
       quadmath_snprintf (fbuf, (size_t)80, "%.*Qg", fprec, result_fval);
       bsize += sprintf (str + bsize, "%65.64s f\r\n", fbuf);
       #else
       bsize += sprintf (str + bsize, "%65.*Lg f\r\n", fprec, result_fval);
       #endif
       n++;
      }
     else
      {
       char imstr[80];
       char cphi[24];
       double re = (double)result_fval;
       double im = (double)result_imval;
       double phi = atan2 (im, re);
       double r   = hypot (re, im);
       dgr2str (cphi, phi);
       sprintf (imstr, "|%.8g|(%s) %.*g%+.*g%c", r, cphi,
                16, re, 16, im, c_imaginary);
       bsize += sprintf (str + bsize, "%65.64s f\r\n", imstr);
       n++;
      }
    }
   // (RO) Scientific (6.8k) format found
   if ((Options & SCI) || (fflags & ENG))
    {
     char scistr[80];
     if ((double)result_imval == 0.0)
      d2scistr (scistr, (double)result_fval);
     else
      {
       char cphi[24];
       char cr[24];
       char *cp  = scistr;
       double im = (double)result_imval;
       double re  = (double)result_fval;
       double phi = atan2 (im, re);
       double r = hypot (re, im);
       d2scistr (cr, r);
       dgr2str (cphi, phi);
       cp += sprintf (cp, "|%s|(%s) ", cr, cphi);
       normz (re, im);
       cp += d2scistr (cp, re);
       if (im >= 0.0) *cp++ = '+';

       cp += d2scistr (cp, im);
       *cp++ = c_imaginary;
       *cp   = '\0';
      }
     bsize += sprintf (str + bsize, "%65.64s S\r\n", scistr);
     n++;
    }
   // (UI) Normalized output
   if (Options & NRM)
    {
     char nrmstr[80];
     double imval = (double)result_imval;
     double fval  = (double)result_fval;
     normz (fval, imval);

     if (imval == 0)
      d2nrmstr (nrmstr, fval);
     else
      {
       char cphi[24];
       char cr[24];
       char *cp       = nrmstr;
       double imval = (double)result_imval;
       double fval  = (double)result_fval;
       double phi   = atan2 (imval, fval);
       double r     = hypot (fval, imval);
       d2nrmstr (cr, r);
       dgr2str (cphi, phi);
       cp += sprintf (cp, "|%s|(%s) ", cr, cphi);
       normz (fval, imval);
       cp += d2nrmstr (cp, fval);
       if (imval >= 0) *cp++ = '+';
       cp += d2nrmstr (cp, imval);
       *cp++ = c_imaginary;
       *cp   = '\0';
      }
     bsize += sprintf (str + bsize, "%65.64s n\r\n", nrmstr);
     n++;
    }

   // (RO) Computing format found
   if (((Options & CMP) || (fflags & CMP)) && ((double)result_imval == 0.0))
    {
     char bscistr[80];
     b2scistr (bscistr, (double)result_fval);
     bsize += sprintf (str + bsize, "%65.64s c\r\n", bscistr);
     n++;
    }

   // (UI) Integer output
   if ((Options & IGR) && (result_imval == 0) && is_integer (result_fval))
    {
       bsize += sprintf (str + bsize, "%65lld i\r\n", result_ival);
       n++;
    }

   // (UI) Factorization output
   if ((Options & FCTR) && (result_imval == 0) && is_integer (result_fval))
    {
       char fctrstr[80];
       factorize_p (fctrstr, (scfg & PAS), result_ival);
       bsize += sprintf (str + bsize, "%65.64s  \r\n", fctrstr);
       n++;
    }

   // (UI) Unsigned output
   if ((Options & UNS) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
       bsize += sprintf (str + bsize, "%65llu u\r\n", result_ival);//%llu|%zu
       n++;
    }

   // (UI) Fraction output
   if ((Options & FRC) && ((double)result_imval == 0.0) && (result_tag == tvFLOAT))
    {
     char frcstr[80];
     int num, denum;
     double val;
     if ((double)result_fval > 0.0)
      val = (double)result_fval;
     else
      val = -(double)result_fval;
     double intpart = floor (val);
     if (intpart < 1e15)
      {
       if (intpart > 0)
        {
         fraction (val - intpart, 0.001, num, denum);
         if ((double)result_fval > 0.0)
          sprintf (frcstr, "%.0f+%d/%d", intpart, num, denum);
         else
          sprintf (frcstr, "-%.0f-%d/%d", intpart, num, denum);
        }
       else
        {
         fraction (val, 0.001, num, denum);
         if ((double)result_fval > 0.0)
          sprintf (frcstr, "%d/%d", num, denum);
         else
          sprintf (frcstr, "-%d/%d", num, denum);
        }
       if (denum)
        {
         bsize += sprintf (str + bsize, "%65.64s F\r\n", frcstr);
         n++;
        }
      }
    }

   // (UI) Fraction inch output
   if ((Options & FRI) && ((double)result_imval == 0.0) && (result_tag == tvFLOAT))
    {
     char frcstr[80];
     int num, denum;
     double val;
     if ((double)result_fval > 0.0)
      val = (double)result_fval;
     else
      val = -(double)result_fval;
     val /= 25.4e-3;
     double intpart = floor (val);
     if (intpart < 1e15)
      {
       if (intpart > 0)
        {
         fraction (val - intpart, 0.001, num, denum);
         if (num && denum)
          {
           if ((double)result_fval > 0.0)
            sprintf (frcstr, "%.0f+%d/%d", intpart, num, denum);
           else
            sprintf (frcstr, "-%.0f-%d/%d", intpart, num, denum);
          }
         else
          {
           sprintf (frcstr, "%.0f", intpart);
          }
        }
       else
        {
         fraction (val, 0.001, num, denum);
         if ((double)result_fval > 0.0)
          sprintf (frcstr, "%d/%d", num, denum);
         else
          sprintf (frcstr, "-%d/%d", num, denum);
        }
       bsize += sprintf (str + bsize, "%65.64s \"\r\n", frcstr);
       n++;
      }
    }

   // (RO) Hex format found
   if (((Options & HEX) || (fflags & HEX)) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
     char binfstr[16];
     sprintf (binfstr, "%%64.%illxh  \r\n", binwide / 4);
      {
       bsize += sprintf (str + bsize, binfstr, result_ival);
       n++;
      }
    }

   // (RO) Octal format found
   if (((Options & OCT) || (fflags & OCT)) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
     char binfstr[16];
     sprintf (binfstr, "%%64.%illoo  \r\n", binwide / 3);
      {
       bsize += sprintf (str + bsize, binfstr, result_ival);
       n++;
      }
    }

   // (RO) Binary format found
   if (((Options & fBIN) || (fflags & fBIN)) && ((double)result_imval == 0.0)
       && is_integer (result_fval))
    {
     char binfstr[16];
     char binstr[80];
     sprintf (binfstr, "%%%ib", binwide);
     b2str (binstr, binfstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%64.64sb  \r\n", binstr);
       n++;
      }
    }

   // (RO) Char format found
   if (((Options & CHR) || (fflags & CHR)) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
     char chrstr[80];
     chr2str (chrstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%64.64s  c\r\n", chrstr);
       n++;
      }
    }

   // (RO) WChar format found
   if (((Options & WCH) || (fflags & WCH)) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
     char wchrstr[80];
     int i = result_ival & 0xffff;
     wchr2str (wchrstr, i);
      {
       bsize += sprintf (str + bsize, "%64.64s  c\r\n", wchrstr);
       n++;
      }
    }

   // (RO) Date time format found
   if (((Options & DAT) || (fflags & DAT)) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
     char dtstr[80];
     t2str (dtstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%65.64s \r\n", dtstr);
       n++;
      }
    }

   // (RO) Unix time
   if (((Options & UTM) || (fflags & UTM)) && ((double)result_imval == 0.0) && is_integer (result_fval))
    {
     char dtstr[80];
     nx_time2str (dtstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%65.64s  \r\n", dtstr);
       n++;
      }
    }

   // (RO) Degrees format found  * 180.0 / M_PI
   if (((Options & DEG) || (fflags & DEG)) && ((double)result_imval == 0.0) && (result_tag == tvFLOAT))
    {
     char dgrstr[80];
     char *cp = dgrstr;
     double angle = (double)result_fval;
     cp += sprintf (cp, "%.6g rad|", angle);
     cp += dgr2str (cp, angle);
     cp += sprintf (cp, " (%.6g`)", (double)(angle * 180.0 / (double)M_PId));
     cp += sprintf (cp, "|%.4g gon", (double)(angle * 200.0 / (double)M_PId));
     cp += sprintf (cp, "|%.4g turn", (double)(angle * 0.5 / (double)M_PId));

     bsize += sprintf (str + bsize, "%65.64s  \r\n", dgrstr);
     n++;
    }

   // (UI) Temperature format
   if (((Options & FRH) || (fflags & FRH)) && ((double)result_imval == 0.0) &&
       ((double)result_fval > -273.15) && (result_tag == tvFLOAT))
    {
     char frhstr[80];
     double temerature = (double)result_fval;
     sprintf (frhstr, "%.6g K|%.6g `C|%.6g `F", (double)(temerature + 273.15),
              temerature, (double)(temerature * 9.0 / 5.0 + 32.0));

     bsize += sprintf (str + bsize, "%65.64s  \r\n", frhstr);
     n++;
    }


   // (RO) String format found
   if (((Options & STR) || (fflags & STR)) && (result_imval == 0))
    {
      {
       if (sres[0])
        {
         char strcstr[80];
         sprintf (strcstr, "'%.64s'", sres);
         bsize += sprintf (str + bsize, "%65.64s S\r\n", strcstr);
         n++;
        }
       else
        {
         bsize += sprintf (str + bsize, "%65.64s S\r\n", "''");
         n++;
        }
      }
    }
  }
 if (size) *size = bsize;
 return n;
}
#pragma endregion
//---------------------------------------------------------------------------

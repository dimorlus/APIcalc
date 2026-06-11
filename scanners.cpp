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

#pragma region Data conversion functions
// Parse a hexadecimal string and convert it to an integer value, returning the number of characters
// parsed
int calculator::hscanf (char *str, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;
 while (c = *str++, c && (n < 16))
  {
   if ((c >= '0') && (c <= '9'))
    {
     res = res * 16 + (c - '0');
     n++;
    }
   else if ((c >= 'A') && (c <= 'F'))
    {
     res = res * 16 + (c - 'A') + 0xA;
     n++;
    }
   else if ((c >= 'a') && (c <= 'f'))
    {
     res = res * 16 + (c - 'a') + 0xa;
     n++;
    }
   else
    break;
  }
 ival = res;
 nn   = n;
 if (n)
  {
   fflags |= HEX;
  }
 return 0;
}

// Parse a binary string and convert it to an integer value, returning the number of characters
// parsed
int calculator::bscanf (char *str, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;

 while (c = *str++, c && (n < 64))
  {
   if ((c >= '0') && (c <= '1'))
    {
     res = res * 2 + (c - '0');
     n++;
    }
   else
    break;
  }
 ival = res;
 nn   = n;
 if (n)
  {
   fflags |= fBIN;
  }
 return 0;
}

// Parse an octal string and convert it to an integer value, returning the number of characters
// parsed
int calculator::oscanf (char *str, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;

 while (c = *str++, c && (n < 24))
  {
   if ((c >= '0') && (c <= '7'))
    {
     res = res * 8 + (c - '0');
     n++;
    }
   else
    break;
  }
 ival = res;
 nn   = n;
 if (n)
  {
   fflags |= OCT;
  }
 return 0;
}

// Parse a string that may be hexadecimal, octal, or an escape sequence, and convert it to an
// integer
int calculator::xscanf (char *str, int len, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;
 int hmax, omax;
 int max;

 switch (len)
  {
  case 1:
   max  = 0x100;
   hmax = 3;
   omax = 3;
   break;
  case 2:
   max  = 0x10000;
   hmax = 5;
   omax = 6;
   break;
  default:
   max  = 0;
   hmax = 0;
   omax = 0;
  }
 switch (*str)
  {
  case '0':
  case '1':
  case '2':
  case '3':
   {
    while (c = *str++, c && (n < omax))
     {
      if ((c >= '0') && (c <= '7'))
       {
        res = res * 8 + (c - '0');
        n++;
       }
      else
       break;
     }
    if (res >= max) n--;
    if (n)
     {
      fflags |= OCT;
     }
   }
   break;

  case 'x':
  case 'X':
   str++;
   n++;
   while (c = *str++, c && (res < max) && (n < hmax))
    {
     if ((c >= '0') && (c <= '9'))
      {
       res = res * 16 + (c - '0');
       n++;
      }
     else if ((c >= 'A') && (c <= 'F'))
      {
       res = res * 16 + (c - 'A') + 0xA;
       n++;
      }
     else if ((c >= 'a') && (c <= 'f'))
      {
       res = res * 16 + (c - 'a') + 0xa;
       n++;
      }
     else
      break;
    }
   if (res >= max) n--;
   if (n)
    {
     fflags |= HEX;
    }
   break;

  case 'a':
   res = '\007';
   n   = 1;
   fflags |= ESC;
   break;

  case 'f':
   res = 255u;
   n   = 1;
   fflags |= ESC;
   break;

  case 'v':
   res = '\x0b';
   n   = 1;
   fflags |= ESC;
   break;

  case 'E':
  case 'e':
   res = '\033';
   n   = 1;
   fflags |= ESC;
   break;

  case 't':
   res = '\t';
   n   = 1;
   fflags |= ESC;
   break;

  case 'n':
   res = '\n';
   n   = 1;
   fflags |= ESC;
   break;

  case 'r':
   res = '\r';
   n   = 1;
   fflags |= ESC;
   break;

  case 'b':
   res = '\b';
   n   = 1;
   fflags |= ESC;
   break;

  case '\\':
   res = '\\';
   n   = 1;
   fflags |= ESC;
   break;
  }
 ival = res;
 nn   = n;
 return 0;
}

// Parse a string that may contain degrees, minutes, and seconds, and convert it to radians
double calculator::dstrtod (char *s, char **endptr)
{
 const char cdeg[]   = { '`', '\'', '\"' }; //` - degrees, ' - minutes, " - seconds
 const double mdeg[] = { M_PId / 180.0, M_PId / (180.0 * 60), M_PId / (180.0 * 60 * 60) };
 double res          = 0;
 double d;
 char *end = s;

 for (int i = 0; i < 3; i++)
  {
   d = strtod (end, &end);
   do
    {
     if (*end == cdeg[i])
      {
       res += d * mdeg[i];
       end++;
       fflags |= DEG;
       break;
      }
     else
      i++;
    }
   while (i < 3);
  }
 *endptr = end;
 return res;
}

// Parse a string that may contain centuries, years, weeks, days, hours, minutes, and seconds, and
// convert it to seconds
// 1:c1:y1:d1:h1:m1:s  => 189377247661s
double calculator::tstrtod (char *s, char **endptr)
{
 const double dms[] = { (60.0L * 60.0 * 60.0 * 24.0 * 365.25 * 100.0),
                        (60.0L * 60.0 * 24.0 * 365.25),
                        (60.0L * 60.0 * 24.0 * 7),
                        (60.0L * 60.0 * 24.0),
                        (60.0L * 60.0),
                        60.0L,
                        1.0L };
 const char cdt[]   = { 'c', 'y', 'w', 'd', 'h', 'm', 's' };
 double res         = 0;
 double d;
 char *end = s;

 for (int i = 0; i < 6; i++)
  {
   d = strtod (end, &end);
   do
    {
     if ((*end == ':') && (*(end + 1) == cdt[i]))
      {
       res += d * dms[i];
       end += 2;
       fflags |= DAT;
       break;
      }
     else
      i++;
    }
   while (i < 6);
  }
 *endptr = end;
 return res;
}

// Parse a string that may contain an engineering suffix (k, M, G, etc.) and apply the appropriate
// https://en.wikipedia.org/wiki/Metric_prefix
// process expression like 1k56 => 1.56k (maximum 3 digits)
void calculator::engineering (double mul, char *&fpos, double &fval)
{
 int fract = 0;
 int div   = 1;
 int n     = 3; // maximum 3 digits
 while (*fpos && (*fpos >= '0') && ((*fpos <= '9')) && n--)
  {
   div *= 10;
   fract *= 10;
   fract += *fpos++ - '0';
  }
 fval *= mul;
 fval += (fract * mul) / div;
 fflags |= ENG;
}

// Check if the next characters are "B" or "iB" for computing format, and set the CMP flag if found
bool calculator::isCMP (char *&fpos)
{
 if (*fpos == 'B')
  {
   fpos++;
   fflags |= CMP;
   return true;
  }
 else if ((*fpos == 'i') && (*(fpos + 1) == 'B'))
  {
   fpos += 2;
   fflags |= CMP;
   return true;
  }
 return false;
}

// Parse a string that may contain a scientific suffix (k, M, G, etc.) and apply the appropriate
void calculator::scientific (char *&fpos, double &fval)
{
 if (*(fpos - 1) == 'E') fpos--;
 switch (*fpos)
  {
  case '\"': // Inch
   if (scfg & FRI)
    {
     fpos++;
     // fval *= 25.4e-3;
     engineering (25.4e-3, fpos, fval);
     fflags |= FRI;
    }
   break;
  case 'Q':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.267650600228229401496703205376e+30L; // 2**100
   else
    engineering (1e30, fpos, fval);
   break;
  case 'R':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.237940039285380274899124224e+27L; // 2**90
   else
    engineering (1e27, fpos, fval);
   break;
  case 'Y':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.208925819614629174706176e+24L; // 2**80
   else
    engineering (1e24, fpos, fval);
   break;
  case 'Z':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.180591620717411303424e+21L; // 2**70
   else
    engineering (1e21, fpos, fval);
   break;
  case 'E':
   fpos++;
   if (isCMP (fpos))
    fval *= 1152921504606846976ull; // 2**60
   else
    engineering (1e18, fpos, fval);
   break;
  case 'P':
   fpos++;
   if (isCMP (fpos))
    fval *= 1125899906842624ull; // 2**50
   else
    engineering (1e15, fpos, fval);
   break;
  case 'T':
   fpos++;
   if (isCMP (fpos))
    fval *= 1099511627776ull; // 2**40
   else
    engineering (1e12, fpos, fval);
   break;
  case 'G':
   fpos++;
   if (isCMP (fpos))
    fval *= 1073741824ull; // 2**30
   else
    engineering (1e9, fpos, fval);
   break;
  case 'M':
   fpos++;
   if (isCMP (fpos))
    fval *= 1048576; // 2**20
   else
    engineering (1e6, fpos, fval);
   break;
  case 'K':
#ifdef _KELVIN_
   fpos++;
   if (isCMP (fpos))
    fval *= 1024; // 2**10
   else if ((scfg & FRH) == 0)
    engineering (1e3, fpos, fval);
   else
    fpos--;
#else
   fpos++;
   if (isCMP (fpos))
    fval *= 1024; // 2**10
   else
    engineering (1e3, fpos, fval);
#endif //_KELVIN_
   break;
  // case 'R':
  //  fpos++;
  //  engineering (1, fpos, fval);
  //  break;
  case 'h':
   fpos++;
   engineering (1e2, fpos, fval);
   break;
  case 'k':
   fpos++;
   engineering (1e3, fpos, fval);
   break;
  case 'D':
   fpos++;
   engineering (1e1, fpos, fval);
   break;
  case 'd':
   fpos++;
   if (*fpos == 'a')
    {
     fpos++;
     engineering (1e1, fpos, fval);
    }
   else
    engineering (1e-1, fpos, fval);
   break;
  case 'c':
   fpos++;
   engineering (1e-2, fpos, fval);
   break;
  case 'm':
   fpos++;
   engineering (1e-3, fpos, fval);
   break;
  case 'u':
   fpos++;
   engineering (1e-6, fpos, fval);
   break;
  case 'n':
   fpos++;
   engineering (1e-9, fpos, fval);
   break;
  case 'p':
   fpos++;
   engineering (1e-12, fpos, fval);
   break;
  case 'f':
   fpos++;
   engineering (1e-15, fpos, fval);
   break;
  case 'a':
   fpos++;
   engineering (1e-18, fpos, fval);
   break;
  case 'z':
   fpos++;
   engineering (1e-21, fpos, fval);
   break;
  case 'y':
   fpos++;
   engineering (1e-24, fpos, fval);
   break;
  case 'r':
   fpos++;
   engineering (1e-27, fpos, fval);
   break;
  case 'q':
   fpos++;
   engineering (1e-30, fpos, fval);
   break;
  }
}

#pragma endregion
//---------------------------------------------------------------------------
#pragma region Scan expression

// M[row, col] matrix element access
int calculator::mx_idx (int &row, int &col)
{
 int rows   = 0;
 int cols   = 0;
 int idx[2] = { 0, 0 };
 int ii     = 0;
 char *ipos = buf + pos;
 while (isspace (*ipos & 0x7f)) ipos++;
 // one child calculator for all elements — new names stay local to matrix
 calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return 0;
  }
 while (*ipos && *ipos != ']')
  {
   // collect element expression respecting parenthesis depth
   char ebuf[STRBUF];
   int eidx  = 0;
   int depth = 0;
   while (*ipos && *ipos != ']')
    {
     if (*ipos == '(')
      depth++;
     else if (*ipos == ')' && depth > 0)
      depth--;
     else if ((*ipos == ',' || *ipos == ']') && depth == 0)
      break;
     if (eidx < STRBUF - 1) ebuf[eidx++] = *ipos++;
    }
   ebuf[eidx] = '\0';
   // if (*ipos == ']') break;
   float__t res = child->evaluate_f (ebuf);
   if (isnan (res) || child->error ()[0])
    {
     error (child->error ());
     delete child;
     return 0;
    }
   if (!(child->result_tag == tvFLOAT || child->result_tag == tvINT))
    {
     error ("Matrix index must be scalar");
     delete child;
     return 0;
    }

   if (child->result_imval != 0.0L)
    {
     error ("Complex matrix indices not supported");
     delete child;
     return 0;
    }
   if (ii < 2)
    {
     idx[ii++] = (int)child->result_fval;
     if (idx[ii - 1] < 0)
      {
       error ("Matrix indices must be positive integers");
       delete child;
       return 0;
      }
    }
   else
    {
     error ("Too many indices for matrix access");
     delete child;
     return 0;
    }
   while (isspace (*ipos & 0x7f)) ipos++;
   if (*ipos == ',') ipos++;
   while (isspace (*ipos & 0x7f)) ipos++;
  }
 fflags |= child->isfflags ();
 delete child; // done with child calculator

 if (*ipos != ']')
  {
   error ("Expected ']'");
   return 0;
  }
 row = idx[0];
 col = idx[1];
 pos = ipos - buf + 1;
 return ii;
}

//[(a11,a12,...);(a21,a22,...);...]
// Matrix parser for calculator
// Called with pos pointing to char after '['
// Format: [(1, 2, 3);(4, 5, 6);(7, 8, 9)]
// Returns toOPERAND with v_stack[v_sp].mval pointing to the matrix  if successful,
// or toERROR if there is a syntax error.
t_operator calculator::sqbraces (void)
{
 char *ipos = buf + pos;
 while (isspace (*ipos & 0x7f)) ipos++;

 float__t tmp[MAX_R * MAX_C];
 int rows    = 0;
 int cols    = 0;
 int curCols = 0;

 // one child calculator for all elements — new names stay local to matrix
 calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return toERROR;
  }

 while (*ipos && *ipos != ']')
  {
   if (*ipos != '(')
    {
     error ("Expected '('");
     delete child;
     return toERROR;
    }
   ipos++;
   rows++;
   if (rows > MAX_R)
    {
     error ("Too many rows");
     delete child;
     return toERROR;
    }
   curCols = 0;
   while (isspace (*ipos & 0x7f)) ipos++;

   while (*ipos && *ipos != ')' && *ipos != ']')
    {
     // collect element expression respecting parenthesis depth
     char ebuf[STRBUF];
     int eidx  = 0;
     int depth = 0;
     while (*ipos)
      {
       if (*ipos == '(')
        depth++;
       else if (*ipos == ')' && depth > 0)
        depth--;
       else if ((*ipos == ',' || *ipos == ')') && depth == 0)
        break;
       if (eidx < STRBUF - 1) ebuf[eidx++] = *ipos++;
      }
     ebuf[eidx] = '\0';
     if (eidx == 0)
      {
       error ("Empty matrix element");
       delete child;
       return toERROR;
      }

     float__t res = child->evaluate_f (ebuf);
     if (isnan (res) || child->error ()[0])
      {
       error (child->error ());
       delete child;
       return toERROR;
      }

     if (!(child->result_tag == tvFLOAT || child->result_tag == tvINT))
      {
       error ("Matrix element must be scalar");
       delete child;
       return toERROR;
      }

     if (child->result_imval != 0.0L)
      {
       error ("Complex matrix elements not supported");
       delete child;
       return toERROR;
      }

     curCols++;
     if (curCols > MAX_C)
      {
       error ("Too many columns");
       delete child;
       return toERROR;
      }
     tmp[(rows - 1) * MAX_C + (curCols - 1)] = child->result_fval;

     while (isspace (*ipos & 0x7f)) ipos++;
     if (*ipos == ',') ipos++;
     while (isspace (*ipos & 0x7f)) ipos++;
    }

   if (*ipos != ')')
    {
     error ("Expected ')'");
     delete child;
     return toERROR;
    }
   ipos++;

   if (rows == 1)
    cols = curCols;
   else if (curCols != cols)
    {
     error ("Inconsistent column count");
     delete child;
     return toERROR;
    }

   while (isspace (*ipos & 0x7f)) ipos++;
   if (*ipos == ';')
    {
     ipos++;
     while (isspace (*ipos & 0x7f)) ipos++;
    }
   else if (*ipos != ']' && *ipos != '\0')
    {
     error ("Expected ';' or ']'");
     delete child;
     return toERROR;
    }
  }

 fflags |= child->isfflags ();
 delete child; // done with child calculator

 if (*ipos != ']')
  {
   error ("Expected ']'");
   return toERROR;
  }
 if (rows == 0 || cols == 0)
  {
   error ("Empty matrix");
   return toERROR;
  }

 float__t *mval = (float__t *)sf_alloc (rows * cols * sizeof (float__t), ptMALLOC);

 if (!mval)
  {
   error ("Memory allocation failed");
   return toERROR;
  }

 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++) mval[r * cols + c] = tmp[r * MAX_C + c];

 pos                 = ipos - buf + 1;
 v_stack[v_sp].sval  = nullptr;
 v_stack[v_sp].var   = nullptr;
 v_stack[v_sp].pos   = pos;
 v_stack[v_sp].fval  = qnan;
 v_stack[v_sp].imval = ((float__t)0.0);
 v_stack[v_sp].ival  = 0;
 v_stack[v_sp].mrows = rows;
 v_stack[v_sp].mcols = cols;
 v_stack[v_sp].mval  = mval;
 v_stack[v_sp].tag   = tvMATRIX;
 v_sp++;
 return toOPERAND;
}

// User function definition syntax: {frq(L, C)1/(2 pi sqrt(L C))}
// 1. Find the expression in {}
// 2. Find the function name in it before (..) -> frq
// 3. Place the name (frq) in the list of names (symbols), and replace the function
//   pointer with a string with parameters and body (L, C)1/(2 pi sqrt(L C)).
//   If such a name already exists, but not for user defined function, return toERROR.
//   If such a name already exists for user defined function, return the existing one.
//   its done in addUF function, which is called from here. addUF returns nullptr if there is a
//   name conflict, and the new symbol if added successfully or already exists as a user
//   function.
// 4. Place the new type tsUFUNC in the list of names (by addUF function)
// 5. Return the new type toCONTINUE to continue scanning the expression.
t_operator calculator::braces (void) //{...}
{
 char sbuf[STRBUF];
 int sidx   = 0;
 char *ipos = buf + pos;
 while (*ipos && (*ipos != '}') && (sidx < STRBUF - 1)) sbuf[sidx++] = *ipos++;
 sbuf[sidx] = '\0';
 if (*ipos == '}')
  {
   // extract user function name here and put it as symbol in the hash table
   char fname[STRBUF];
   char *fnp;
   fnp      = fname;
   int spos = 0;
   while (isalnum (sbuf[spos] & 0x7f) || sbuf[spos] == '_')
    {
     *fnp++ = sbuf[spos++] & 0x7f;
    }
   if (fnp == fname)
    {
     error ("Bad character");
     return toERROR;
    }
   *fnp = '\0';

   if (fname[0])
    {
     // Add user function to symbol table
     if (!addUF (fname, &sbuf[spos]))
      {
       //error ("Duplicate name");
       return toERROR;
      }
    }
   else
    {
     error ("User function name missing");
     return toERROR;
    }
  }
 else
  {
   error ("unmatched brace");
   return toERROR;
  }
 pos = ipos - buf + 1;
#ifdef _UF_AS_OPERAND_
 // if used this way, expression in {} is treated as a 0 and {expr};expr syntax is supported for
 // user functions
 v_stack[v_sp].tag   = tvINT;
 v_stack[v_sp].ival  = 0;
 v_stack[v_sp].pos   = pos;
 v_stack[v_sp++].var = nullptr;
 return toOPERAND;
#else  //_UF_AS_OPERAND_
 // if used this way, expression in {} is treated as a empty and {expr}expr syntax is supported
 // for user functions
 return toCONTINUE;
#endif //_UF_AS_OPERAND_
}

// "...." or '....'
// Supported escape sequences:
//   \n  -> newline
//   \r  -> carriage return
//   \t  -> tab
//   \\  -> backslash
//   \"  -> double quote (alternative to "" inside ""-quoted strings)
//   \'  -> single quote (alternative to '' inside ''-quoted strings)
// Doubled quote character (same as opening quote) also represents a single quote:
//   "Hello! ""World"""  -> Hello! "World"
//   'It''s fine'        -> It's fine
t_operator calculator::dqscan (char qc)
{
 char *ipos;
 char sbuf[STRBUF];
 int sidx = 0;
 ipos     = buf + pos;

 while (*ipos && (sidx < STRBUF - 1))
  {
   if (*ipos < ' ') // skip control characters
    {
     ipos++;
     continue;
    }
   if (*ipos == '\\')
    {
     // escape sequence
     ipos++;
     if (!*ipos) break; // unexpected end of string
     switch (*ipos)
      {
#ifdef _CRLF_
      case 'n':
       sbuf[sidx++] = '\n';
       break;
      case 'r':
       sbuf[sidx++] = '\r';
       break;
      case 't':
       sbuf[sidx++] = '\t';
       break;
#endif //_CRLF_
      case '\\':
       sbuf[sidx++] = '\\';
       break;
      case '"':
       sbuf[sidx++] = '"';
       break;
      case '\'':
       sbuf[sidx++] = '\'';
       break;
      default: // unknown escape - keep as-is (e.g. \x -> \x)
       if (sidx < STRBUF - 2) sbuf[sidx++] = '\\';
       sbuf[sidx++] = *ipos;
       break;
      }
     ipos++;
    }
   else if (*ipos == qc)
    {
     // check for doubled quote: "" or ''
     if (*(ipos + 1) == qc)
      {
       sbuf[sidx++] = qc; // one quote character in result
       ipos += 2;         // skip both
      }
     else
      break; // end of string literal
    }
   else
    {
     sbuf[sidx++] = *ipos++;
    }
  }

 sbuf[sidx] = '\0';

 if (*ipos == qc)
  {
   v_stack[v_sp].tag   = tvSTR;
   v_stack[v_sp].ival  = 0;
   v_stack[v_sp].sval  = dupString (sbuf);
   pos                 = ipos - buf + 1;
   v_stack[v_sp].pos   = pos;
   v_stack[v_sp++].var = nullptr;
   return toOPERAND;
  }
 else
  {
   error ("unterminated string literal");
   return toERROR;
  }
}

void calculator::isNRM (char *start, char *end)
{
 int ppos = 0;
 int epos = 0;
 int len  = end - start;
 // Check if the number is in normal (non-scientific) format
 // by looking for 'e' or 'E' in the part of the string that was parsed as a number
 for (char *p = start; p < end; p++)
  {
   if (*p == '.') ppos = p - start + 1;              // position of decimal point, if any
   if (*p == 'e' || *p == 'E') epos = p - start + 1; // position of exponent, if any
  }

 if (ppos == 0 && epos == 0 && len <= 4) // 1234
  {
   fflags |= NRM;
   return;
  }
 if (ppos && epos == 0 && len <= 5) // 123.4
  {
   fflags |= NRM;
   return;
  }
 if (ppos && epos > 0 && epos <= 6) // 123.4E3
  {
   fflags |= NRM;
   return;
  }
 if (ppos == 0 && epos > 0 && epos <= 4) // 123E3
  {
   fflags |= NRM;
   return;
  }
}

// Scan a number in various formats: decimal, hex (0x or $), octal (0o), binary (0b), or with
// backslash for base prefix
t_operator calculator::dscan (bool operand, bool percent)
{
 uint32_t info = 0;
 int_t ival    = 0;
 double fval   = 0;
 double sfval  = 0;
 int ierr      = 0, ferr;
 char *ipos, *fpos, *sfpos;
 int n = 0;

 if (buf[pos - 1] == '\\')
  {
   ierr = xscanf (buf + pos, 1, ival, n);
   ipos = buf + pos + n;
   if (n) info |= ESC;
   fflags |= ESC;
  }
 else if ((buf[pos - 1] == '0') && ((buf[pos] == 'B') || (buf[pos] == 'b')))
  {
   ierr = bscanf (buf + pos + 1, ival, n);
   ipos = buf + pos + n + 1;
   if (n) info |= fBIN;
   fflags |= fBIN;
  }
 else if ((buf[pos - 1] == '0') && ((buf[pos] == 'O') || (buf[pos] == 'o')))
  {
   ierr = oscanf (buf + pos + 1, ival, n);
   ipos = buf + pos + n + 1;
   if (n) info |= OCT;
   fflags |= OCT;
  }
 else if (buf[pos - 1] == '$')
  {
   ierr = hscanf (buf + pos, ival, n);
   ipos = buf + pos + n;
   if (n) info |= HEX;
   fflags |= HEX;
  }
 else if ((buf[pos - 1] == '0') && ((buf[pos] == 'X') || (buf[pos] == 'x')))
  {
   ierr = hscanf (buf + pos + 1, ival, n);
   ipos = buf + pos + n + 1;
   if (n) info |= HEX;
   fflags |= HEX;
  }
 else
  {
   errno = 0;
#ifdef __BORLANDC__
   ival = strtol (buf + pos - 1, &ipos, 10);
#else
   ival = strtoll (buf + pos - 1, &ipos, 10);
#endif
   ierr = errno;
  }
 errno = 0;

 sfval = fval = strtod (buf + pos - 1, &fpos);

 if (errno == 0 && (fpos > ipos)) isNRM (buf + pos - 1, fpos); //

 sfpos = fpos;

 v_stack[v_sp].tag = tvFLOAT;

 //` - degrees, ' - minutes, " - seconds
 if ((*fpos == '\'') || (*fpos == '`') || (((scfg & FRI) == 0) && (*fpos == '\"')))
  {
   fval = dstrtod (buf + pos - 1, &fpos);
   if ((fval != qnan) && (fpos > sfpos)) info |= DEG;
  }
 else if (*fpos == ':')
  {
   fval = tstrtod (buf + pos - 1, &fpos);
   if ((fval != qnan) && (fpos > sfpos)) info |= DAT;
  }
 else if (scfg & (ENG | SCI | FRI))
  {
   scientific (fpos, fval);
   if (fpos > sfpos) info |= ENG;
  }
 if ((scfg & FRH) && (*fpos == 'F')) // Fahrenheit to Celsius
  {
   fpos++;
   if ((o_sp > 0) && (o_stack[o_sp - 1] == toMINUS))
    fval = -(-fval - 32.0) * 5.0 / 9.0;
   else
    fval = (fval - 32.0) * 5.0 / 9.0;
   fflags |= FRH;
   info |= FRH;
  }
#ifdef _KELVIN_
 if ((scfg & FRH) && (*fpos == 'K')) // Kelvin to Celsius
  {
   fpos++;
   if ((o_sp > 0) && (o_stack[o_sp - 1] == toMINUS)) // fval = qnan;
    {
     error ("Temperature below absolute zero");
     return toERROR;
    }
   else
    fval = fval - 273.15;
   fflags |= FRH;
  }
#endif //_KELVIN_
 if (operand && percent && (*fpos == '%'))
  {
   fpos++;
   v_stack[v_sp].tag = tvPERCENT;
  }
 if ((*fpos == 'i') || (*fpos == 'j'))
  {
   c_imaginary = *fpos;
   fpos++;
   fflags |= CPX;
   v_stack[v_sp].tag = tvCOMPLEX;
  }
 if (*fpos && (isalnum (*fpos & 0x7f) || *fpos == '@' || *fpos == '_' || *fpos == '?'))
  { // Rollback to float if followed by identifier (e.g. 1k => 1.0k, but 1kB => 1k * B)
   fpos              = sfpos;
   fval              = sfval;
   v_stack[v_sp].tag = tvFLOAT;
  }

 if (v_stack[v_sp].tag == tvCOMPLEX)
  {
   v_stack[v_sp].imval = (float__t)fval;
   v_stack[v_sp].fval  = (float__t)0.0L;
  }
 else
  {
   v_stack[v_sp].fval  = (float__t)fval;
   v_stack[v_sp].imval = (float__t)0.0L;
  }
 pos = fpos - buf;

 if (v_stack[v_sp].tag == tvFLOAT)
  {
   ferr = errno;
   if ((ipos <= fpos) && ((*fpos == '.') || (*fpos == '$') || (*fpos == '\\')))
    {
     pos = fpos - buf + 1;
     error ("bad numeric constant");
     return toERROR;
    }
   if (ierr && ferr)
    {
     error ("bad numeric constant");
     return toERROR;
    }
   if (v_sp == max_stack_size)
    {
     error ("stack overflow");
     return toERROR;
    }
   if (!ierr && ipos >= fpos && (*fpos != 'i') && (*fpos != 'j') && (*fpos != '%'))
    {
     if (scfg & FFLOAT)
      v_stack[v_sp].tag = tvFLOAT;
     else
      v_stack[v_sp].tag = tvINT;
     v_stack[v_sp].ival = ival;
     v_stack[v_sp].fval = (float__t)ival;
     pos                = ipos - buf;
    }
  }
 v_stack[v_sp].info = info;
 v_stack[v_sp].pos  = pos;
 if (v_stack[v_sp].tag == tvFLOAT) fflags |= FLT;
 v_stack[v_sp++].var = nullptr;
 return toOPERAND;
}

// solve (x(2x+2)-2,x:=0)
// calc (x(2x+2)-2,x:=0)
// integr (x(2x+2)-2,0,10,x)
// diff (x(2x+2)-2, 0, x)
// for(expr, from, to, var)
// sum(expr, from, to, var)
// plot(fname, expr, from, to, var)
// extract expression in () after the function name, and put it as string in the symbol table,
// put variable with tvSOLVE tag and 'x(2x+2)-2,x:=0' in sval to variable stack
// and return toOPERAND or toERROR if something wrong.
t_operator calculator::sscan (symbol *sym)
{
 char sbuf[STRBUF];
 int sidx              = 0;
 int comma_count       = 0;
 int parenthesis_count = 1; // we start after the opening parenthesis, so we are already at depth 1

 char *ipos = buf + pos;
 if (*ipos == ')')
  {
   pos++;
   return toRPAR;
  }
 else if (*ipos == '\0')
  return toEND; // end of input

 // skip whitespace befor '('
 while (isspace (*ipos & 0x7f)) ipos++;

 while (*ipos && (sidx < STRBUF - 1) && (parenthesis_count > 0))
  {
   if (*ipos == ',' && parenthesis_count == 1)
    comma_count++; // count commas only at the top level of parentheses
   else if (*ipos == '(' || *ipos == '[')
    parenthesis_count++; // increase depth for nested parentheses and brackets
   else if (*ipos == ')' || *ipos == ']')
    parenthesis_count--; // decrease depth for closing parentheses and brackets
   sbuf[sidx++] = *ipos++;
  }
 if (sidx && sbuf[sidx - 1] == ')')
  sbuf[sidx - 1] = '\0';  // remove the closing parenthesis from the string
 sbuf[STRBUF - 1] = '\0'; // null-terminate the string in case of overflow

 if (sym)
  {
   blockflag |= sym->block;
   if ((sym->tag == tsSOLVE) // solve (x(2x+2)-2,x:=0)
    || (sym->tag == tsCALC)) // calc (x(2x+2)-2,x:=0)
    {
     if (parenthesis_count == 0 && comma_count == 1)
      {
       v_stack[v_sp].tag = tvSOLVE;
      }
     else if (parenthesis_count == 0 && comma_count == 2)
      {
       v_stack[v_sp].tag  = tvSOLVEA; // Alternative form for calc/solve solve(x(2x+2)-2, 0, x)
      }
     else
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in solve expression");
       else
        error ("wrong number of arguments in solve expression");
       return toERROR;
      }
    }
   else if (sym->tag == tsINTEGR || // integr (x(2x+2)-2,0,10,x)
            sym->tag == tsSUM)      // sum (x(2x+2)-2,0,10,x)
    {
     if (parenthesis_count == 0 && comma_count == 3)
      {
       v_stack[v_sp].tag = tvINTEGR;
      }
     else
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in integral expression");
       else
        error ("wrong number of arguments in integral expression");
       return toERROR;
      }
    }
   else if (sym->tag == tsDIFF) // diff (x(2x+2)-2, 0, x)
    {
     if (parenthesis_count == 0 && comma_count == 2)
      {
       v_stack[v_sp].tag = tvDIFF;
      }
     else
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in diff");
       else
        error ("wrong number of arguments in diff");
       return toERROR;
      }
    }
   else if (sym->tag == tsEXTR) // extr (x(2x+2)-2, 0, x)
    {
     if (parenthesis_count == 0 && comma_count == 2)
      {
       v_stack[v_sp].tag = tvEXTR;
      }
     else
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in extr");
       else
        error ("wrong number of arguments in extr");
       return toERROR;
      }
    }
   else if (sym->tag == tsINVERSE) // inverse (x(2x+2)-2, 0, x)
    {
     if (parenthesis_count == 0 && comma_count == 2)
      {
       v_stack[v_sp].tag = tvINVERSE;
      }
     else
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in inverse");
       else
        error ("wrong number of arguments in inverse");
       return toERROR;
      }
    }
   else if (sym->tag == tsPLOT) // plot(fname, expr)
    {
     bool error_in_args = false;
     switch (sym->fidx)
      {
      case pl_plot:    // plot(expr, from, to, var) or plot(expr, var)
      case pl_plotpol: // plotpol(expr, from, to, var)
      case pl_plotlgx:
      case pl_plotlgy:
      case pl_plotlgxy:
      case pl_plotsmith: // plotsmith(expr, from, to, var)
       if (parenthesis_count == 0 && (comma_count == 3||comma_count == 1))
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      case pl_xyplot: // xyplot(xexpr, yexpr, from, to, var) or xyplot(xexpr, yexpr, var)
       if (parenthesis_count == 0 && (comma_count == 4 || comma_count == 2))
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      case pl_plotsmithz: // plotsmithz(expr, from, to, var, Z0)
       if (parenthesis_count == 0 && comma_count == 4)
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      case pl_plotdata:  // plotdata(datafile, mask)
      case pl_plotdatal: // plotdatal(datafile, mask) - with lines
       if (parenthesis_count == 0 && comma_count <= 1)
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      default:
       error ("Unknown plot function");
       return toERROR;
      }
     if (error_in_args)
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in plot expression");
       else
        error ("wrong number of arguments in plot expression");
       return toERROR;
      }
    }
   else if (sym->tag == tsFOR) // for(expr, from, to, var)
    {
     if (parenthesis_count == 0 && comma_count == 3)
      {
       v_stack[v_sp].tag = tvFOR;
      }
     else
      {
       if (parenthesis_count)
        error ("unmatched parenthesis in for expression");
       else
        error ("wrong number of arguments in for expression");
       return toERROR;
      }
    }

   {
    char *sval = dupString (sbuf); // dup and register the string in the string table
    if (!sval)
     {
      error ("memory allocation failed");
      return toERROR;
     }

    pos                  = ipos - buf - 1;
    v_stack[v_sp].sval   = sval;
    v_stack[v_sp].var    = sym;
    v_stack[v_sp].pos    = pos;
    v_stack[v_sp].fval   = qnan;
    v_stack[v_sp].imval  = ((float__t)0.0);
    v_stack[v_sp++].ival = 0;
    return toOPERAND;
   }
  }
 return toERROR;
}

// parse the next operator from the input buffer, returning the operator type
t_operator calculator::scan (bool operand, bool percent)
{
 char name[max_expression_length], *np;

 while (isspace (buf[pos] & 0x7f)) pos += 1; // skip whitespace
 switch (buf[pos++])
  {
  case '\0':
   return toEND; // end of input
  case '(':
   return toLPAR;
  case ')':
   return toRPAR;
  case '+':
   if (buf[pos] == '+') // (RO) ++ operator
    {
     pos += 1;
     return operand ? toPREINC : toPOSTINC;
    }
   else if (buf[pos] == '=') // (RO) += operator
    {
     pos += 1;
     return toSETADD;
    }
   return operand ? toPLUS : toADD;
  case '-':
   if (buf[pos] == '-') // (RO) -- operator
    {
     pos += 1;
     return operand ? toPREDEC : toPOSTDEC;
    }
   else if (buf[pos] == '=') // (RO) -= operator
    {
     pos += 1;
     return toSETSUB;
    }
   return operand ? toMINUS : toSUB;
  case '!':
   if (buf[pos] == '=') // (RO) != operator
    {
     pos += 1;
     return toNE;
    }
   return operand ? toNOT : toFACT;
  case '~':
   return toCOM;
  case ';':
   if (buf[pos] == ';') // (RO) ;; operator (comment to end of line)
    {
     pos += 1;
     scan_opt (&buf[pos], fflags);
     return toEND;
    }
   return toSEMI;
  case '*':
   if (buf[pos] == '*') // (RO) ** or **= operator
    {
     if (buf[pos + 1] == '=') // (RO) **= operator
      {
       pos += 2;
       return toSETPOW;
      }
     pos += 1;
     return toPOW;
    }
   else if (buf[pos] == '=') // (RO) *= operator
    {
     pos += 1;
     return toSETMUL;
    }
   return toMUL;
  case '/':
   if (buf[pos] == '=') // (RO) /= operator
    {
     pos += 1;
     return toSETDIV;
    }
   else if (buf[pos] == '/') // (RO) // operator (parallel resistors)
    {
     pos += 1;
     return toPAR;
    }
   return toDIV;
  case '%':
   if (buf[pos] == '=') // (RO) %= operator
    {
     pos += 1;
     return toSETMOD;
    }
   else if (buf[pos] == '%') // (RO) %% operator (percent)
    {
     pos += 1;
     return toPERCENT;
    }
   return toMOD;
  case '<':
   if (buf[pos] == '<') // (RO) << or <<= operator
    {
     if (buf[pos + 1] == '=') // (RO) <<= operator
      {
       pos += 2;
       return toSETASL;
      }
     else // (RO) << operator
      {
       pos += 1;
       return toASL;
      }
    }
   else if (buf[pos] == '=') // (RO) <= operator
    {
     pos += 1;
     return toLE;
    }
   else if (buf[pos] == '>') // (RO) <> operator (not equal)
    {
     pos += 1;
     return toNE;
    }
   return toLT;
  case '>':
   if (buf[pos] == '>') // (RO) >> or >>= operator
    {
     if (buf[pos + 1] == '>') // (RO) >>> or >>>= operator
      {
       if (buf[pos + 2] == '=') // (RO) >>>= operator
        {
         pos += 3;
         return toSETLSR;
        }
       pos += 2;
       return toLSR;
      }
     else if (buf[pos + 1] == '=') // (RO) >>= operator
      {
       pos += 2;
       return toSETASR;
      }
     else // (RO) >> operator
      {
       pos += 1;
       return toASR;
      }
    }
   else if (buf[pos] == '=') // (RO) >= operator
    {
     pos += 1;
     return toGE;
    }
   return toGT;
  case '=':
   if (buf[pos] == '=') // (RO) == operator
    {
     scfg &= ~PAS;
     fflags &= ~PAS;
     pos += 1;
     return toEQ;
    }
   if (scfg & PAS)
    return toEQ;
   else
    return toSET;
  case ':':
   if (buf[pos] == '=') // (RO) := operator
    {
     scfg |= PAS;
     fflags |= PAS;
     pos += 1;
     return toSET;
    }
   error ("syntax error");
   return toERROR;
  case '&':
   if (buf[pos] == '&') // (RO) && operator
    {
     pos += 1;
     return toAND;
    }
   else if (buf[pos] == '=') // (RO) &= operator
    {
     pos += 1;
     return toSETAND;
    }
   return toAND;
  case '|':
   if (buf[pos] == '|') // (RO) || operator
    {
     pos += 1;
     return toOR;
    }
   else if (buf[pos] == '=') // (RO) |= operator
    {
     pos += 1;
     return toSETOR;
    }
   return toOR;
  case '^':
   if (scfg & PAS)
    {
     if (buf[pos] == '=') // (RO) ^= operator
      {
       pos += 1;
       return toSETPOW;
      }
     return toPOW;
    }
   else
    {
     if (buf[pos] == '=') // (RO) ^= operator
      {
       pos += 1;
       return toSETXOR;
      }
     return toXOR;
    }
  case '#':
   if (operand) // (RO) # gauge simbol
    {
     float__t fval;
     char *fpos;
     if (buf[pos])
      {
       fval                = Awg ((float__t)strtod (buf + pos, &fpos));
       pos                 = fpos - buf;
       v_stack[v_sp].tag   = tvFLOAT;
       v_stack[v_sp].fval  = (float__t)fval;
       v_stack[v_sp].pos   = pos;
       v_stack[v_sp++].var = nullptr;
       return toOPERAND;
      }
     else
      {
       error ("bad numeric constant");
       return toERROR;
      }
    }
   else
    {
     if (buf[pos] == '=') // (RO) #= operator
      {
       pos += 1;
       return toSETXOR;
      }
     return toXOR;
    }
  case ',':
   return toCOMMA;
  case '{':
   return braces ();
  case '[':
   if (operand)
    return sqbraces ();
   else
    {
     if (v_sp && v_stack[v_sp - 1].tag == tvMATRIX && v_stack[v_sp - 1].mval)
      {
       int row = 0, col = 0, ii = 0;
       ii = mx_idx (row, col);

       if (ii == 0)
        {
         error ("syntax error");
         return toERROR;
        }
       else if (ii == 1)
        {
         int idx = row; // single index provided, treat as linear index into matrix
         row     = idx / v_stack[v_sp - 1].mcols;
         col     = idx % v_stack[v_sp - 1].mcols;
         ii      = 2; // treat as element access if index is valid
        }

       if (ii == 2 && (v_stack[v_sp - 1].mrows > row) && (v_stack[v_sp - 1].mcols > col))
        {
         v_stack[v_sp].tag   = tvMX_ELEM;
         v_stack[v_sp].info  = v_stack[v_sp - 1].info;
         v_stack[v_sp].imval = (float__t)0.0L;
         v_stack[v_sp].fval  = v_stack[v_sp - 1].mval[row * v_stack[v_sp - 1].mcols + col];
         v_stack[v_sp].ival  = (int_t)v_stack[v_sp].fval;
         v_stack[v_sp].pos   = pos;
         v_stack[v_sp].var   = nullptr;
         v_stack[v_sp].mrows = v_stack[v_sp - 1].mrows;
         v_stack[v_sp].mcols = v_stack[v_sp - 1].mcols;
         v_stack[v_sp].irows = row;
         v_stack[v_sp].icols = col;
         v_stack[v_sp].mval  = v_stack[v_sp - 1].mval; // keep pointer to matrix for later updates
         v_sp++;
         return toMX_ELEM;
        }
       else
        {
         errorf (pos, "Matrix index out of bounds: [%d,%d]", row, col);
         return toERROR;
        }
      }
     else
      {
       error ("syntax error");
       return toERROR;
      }
    }

  case '\'':
   {
    int_t ival;
    uint32_t info = 0;
    char *ipos;
    int n = 0;

    if (buf[pos] == '\\')
     {
      xscanf (buf + pos + 1, 1, ival, n);
      ipos = buf + pos + n + 1;
      if (*ipos == '\'')
       ipos++;
      else
       {
        error ("bad char constant");
        return toERROR;
       }
     }
    else
     {
      ipos = buf + pos + 1;
      if (*ipos == '\'')
       {
#ifdef _WCHAR_
#ifdef _WIN_
        if (*(ipos + 1) == 'W') // (RO) wide char constant
         {
          wchar_t wbuf[2];
          char cbuf[2];

          cbuf[0] = *(ipos - 1);
          cbuf[1] = '\0';

          MultiByteToWideChar (CP_OEMCP, 0, (LPSTR)cbuf, -1, (LPWSTR)wbuf, 2);
          // ival = *(int *)&wbuf[0];
          ival = 0;
          memcpy (&ival, &wbuf[0], 2);
          ipos += 2;
          fflags |= WCH;
          info |= WCH;
         }
        else
#endif /*_WIN_*/
#endif /*_WCHAR_*/
         {
          fflags |= CHR;
          info |= CHR;
          ival = *(unsigned char *)(ipos - 1);
          v_stack[v_sp].sval = (char *)sf_alloc (2, ptMALLOC);
          if (v_stack[v_sp].sval)
           {
            if (v_stack[v_sp].sval) v_stack[v_sp].sval[0] = *(ipos - 1);
            if (v_stack[v_sp].sval) v_stack[v_sp].sval[1] = '\0';
           }
          ipos++;
         }
       }
      else
       {
        return dqscan ('\'');
       }
     }
    pos                 = ipos - buf;
    v_stack[v_sp].tag   = tvINT;
    v_stack[v_sp].info  = info;
    v_stack[v_sp].ival  = ival;
    v_stack[v_sp].pos   = pos;
    v_stack[v_sp++].var = nullptr;
    return toOPERAND;
   }
#ifdef _WCHAR_
#ifdef _WIN_
  case 'L':
   {
    int_t ival;
    char *ipos;
    int n         = 0;
    uint32_t info = 0;
    if (buf[pos] == '\'')
     {
      if (buf[pos + 1] == '\\')
       {
        xscanf (buf + pos + 2, 2, ival, n);
        ipos = buf + pos + n + 2;
        if (*ipos == '\'')
         ipos++;
        else
         {
          error ("bad char constant");
          return toERROR;
         }
       }
      else
       {
        ipos = buf + pos;
        if (*(ipos + 2) == '\'')
         {
          wchar_t wbuf[2];
          char cbuf[2];

          cbuf[0] = *(ipos + 1);
          cbuf[1] = '\0';

          MultiByteToWideChar (CP_OEMCP, 0, (LPSTR)cbuf, -1, (LPWSTR)wbuf, 2);
          // ival = *(int *)&wbuf[0];
          ival = 0;
          memcpy (&ival, &wbuf[0], 2);
          ipos += 3;
          fflags |= WCH;
          info |= WCH;
         }
        else
         {
          error ("bad char constant");
          return toERROR;
         }
       }
      pos                 = ipos - buf;
      v_stack[v_sp].tag   = tvINT;
      v_stack[v_sp].info  = info;
      v_stack[v_sp].ival  = ival;
      v_stack[v_sp].pos   = pos;
      v_stack[v_sp++].var = nullptr;
      return toOPERAND;
     }
    goto def;
   }
#endif /*_WIN_*/
#endif /*_WCHAR_*/
  case '"':
   return dqscan ('"');

#ifdef _ENABLE_PREIMAGINARY_
  case 'i':
  case 'j':
   {
    uint32_t info = 0;
    char *fpos;
    if (buf[pos] && (isdigit (buf[pos] & 0x7f) || buf[pos] == '.'))
     {
      double fval = strtod (buf + pos, &fpos);
      if (scfg & (ENG | SCI | FRI))
       {
        char *spos = fpos;
        scientific (fpos, fval);
        if (fpos > spos) info |= ENG;
       }
      int ferr = errno;
      if ((ferr) && (*fpos == '.'))
       {
        pos = fpos - buf + 1;
        error ("bad numeric constant");
        return toERROR;
       }
      if (v_sp == max_stack_size)
       {
        error ("stack overflow");
        return toERROR;
       }

      c_imaginary = buf[pos - 1];

      v_stack[v_sp].tag  = tvCOMPLEX;
      v_stack[v_sp].info = info;
      scfg |= CPX;
      fflags |= CPX;
      v_stack[v_sp].imval = (float__t)fval;
      v_stack[v_sp].fval  = (float__t)0.0;
      v_stack[v_sp].pos   = pos;
      v_stack[v_sp++].var = nullptr;
      pos                 = fpos - buf;
      return toOPERAND;
     }
    else
     goto def;
   }
#endif /*_ENABLE_PREIMAGINARY_*/
  case '.':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '\\':
  case '$':
   return dscan (operand, percent);
  default:
  def:
   pos -= 1;
   np = name;
   while (isalnum (buf[pos] & 0x7f) || buf[pos] == '@' || buf[pos] == '_' || buf[pos] == '?')
    {
     *np++ = buf[pos++] & 0x7f;
    }
   if (np == buf)
    {
     error ("Bad character");
     return toERROR;
    }
   *np         = '\0';
   symbol *sym = nullptr;
   if (strlen (name) > MAXNAME)
    {
     error ("Name too long");
     return toERROR;
    }
   if (name[0])
    {
     if (buf[pos] == '\0')
      sym = find (name);
     else
      sym = add (tsVARIABLE, name);
    }
   if (v_sp == max_stack_size)
    {
     error ("stack overflow");
     return toERROR;
    }

   if (sym)
    {
     blockflag |= sym->block;
     if (sym->tag == tsVARIABLE) strcpy (lastvar, sym->name);
     v_stack[v_sp]       = sym->val;
     v_stack[v_sp].pos   = pos;
     v_stack[v_sp++].var = sym;
     if (sym->tag == tsSUM)
      return toSOLVE;
     else if (sym->tag == tsINTEGR)
      return toSOLVE;
     else if (sym->tag == tsDIFF)
      return toSOLVE;
     else if (sym->tag == tsEXTR)
      return toSOLVE;
     else if (sym->tag == tsINVERSE)
      return toSOLVE;
     else if (sym->tag == tsSOLVE)
      return toSOLVE;
     else if (sym->tag == tsCALC)
      return toSOLVE;
     else if (sym->tag == tsPLOT)
      return toSOLVE;
     else if (sym->tag == tsFOR)
      return toSOLVE;
     return (sym->tag == tsVARIABLE || sym->tag == tsCONSTANT) ? toOPERAND : toFUNC;
    }
   else
    return toOPERAND;
  }
}

bool isname (const char *name)
{
 if (!name || !*name) return false;
 char c = *name & 0x7f;
 if (!((c == '_') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '@') || (c == '?')))
  return false;
 for (const char *p = name + 1; *p; p++)
  {
   if (!(isalnum (*p & 0x7f) || *p == '@' || *p == '_' || *p == '?')) return false;
  }
 return true;
}
#pragma endregion
//---------------------------------------------------------------------------

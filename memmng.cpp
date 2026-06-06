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

#pragma region Memory Management
// Memory management functions for static memory management mode

// Do not clean variables with other trash
void calculator::save_vars_mem (void) 
{
 symbol *sp;
 for (int i = 0; i < hash_table_size; i++)
  {
   if ((sp = hash_table[i]))
    {
     do
      {
       unregister_mem (sp->val.sval);
       unregister_mem (sp->val.mval);
       sp = sp->next;
      }
     while (sp);
    }
  }
}

// Duplicate a string and register it for cleanup
 char *calculator::dupString (const char *src) // Duplicate a string and register it for cleanup
 {
  if (src && src[0]) 
   {
    char *dup = strdup (src); // Duplicate string using strdup
    if (dup) register_mem (dup, ptMALLOC); // Register duplicated string for cleanup
    return dup;
   }
  return nullptr;
 }

 // Duplicate a matrix and register it for cleanup
 float__t *calculator::dupMatrix (value &val)
  {
   // Assuming matrix is stored as a flat array of float__t with dimensions res_rows x res_cols
   if (val.tag != tvMATRIX || !val.mval) return nullptr; // Check if it's a valid matrix
   int rows  = val.mrows;
   int cols  = val.mcols;
   int msize = rows * cols * sizeof (float__t);
   if (msize)
    {
     float__t *new_mval = (float__t *)malloc (msize);
     if (new_mval)
      {
       register_mem (new_mval, ptMALLOC); // Register the new matrix for cleanup
       memcpy (new_mval, val.mval, msize);
       return new_mval;
      }
    }
   return nullptr;
  }

 // Duplicate a bitmap and register it for cleanup
  bmpdraw *calculator::dupBMP (bmpdraw *src)
  {
   if (!src || !src->data) return nullptr;

   bmpdraw *dup = new bmpdraw ();
   if (!dup) return nullptr;

   // Create bitmap data
   if (!dup->newbmp (src->width, src->height, 0))
    {
     delete dup;
     return nullptr;
    }

   // Copy pixel data
   memcpy (dup->data, src->data, src->height * src->rowSize);

   // Copy positioning
   dup->top  = src->top;
   dup->left = src->left;

   // Register for cleanup
   if (dup) register_mem (dup, ptBMP);

   return dup;
  }



bool calculator::dupvar (value &dst, value &src) 
{
 dst.tag = src.tag;
 dst.fval = src.get();
 dst.ival = src.get_int ();
 dst.imval = src.imval;
 if (src.tag == tvSTR)
  {
   dst.sval = dupString (src.sval); // Duplicate string value
   if (!dst.sval) return false; // Check for duplication failure
  }
 else if (src.tag == tvMATRIX)
  {
   dst.mval = dupMatrix (src); // Duplicate matrix value
   if (!dst.mval) return false; // Check for duplication failure
   dst.mrows = src.mrows;
   dst.mcols = src.mcols;
  }
 else if (src.tag == tvBMP)
  {
   dst.sval = (char *)dupBMP ((bmpdraw *)src.sval); // Duplicate bitmap value
   if (!dst.sval) return false; // Check for duplication failure
  }
 //else if (src.tag == tvCOLOR)
 // {
 //  char color[128] = { 0 };
 //  rgb_to_color_name_extended (color, true, src.get_int ());
 //  dst.sval = dupString (color); // Duplicate color name string
 // }
 return true;
}

bool calculator::freevar (value &src)
{
 if (src.tag == tvSTR)
  {
   sf_free (src.sval, ptMALLOC); // Unregister string value
   src.sval = nullptr;
  }
 else if (src.tag == tvMATRIX)
  {
   if (src.mval) sf_free (src.mval, ptMALLOC); // Unregister matrix value
   src.mval = nullptr;
   src.mrows = 0;
   src.mcols = 0;
  }
 else if (src.tag == tvBMP)
  {
   if (src.sval) sf_free (src.sval, ptBMP); // Unregister bitmap value
   src.sval = nullptr;
  }
 //else if (src.tag == tvCOLOR)
 // {
 //  if (src.sval) sf_free (src.sval, ptMALLOC); // Unregister color name string
 //  src.sval = nullptr;
 // }
 src.tag = tvERR; // Reset tag to indicate variable is now empty
 src.ival = 0;
 src.fval = qnan;
 src.imval = (float__t)0.0L;
 if (src.sval) sf_free (src.sval, ptMALLOC); 
 src.sval = nullptr; 
 return true;
}

#pragma endregion
//---------------------------------------------------------------------------

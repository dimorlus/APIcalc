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

#pragma region Symbols and vars

// Copy symbols from the provided symbol table with the specified copy mask
// Copy symbols from the provided symbol table
// The function should create a new hashed linked list
// from the existing one, duplicate the names, and create
// copies of string variables that will be deleted in
// the destructor. At the same time, it should allow
// flexible selection of which nodes to copy and which
// not (depending on the value of the tag field).
// All pointers freed in the destructor must be
// recreated; the rest can be copied.

void calculator::copy_symbols (symbol **symtab, uint64_t mask)
{
 symbol *sp;
 symbol *nsp;

 for (int i = 0; i < hash_table_size; i++)
 {
  if ((sp = symtab[i]) != nullptr)
   {
    do
     {
      nsp = sp->next;
      if (sp->name[0])
       {
        if ((mask & (1ULL << sp->tag))) // Check if the symbol's tag matches the copy mask
         {
          symbol *new_symbol = add (sp->tag, sp->name); //, sp->func);
          if (new_symbol)
           {
            new_symbol->tag = sp->tag;
            strcpy (new_symbol->name, sp->name); // Copy symbol name 

            new_symbol->fidx = sp->fidx;
            if (sp->tag == tsUFUNCT && sp->func)
              {
                new_symbol->func = strdup((char *)sp->func); // Copy UDF
                //register_mem (new_symbol->func); // Register UDF for cleanup
              }
            else
                new_symbol->func = sp->func; // Copy function pointer as is (static functions)
            
            new_symbol->val.tag = sp->val.tag;
            new_symbol->val.ival = sp->val.ival;
            new_symbol->val.fval = sp->val.fval;
            new_symbol->val.imval = sp->val.imval;
            new_symbol->val.mcols = sp->val.mcols;
            new_symbol->val.mrows = sp->val.mrows;
            new_symbol->val.sval  = nullptr;
            new_symbol->val.mval  = nullptr;
            #ifdef _COPY_BMP_ //child calculator does'nt work with BMP values
            if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvBMP))
              new_symbol->val.sval = (char*)dupBMP ((bmpdraw *)sp->val.sval); // Duplicate and register BMP value 
            else 
            #endif
            if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvSTR))
              new_symbol->val.sval = dupString (sp->val.sval); // Duplicate and register string value 
            else 
            if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvMATRIX))
              new_symbol->val.mval = dupMatrix (sp->val); // Duplicate and register matrix value 
           }
         }
       }
      sp = nsp;
     }
    while (nsp);
   }
 }
}

void calculator::destroyvars (void) // Free all symbols in the hash table
{
 symbol *sp;
 symbol *nsp;

 for (int i = 0; i < hash_table_size; i++)
  {
   if ((sp = hash_table[i]) != nullptr)
    {
     do
      {
       nsp = sp->next;
       if (sp->name[0])
        {
         if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvBMP))
          {
           sf_free (sp->val.sval, ptBMP); // Free BMP value using sf_free to ensure it's unregistered
           sp->val.sval = nullptr;
          }
         if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvSTR))
          {
           sf_free (sp->val.sval, ptMALLOC); // Free string value using sf_free to ensure it's unregistered
           sp->val.sval = nullptr;
          }
         if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvMATRIX))
          {
           sf_free (sp->val.mval, ptMALLOC); // Free matrix value using sf_free to ensure it's unregistered
           sp->val.mval = nullptr;
          }
         if (sp->tag == tsUFUNCT && sp->func) 
          {
           sf_free (sp->func, ptMALLOC); // Free function name using sf_free to ensure it's unregistered
           sp->func = nullptr;
          }    
         sp->name[0] = '\0';
        }
       delete sp;
       sp            = nsp;
       hash_table[i] = nullptr;
      }
     while (nsp);
    }
  }
}
#pragma endregion
//---------------------------------------------------------------------------

#pragma region Variable listing and hash table management

int calculator::varlist (char *buf, int bsize, int *maxlen)
{
 char *cp = buf;
 symbol *sp;
 int lineCount = 0;
 int localMax  = 0;
 for (int i = 0; i < hash_table_size; i++)
  {
   if ((sp = hash_table[i]) != nullptr)
    {
     do
      {
       if (sp->tag == tsVARIABLE)
        {
         int written = 0;
         if ((sp->val.tag == tvCOMPLEX) || (sp->val.imval != 0))
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %-.5g%+.5gi\r\n", sp->name,
                               (double)sp->val.fval, (double)sp->val.imval);
          }
         else if (sp->val.tag == tvSTR)
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = \"%s\"\r\n", sp->name,
                               sp->val.sval ? sp->val.sval : "");
          }
         else if (sp->val.tag == tvMATRIX)
          {
           char mstr[1024];
           Mxprint (sp->val.tag, sp->val.mrows, sp->val.mcols, sp->val.mval, mstr, false, nullptr);
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %s\r\n", sp->name, mstr);

          }
         else if (sp->val.tag == tvINT)
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %lld (%llX)\r\n", sp->name,
                               (long long)sp->val.ival, (long long)sp->val.ival);
          }
         else if (sp->val.tag == tvCOLOR)
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %x\r\n", sp->name,
                               (uint32_t)sp->val.ival);
          }
         else if (sp->val.tag == tvBMP)
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = BMP%dx%d\r\n", sp->name,
                               ((bmpdraw *)sp->val.sval)->width, ((bmpdraw *)sp->val.sval)->height);
          }
         else
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %-.5g\r\n", sp->name,
                               (double)sp->val.get());
          }
         if (written > localMax) localMax = written;
         cp += written;
         lineCount++;
        }
       sp = sp->next;
      }
     while (sp);
    }
  }
 if (maxlen) *maxlen = localMax;
 return lineCount;
}

//---------------------------------------------------------------------------
// A simple hash function for strings (djb2 by Dan Bernstein)
unsigned calculator::string_hash_function (const char *p) 
{
 unsigned h = 0, g;
 while (*p)
  {
   if (scfg & UPCASE)
    h = (h << 4) + tolower (*p++);
   else
    h = (h << 4) + *p++;

   if ((g = h & 0xF0000000) != 0)
    {
     h ^= g >> 24;
    }
   h &= ~g;
  }
 return h;
}

// Add a symbol to the hash table, or return the existing symbol if it already exists
symbol *calculator::add (t_symbol tag, v_func fidx, const char *name, void *func, bool block)
{
 symbol *sp;
 unsigned h = string_hash_function (name) % hash_table_size;
 for (sp = hash_table[h]; sp != nullptr; sp = sp->next)
  {
   if (scfg & UPCASE)
    {
     if (stricmp (sp->name, name) == 0) return sp;
    }
   else
    {
     if (strcmp (sp->name, name) == 0) return sp;
    }
  }
 sp            = new symbol;
 sp->tag       = tag;
 sp->fidx      = fidx;
 sp->func      = func;
 sp->block     = block;
 strcpy (sp->name, name);
 sp->val.tag   = tvERR; // tvINT;
 sp->val.ival  = 0;
 sp->val.fval  = qnan;
 sp->val.imval = ((float__t)0.0);
 sp->val.sval  = nullptr;
 sp->val.mcols = 0;
 sp->val.mrows = 0;
 sp->val.mval  = nullptr;
 sp->next      = hash_table[h];
 hash_table[h] = sp;
 return sp;
}

// Add a symbol to the hash table, or return the existing symbol if it already exists (without
// function index)
symbol *calculator::add (t_symbol tag, const char *name, void *func, bool block)
{
 symbol *sp;
 unsigned h = string_hash_function (name) % hash_table_size;
 for (sp = hash_table[h]; sp != nullptr; sp = sp->next)
  {
   if (scfg & UPCASE)
    {
     if (stricmp (sp->name, name) == 0) return sp;
    }
   else
    {
     if (strcmp (sp->name, name) == 0) return sp;
    }
  }
 sp            = new symbol;
 sp->tag       = tag;
 sp->func      = func;
 sp->block     = block; 
 strcpy (sp->name, name);
 sp->val.tag   = tvERR; // tvINT;
 sp->val.ival  = 0;
 sp->val.fval  = qnan;
 sp->val.imval = ((float__t)0.0);
 sp->val.sval  = nullptr;
 sp->next      = hash_table[h];
 hash_table[h] = sp;
 return sp;
}

// Add constant value
bool calculator::addconst (const char *name, value &val)
{
 if (find (name))
  {
   error ("constant redefinition");
   return false;
  } 
 symbol *sp    = add (tsCONSTANT, name);
 sp->val.tag   = val.tag;
 sp->val.fval  = val.fval;
 sp->val.ival  = val.ival;
 sp->val.imval = val.imval;
 // For string and matrix types, we need to copy the values
 if (val.sval)
  sp->val.sval = strdup (val.sval);
 else
  sp->val.sval = nullptr;
 register_mem (sp->val.sval);
 if ((sp->tag == tsCONSTANT) && (sp->val.tag == tvMATRIX))
  {
   sp->val.mval = dupMatrix (val);
  }
 sp->val.mcols = val.mcols;
 sp->val.mrows = val.mrows;
 return true;
}

// Add a constant to the hash table, or return an error if it already exists (not in use)
float__t calculator::AddConst (const char *name, float__t val)
{
 if (find (name))
  {
   error ("constant redefinition");
   return qnan; 
  } 
 addfconst (name, val);
 return val;
}

// Add a variable to the hash table (not in use)
float__t calculator::AddVar (const char *name, float__t val)
{
 addfvar (name, val);
 return val;
}

// Find a symbol in the hash table by name, or return nullptr if it doesn't exist
symbol *calculator::find (const char *name)
{
 symbol *sp;
 unsigned h = string_hash_function (name) % hash_table_size;
 for (sp = hash_table[h]; sp != nullptr; sp = sp->next)
  {
   if (scfg & UPCASE)
    {
     if (stricmp (sp->name, name) == 0) return sp;
    }
   else
    {
     if (strcmp (sp->name, name) == 0) return sp;
    }
  }
 return nullptr;
}

// Add a user-defined function to the hash table, or return an error if it already exists
symbol *calculator::addUF (const char *name, const char *expr)
{
 if (!expr) return nullptr;
 symbol *sp = find (name);
 
 if (sp && sp->tag == tsUFUNCT)
  {
   // redefine user function.
   if (sp->func)
    {
     if (strcmp ((char*)sp->func, expr) == 0)
      return sp; // If the existing function is the same as the new one, return it
     free (sp->func);
     sp->func = strdup (expr);
    }
   return sp;
  }
 if (sp) 
  return nullptr; // If a symbol with the same name exists but is not a user function, return an error

 // If no existing symbol is found, add the new user function to the hash table
 unsigned h    = string_hash_function (name) % hash_table_size;
 sp            = new symbol;
 sp->tag       = tsUFUNCT;
 sp->func      = strdup(expr);
 strcpy (sp->name, name);
 sp->val.tag   = tvUFUNCT;
 sp->val.ival  = 0;
 sp->val.fval  = ((float__t)0.0L);
 sp->val.imval = ((float__t)0.0L);
 sp->val.sval  = nullptr;
 sp->val.mcols = 0;
 sp->val.mrows = 0;
 sp->val.mval  = nullptr;
 sp->next      = hash_table[h];
 hash_table[h] = sp;

 return sp;
}

// Add a float constant to the hash table 
void calculator::addfconst (const char *name, float__t val)
{
 symbol *sp   = add (tsCONSTANT, name);
 sp->val.tag  = tvFLOAT;
 sp->val.fval = val;
 sp->val.ival = 0;
 sp->val.imval = ((float__t)0.0L);
 sp->val.sval  = nullptr;
 sp->val.mcols = 0;
 sp->val.mrows = 0;
 sp->val.mval  = nullptr;
}

// Add a float variable to the hash table
void calculator::addfvar (const char *name, float__t fval, float__t imval)
{
 symbol *sp   = add (tsVARIABLE, name);
 sp->val.fval = fval;
 sp->val.ival  = 0;
 sp->val.imval = imval;
 if (imval == (float__t)0.0L) sp->val.tag  = tvFLOAT;
 else sp->val.tag  = tvCOMPLEX;
 sp->val.sval  = nullptr;
 sp->val.mcols = 0;
 sp->val.mrows = 0;
 sp->val.mval  = nullptr;
}

// Add a float variable to the hash table
void calculator::addivar (const char *name, int_t ival)
{
 symbol *sp    = add (tsVARIABLE, name);
 sp->val.fval  = (float__t)ival;
 sp->val.ival  = ival;
 sp->val.imval = (float__t)0.0L;
 sp->val.tag = tvINT;
 sp->val.sval  = nullptr;
 sp->val.mcols = 0;
 sp->val.mrows = 0;
 sp->val.mval  = nullptr;
}

int_t calculator::getivar (const char *name)
{
 symbol *sp = find (name);
 if (sp && ((sp->val.tag == tvINT) || (sp->val.tag == tvFLOAT)))
  {
   return sp->val.get_int();
  }
 return 0;
}

float__t calculator::getfvar (const char *name)
{
 symbol *sp = find (name);
 if (sp && ((sp->val.tag == tvINT) || (sp->val.tag == tvFLOAT)))
  {
   return sp->val.get ();
  }
 return qnan;
}

char *calculator::getsvar (const char *name)
{
 symbol *sp = find (name);
 if (sp && sp->val.tag == tvSTR)
  {
   return sp->val.sval;
  }
 return nullptr;
}

// Add a string variable to the hash table
void calculator::addsvar (const char *name, const char *svar)
{
 symbol *sp    = add (tsVARIABLE, name);
 sp->val.fval  = 0L;
 sp->val.ival  = 0;
 sp->val.imval = 0L;
 sp->val.tag   = tvSTR;
 if (svar) sp->val.sval = dupString (svar);
 sp->val.mcols = 0;
 sp->val.mrows = 0;
 sp->val.mval  = nullptr;
}


// Add  variable to the hash table
// todo:add matrix variable
bool calculator::addvar (const char *name, value &val)
{
 symbol *sp   = add (tsVARIABLE, name);
 sp->val.tag  = val.tag;
 sp->val.fval = val.fval;
 sp->val.ival = val.ival;
 sp->val.imval = val.imval;
 // For string and matrix types, we need to copy the values
 if (val.sval) sp->val.sval = strdup(val.sval);
 else sp->val.sval = nullptr;
 register_mem (sp->val.sval);
 if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvMATRIX))
  {
   sp->val.mval = dupMatrix (val);
  }   
 sp->val.mcols = val.mcols;
 sp->val.mrows = val.mrows;
 return true;
}

// Add an imaginary constant to the hash table (if enabled)
void calculator::addim ()
{
#ifdef _ENABLE_PREIMAGINARY_
 symbol *sp = add(tsCONSTANT, "i");
 sp->val.tag       = tvCOMPLEX;
 sp->val.fval      = ((float__t)0.0L);
 sp->val.imval     = ((float__t)1.0L);
 sp                = add (tsCONSTANT, "j");
 sp->val.tag       = tvCOMPLEX;
 sp->val.fval      = ((float__t)0.0L);
 sp->val.imval     = ((float__t)1.0L);
#endif // _ENABLE_PREIMAGINARY_
}

// Add an integer constant to the hash table (not in use)
void calculator::addiconst (const char *name, int_t val)
{
 symbol *sp   = add (tsCONSTANT, name);
 sp->val.tag  = tvINT;
 sp->val.ival = val;
 sp->val.fval = (float__t)val;
 sp->val.imval = 0;
}

// Add an integer variable to the hash table
void calculator::addlconst (const char *name, float__t fval, int_t ival)
{
 symbol *sp   = add (tsCONSTANT, name);
 sp->val.tag  = tvINT;
 sp->val.fval = fval;
 sp->val.ival = ival;
 sp->val.imval = 0;
}

#pragma endregion
//---------------------------------------------------------------------------

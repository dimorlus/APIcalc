// calculator_dll.h — replacement for scalc.h for those who use dll
#pragma once
#include "calclib.h"

class calculator
{
 HCALC h;
public:
 calculator (int options = PAS + SCI + UPCASE) { h = calc_create(options); }
 ~calculator () { calc_destroy(h); }

 // non-copyable — like the original
 calculator (const calculator&)            = delete;
 calculator& operator= (const calculator&) = delete;

 void setEscFn (int (__cdecl *fn) (void)) { calc_setEscFn (h, fn); }
 double evaluate (char *expr)  { return calc_evaluate(h, expr); } 
 char *error() {return calc_error(h);}
 int print (char *str, int options, int binw, int *size = nullptr)
 {
  return calc_print (h, str, options, binw, size);
 }
 int printres (char *str, int options = FFLOAT, int binw = 64)
 {
  return calc_print_res (h, str, options, binw);
 }
 int  isfflags () { return calc_get_flags(h); }
 void clrfflags() { calc_clr_flags(h); }
 void syntax (int s) { calc_set_syntax(h, s); }
 int issyntax() {return calc_get_syntax(h);}
 void addfn (const char *name, void *fn) { calc_addfn(h, name, fn); }
 int varlist (char *buf, int bsize, int *maxlen = nullptr) {return calc_varlist (h, buf, bsize, maxlen);}
};
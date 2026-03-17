// calculator_dll.h — замена scalc.h для тех кто использует dll
#pragma once
#include "calclib.h"

/*
calculator (m_options);
addfn ("menu", (void *)(int (*) (int))MenuFunction);
syntax (m_options);
evaluate (exprBuf);
evaluate (line);
error ();
printres (resultStr);
issyntax ();
print (printBuf, m_options, m_binWidth, &prnSize);
varlist (textBuffer, sizeof (textBuffer) - 1, &maxlen);
*/
class calculator
{
 HCALC h;
public:
 calculator (int options = PAS + SCI + UPCASE) { h = calc_create(options); }
 ~calculator () { calc_destroy(h); }

 // некопируемый — как и оригинал
 calculator (const calculator&)            = delete;
 calculator& operator= (const calculator&) = delete;

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
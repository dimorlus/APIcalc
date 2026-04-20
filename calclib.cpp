// calc_dll.cpp — Realization, only this file knows about calculator
#include "calclib.h"
#include "scalc.h"

extern "C" {

HCALC __cdecl calc_create (int cfg)
{
 return new calculator(cfg);
}

void __cdecl calc_destroy(HCALC h)
{
 delete (calculator*)h;
}

double __cdecl calc_evaluate(HCALC h, char* expr)
{
 calculator* c = (calculator*)h;
 return c->evaluate(expr);
}
int __cdecl calc_print (HCALC h, char* str, int options, int binw, int *size)
{
 calculator* c = (calculator*)h;
 return c->print(str, options, binw, size);
}	
int __cdecl calc_print_res (HCALC h, char* str, int options, int binw)
{
 calculator* c = (calculator*)h;
 return c->printres(str, options, binw);
}	

void __cdecl calc_addfn (HCALC h, const char* name, void *fn)
{
 calculator* c = (calculator*)h;
 c->addfn (name, fn);
}	

int  __cdecl calc_get_flags(HCALC h)
{
 calculator* c = (calculator*)h;
 return c->isfflags();
}	

void  __cdecl calc_clr_flags(HCALC h)
{
 calculator* c = (calculator*)h;
 c->clrfflags();
}

void __cdecl calc_set_syntax(HCALC h, int syntax)
{
 calculator* c = (calculator*)h;
 c->syntax(syntax);
}

int __cdecl calc_get_syntax (HCALC h)
{
 calculator *c = (calculator *)h;
 return c->issyntax ();
}


int __cdecl calc_varlist(HCALC h, char *buf, int bsize, int *maxlen)
{ 
 calculator* c = (calculator*)h;
 return c->varlist(buf, bsize, maxlen);
}

char *__cdecl calc_error (HCALC h)
{
 calculator *c = (calculator *)h;
 return c->error ();
}

void __cdecl calc_setEscFn (HCALC h, int (__cdecl*fn) (void))
{
 calculator *c = (calculator *)h;
 c->setEscFn (fn);
}

void __cdecl calc_setFileDlgFn (HCALC h, bool (__cdecl *fn) (char *, int))
{
 calculator *c = (calculator *)h;
 c->setFileDlgFn (fn);
}
} // extern "C"
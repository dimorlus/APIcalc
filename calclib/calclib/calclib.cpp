// calc_dll.cpp — реализация, только этот файл знает про calculator
//#include "pch.h"
#include "calclib.h"
#include "../../scalc.h"

extern "C" {

HCALC __cdecl calc_create(void)
{
 return new calculator();
}

void __cdecl calc_destroy(HCALC h)
{
 delete (calculator*)h;
}

void __cdecl calc_evaluate(HCALC h, char* expr)
{
 calculator* c = (calculator*)h;
 c->evaluate(expr);
}
void __cdecl calc_print (HCALC h, char* str, int options, int binw)
{
 calculator* c = (calculator*)h;
 c->print(str, options, binw);
}	
void __cdecl calc_print_res (HCALC h, char* str, int options, int binw)
{
 calculator* c = (calculator*)h;
 c->printres(str, options, binw);
}	

void __cdecl calc_addfn (HCALC h, char* name, void *fn)
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


} // extern "C"
//---------------------------------------------------------------------------

#ifndef sfmtsH
#define sfmtsH
//---------------------------------------------------------------------------
#include <cstdint>

extern int t2str(char * str, uint64_t sec);
extern int b2str(char *str, const char *fmt, uint64_t b);
extern int d2scistr(char *str, float__t d);
extern int d2frcstr(char *str, float__t d, int eps_order);
extern int b2scistr(char *str, float__t d);
extern int d2nrmstr(char *str, float__t d);
extern int dgr2str(char *str, float__t d);
extern int chr2str(char *str, unsigned char c);
extern int wchr2str(char *str, int i);
extern int nx_time2str(char *str, uint64_t time);
extern void fraction(double val, double eps, int &num, int &denum);
extern int format_out(int Options, int scfg, int binwide, int n, 
	    float__t fVal, float__t imVal, int64_t iVal, char* expr, 
	    char strings[20][80], calculator* ccalc);
//n = format_out(opts.options, scfg, binwide, n, fVal, imVal, iVal, exprBuf, strings, ccalc);
#endif

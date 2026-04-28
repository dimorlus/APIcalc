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

#define _WIN_
#define _WCHAR_ // L'c' and 'c'W input format allow
#define _ENABLE_PREIMAGINARY_

// Macros to determine if an operator is binary or unary based on its position in the enumeration
#define BINARY(opd) (opd >= toPOW)
#define UNARY(opd)  ((opd >= toPOSTINC) && (opd <= toCOM))

#ifdef _ENABLE_DEBUG_LOG_
// Debug logging function
void DebugLog (const char *format, ...)
{
 va_list args;
 va_start (args, format);
 char message[1024];
 vsnprintf (message, sizeof (message), format, args);
 va_end (args);

 static std::ofstream debugFile ("debug_stack.txt", std::ios::app);
 debugFile << message << std::endl;
 debugFile.flush ();
}
#else // Stub function when debug logging is disabled
#ifndef __BORLANDC__
#define DebugLog(x, ...) ((void)0)
#endif //__BORLANDC__
#endif // _ENABLE_DEBUG_LOG_
#pragma endregion
//---------------------------------------------------------------------------

#pragma region Calculator Class constructor and destructor
calculator::calculator (int cfg, symbol **symtab, uint64_t copyMask, int deep)
{
 this->deep  = deep + 1; // Set the current stack depth (incremented by 1 to account for the new instance)
 v_sp         = 0;    // Clear the value stack pointer
 o_sp         = 0;    // Clear the operator stack pointer

 blockflag     = false;   // Clear the block flag
 EscFn         = nullptr; // Clear the escape function pointer
 FileDlgFn     = nullptr; // Clear the file dialog function pointer
 ShowImageFn   = nullptr; // Clear the show image function pointer

 res_cols      = 0;    // Clear the result columns count
 res_rows      = 0;    // Clear the result rows count
 res_mval      = nullptr; // Clear the matrix result pointer
 result_info   = 0;    // Clear the result info

 result_fval  = qnan; // Clear the floating-point result
 result_imval = (float__t)0.0L;  // Clear the imaginary result
 result_ival  = 0;    // Clear the integer result
 buf           = nullptr;
 errpos        = 0;
 errtype       = teSyntax;
 pos           = 0;
 expr          = false;
 tmp_var_count = 0;
 err[0]        = '\0';
 scfg          = cfg;
 fflags        = 0;
 sres[0]       = '\0';
 memset (v_stack, 0, sizeof v_stack);

 fprec         = 16; // Default precision for output formatting
 c_imaginary   = 'i';
 Randomize ();
 srand (static_cast<unsigned int> (time (nullptr)));
 memset (hash_table, 0, sizeof hash_table);

 init_mem_list ();

 if (symtab) copy_symbols (symtab, copyMask);
 else AddPredefined ();
}

calculator::~calculator (void)
{
 clear_v_stack ();
 destroyvars ();
 clear_mem_list ();
 if (res_mval) free (res_mval);
}
#pragma endregion
//---------------------------------------------------------------------------

#pragma region Wrapper Functions for External Use
bool Const (void *clc, char *name, value &x)
{
 return ((calculator *)clc)->addconst (name, x);
}

bool Var (void *clc, char *name, value &x)
{
 return ((calculator *)clc)->addvar (name, x);
}

float__t Det (void *clc, value &M)
{
 return ((calculator *)clc)->mxDet (M);
}

float__t Trace (void *clc, value &M)
{
 return ((calculator *)clc)->mxTrace (M);
}

float__t Norm (void *clc, value &M)
{
 return ((calculator *)clc)->mxNorm (M);
}

bool Dot (void *clc, value &res, value &A, value &B)
{
 return ((calculator *)clc)->mxDot (res, A, B);
}

bool Cross (void *clc, value &res, value &A, value &B)
{
 return ((calculator *)clc)->mxCross (res, A, B);
}

bool Zeros (void *clc, value &res, int rows, int cols)
{
 return ((calculator *)clc)->mxZeros (res, rows, cols);
}

bool Diag (void *clc, value &res, int rows, int cols)
{
 return ((calculator *)clc)->mxDiag (res, rows, cols);
}

float__t Rows (void *clc, value &M)
{
 return ((calculator *)clc)->mxDim (M, mxRows);
}

float__t Cols (void *clc, value &M)
{
 return ((calculator *)clc)->mxDim (M, mxCols);
}

float__t Size (void *clc, value &M)
{
 return ((calculator *)clc)->mxDim (M, mxSize);
}

int_t SetPrecision (void *clc, int_t prec)
{
 calculator *calc = (calculator *)clc;
 if (prec < 0) prec = 0;
 if (prec > MAX_PREC) prec = MAX_PREC;
 calc->set_fprec (prec);
 return prec;
}
#pragma endregion
//---------------------------------------------------------------------------

#pragma region Predefined Symbols
void calculator::AddPredefined (void)
{
 // This function can be used to add predefined constants and functions
 // to the symbol table. It is called from the constructor after initializing
 // the hash table and copying symbols from the provided symbol table (if any).
 add (tsSOLVE, "solve", nullptr);
 add (tsCALC, "calc", nullptr);
 add (tsINTEGR, "integr", nullptr);
 add (tsINTEGR, "integral", nullptr);
 add (tsSUM, "sum", nullptr);
 add (tsFOR, "for", nullptr);
 add (tsIF, "if", nullptr);

 // Cartesian plots
 add (tsPLOT, pl_plot, "plot", nullptr, true);
 add (tsPLOT, pl_fplot, "fplot", nullptr, true);
 add (tsPLOT, pl_oplot, "oplot", nullptr, true);

 // Polar plots
 add (tsPLOT, pl_plotpol, "plotpol", nullptr, true);
 add (tsPLOT, pl_fplotpol, "fplotpol", nullptr, true);
 add (tsPLOT, pl_oplotpol, "oplotpol", nullptr, true);

 // Parametric plots
 add (tsPLOT, pl_xyplot, "plotxy", nullptr, true);
 add (tsPLOT, pl_fxyplot, "fplotxy", nullptr, true);
 add (tsPLOT, pl_oxyplot, "oplotxy", nullptr, true);

 // Logarithmic plots
 add (tsPLOT, pl_plotlgx, "plotlgx", nullptr, true);
 add (tsPLOT, pl_fplotlgx, "fplotlgx", nullptr, true);
 add (tsPLOT, pl_oplotlgx, "oplotlgx", nullptr, true);

 add (tsPLOT, pl_plotlgy, "plotlgy", nullptr, true);
 add (tsPLOT, pl_fplotlgy, "fplotlgy", nullptr, true);
 add (tsPLOT, pl_oplotlgy, "oplotlgy", nullptr, true);
 add (tsPLOT, pl_plotlgxy, "plotlgxy", nullptr, true);
 add (tsPLOT, pl_fplotlgxy, "fplotlgxy", nullptr, true);
 add (tsPLOT, pl_oplotlgxy, "oplotlgxy", nullptr, true);

 // Smith chart
 add (tsPLOT, pl_plotsmith, "plotsmith", nullptr, true);
 add (tsPLOT, pl_fplotsmith, "fplotsmith", nullptr, true);
 add (tsPLOT, pl_oplotsmith, "oplotsmith", nullptr, true);

 add (tsPLOT, pl_plotsmithz, "plotsmithz", nullptr, true);
 add (tsPLOT, pl_fplotsmithz, "fplotsmithz", nullptr, true);
 add (tsPLOT, pl_oplotsmithz, "osmithz", nullptr, true);

 // Data plots (points)
 add (tsPLOT, pl_plotdata, "plotdata", nullptr, true);
 add (tsPLOT, pl_fplotdata, "fplotdata", nullptr, true);
 add (tsPLOT, pl_oplotdata, "oplotdata", nullptr, true);

 // Data plots (lines)
 add (tsPLOT, pl_plotdatal, "plotdatal", nullptr, true);
 add (tsPLOT, pl_fplotdatal, "fplotdatal", nullptr, true);
 add (tsPLOT, pl_oplotdatal, "oplotdatal", nullptr, true);

 add (tsDIFF, "diff", nullptr);
 add (tsDIFF, "derivative", nullptr);

 add (tsFFUNCM, "det", (void *)Det);
 add (tsFFUNCM, "trace", (void *)Trace);
 add (tsFFUNCM, "tr", (void *)Trace);
 add (tsFFUNCM, "norm", (void *)Norm);
 add (tsMFUNCM2, "dot", (void *)Dot);
 add (tsMFUNCM2, "cross", (void *)Cross);
 add (tsFFUNCM, "rows", (void *)Rows);
 add (tsFFUNCM, "cols", (void *)Cols);
 add (tsFFUNCM, "size", (void *)Size);
 add (tsMFUNCI2, "zeros", (void *)Zeros);
 add (tsMFUNCI2, "diag", (void *)Diag);
 add (tsMFUNCI2, "eye", (void *)Diag);

 add (tsFITFN, rtPoly, "fitpoly", nullptr);
 add (tsFITFN, rtExp, "fitexp", nullptr);
 add (tsFITFN, rtLg, "fitlog", nullptr);
 add (tsFITFN, rtPow, "fitpow", nullptr);
 add (tsFITFN, rtInv, "fitinv", nullptr);

 add (tsCLCFN, rtPoly, "clcpoly", nullptr);
 add (tsCLCFN, rtExp, "clcexp", nullptr);
 add (tsCLCFN, rtLg, "clclog", nullptr);
 add (tsCLCFN, rtPow, "clcpow", nullptr);
 add (tsCLCFN, rtInv, "clcinv", nullptr);
 add (tsSTFUN, sfNum, "num", nullptr);
 add (tsSTFUN, sfMean, "mean", nullptr);
 add (tsSTFUN, sfMedian, "median", nullptr);
 add (tsSTFUN, sfRMS, "rms", nullptr);
 add (tsSTFUN, sfSumX, "sumx", nullptr);
 add (tsSTFUN, sfStdDevP, "stddevp", nullptr);
 add (tsSTFUN, sfStdDevS, "stddevs", nullptr);
 add (tsSTFUN, sfMin, "stmin", nullptr);
 add (tsSTFUN, sfMax, "stmax", nullptr);

 add (tsSTFUN, sfNormP, "normp", nullptr);
 add (tsSTFUN, sfNormQ, "normq", nullptr);
 add (tsSTFUN, sfNormR, "normr", nullptr);

 add (tsSTFUN, sfInvNorm, "invnorm", nullptr);
 add (tsSTFUN, sfNormPD, "normpd", nullptr);

 add (tsSFUNCS1, ssFdlg, "fdlg", nullptr, true);

 add (tsVFUNC1, vf_pol_rt, "polynom", (void *)vfunc);

 add (tsSFUNCF2, "const", (void *)Const);
 add (tsSFUNCF2, "var", (void *)Var);

 add (tsCIFUNC1, "prec", (void *)SetPrecision);

 add (tsVFUNC1, vf_abs, "abs", (void *)vfunc);
 add (tsVFUNC1, vf_pol, "pol", (void *)vfunc);
 add (tsVFUNC1, vf_pol, "arg", (void *)vfunc);

 add (tsVFUNC1, vf_sin, "sin", (void *)vfunc);
 add (tsVFUNC1, vf_cos, "cos", (void *)vfunc);
 add (tsVFUNC1, vf_tan, "tan", (void *)vfunc);
 add (tsVFUNC1, vf_tan, "tg", (void *)vfunc);
 add (tsVFUNC1, vf_cot, "cot", (void *)vfunc);
 add (tsVFUNC1, vf_cot, "ctg", (void *)vfunc);

 add (tsVFUNC1, vf_sinh, "sinh", (void *)vfunc);
 add (tsVFUNC1, vf_sinh, "sh", (void *)vfunc);
 add (tsVFUNC1, vf_cosh, "cosh", (void *)vfunc);
 add (tsVFUNC1, vf_cosh, "ch", (void *)vfunc);
 add (tsVFUNC1, vf_tanh, "tanh", (void *)vfunc);
 add (tsVFUNC1, vf_tanh, "th", (void *)vfunc);
 add (tsVFUNC1, vf_ctnh, "ctanh", (void *)vfunc);
 add (tsVFUNC1, vf_ctnh, "cth", (void *)vfunc);

 add (tsVFUNC1, vf_asin, "asin", (void *)vfunc);
 add (tsVFUNC1, vf_asin, "arcsin", (void *)vfunc);
 add (tsVFUNC1, vf_acos, "acos", (void *)vfunc);
 add (tsVFUNC1, vf_acos, "arccos", (void *)vfunc);
 add (tsVFUNC1, vf_atan, "atan", (void *)vfunc);
 add (tsVFUNC1, vf_atan, "arctg", (void *)vfunc);
 add (tsVFUNC1, vf_acot, "acot", (void *)vfunc);
 add (tsVFUNC1, vf_acot, "arcctg", (void *)vfunc);

 add (tsVFUNC1, vf_asinh, "asinh", (void *)vfunc);
 add (tsVFUNC1, vf_asinh, "arsh", (void *)vfunc);
 add (tsVFUNC1, vf_acosh, "acosh", (void *)vfunc);
 add (tsVFUNC1, vf_acosh, "arch", (void *)vfunc);
 add (tsVFUNC1, vf_atanh, "atanh", (void *)vfunc);
 add (tsVFUNC1, vf_atanh, "arth", (void *)vfunc);
 add (tsVFUNC1, vf_acoth, "acoth", (void *)vfunc);
 add (tsVFUNC1, vf_acoth, "arcth", (void *)vfunc);

 add (tsVFUNC1, vf_exp, "exp", (void *)vfunc);
 add (tsVFUNC1, vf_exp10, "exp10", (void *)vfunc);
 add (tsVFUNC1, vf_log, "log", (void *)vfunc);
 add (tsVFUNC1, vf_log, "ln", (void *)vfunc);
 add (tsVFUNC1, vf_log10, "log10", (void *)vfunc);
 add (tsVFUNC1, vf_log10, "lg", (void *)vfunc);
 add (tsVFUNC1, vf_log2, "log2", (void *)vfunc);
 add (tsVFUNC1, vf_sqrt, "sqrt", (void *)vfunc);
 add (tsVFUNC1, vf_sqrt, "root2", (void *)vfunc);
 add (tsVFUNC1, vf_root3, "root3", (void *)vfunc);
 add (tsVFUNC1, vf_root3, "cbrt", (void *)vfunc);
 add (tsVFUNC1, vf_rand, "rnd", (void *)vfunc);
 add (tsVFUNC1, vf_rand, "rand", (void *)vfunc);
 add (tsVFUNC1, vf_round, "round", (void *)vfunc);
 add (tsVFUNC1, vf_ceil, "ceil", (void *)vfunc);
 add (tsVFUNC1, vf_floor, "floor", (void *)vfunc);
 add (tsVFUNC1, vf_frac, "frac", (void *)vfunc);
 add (tsVFUNC1, vf_int, "int", (void *)vfunc);
 add (tsVFUNC1, vf_float, "float", (void *)vfunc);

 add (tsVFUNC1, vf_db, "db", (void *)vfunc);
 add (tsVFUNC1, vf_np, "np", (void *)vfunc);
 add (tsVFUNC1, vf_adb, "adb", (void *)vfunc);
 add (tsVFUNC1, vf_anp, "anp", (void *)vfunc);

 add (tsVFUNC2, vf_pow, "pow", (void *)vfunc2);
 add (tsVFUNC2, vf_rootn, "rootn", (void *)vfunc2);
 add (tsVFUNC2, vf_logn, "logn", (void *)vfunc2);

 add (tsVFUNC2, vf_cplx, "cmplx", (void *)vfunc2);
 add (tsVFUNC2, vf_cplx, "cplx", (void *)vfunc2);
 add (tsVFUNC2, vf_cplx, "cpx", (void *)vfunc2);
 add (tsVFUNC2, vf_polar, "polar", (void *)vfunc2);
 add (tsVFUNC2, vf_polar, "rect", (void *)vfunc2);
 add (tsVFUNC1, vf_re, "re", (void *)vfunc);
 add (tsVFUNC1, vf_im, "im", (void *)vfunc);
 add (tsVFUNC1, vf_conj, "conj", (void *)vfunc);
 add (tsVFUNC1, vf_factorial, "fact", (void *)vfunc);
 add (tsVFUNC2, vf_hypot, "hypot", (void *)vfunc2);
 add (tsVFUNC2, vf_atan2, "atan2", (void *)vfunc2);
 add (tsVFUNC2, vf_fmod, "fmod", (void *)vfunc2);
 add (tsVFUNC2, vf_fmod, "mod", (void *)vfunc2);

 add (tsFFUNC1, "erf", (void *)(float__t (*) (float__t))Erf);
 add (tsFFUNC1, "aerf", (void *)(float__t (*) (float__t))Erfinv);
 add (tsFFUNC1, "erfc", (void *)(float__t (*) (float__t))Erfc);
 
 add (tsIFUNC2, "gcd", (void *)(int_t (*) (int_t, int_t))Gcd);
 add (tsIFUNC2, "invmod", (void *)(int_t (*) (int_t, int_t))Invmod);

 add (tsIFUNC1, "prime", (void *)Prime);
 add (tsIFUNC1, "tick", (void *)Tick);

 add (tsSFUNCI1, "factorize", (void *)factorize_p);

 add (tsPFUNCn, "fprn", (void *)(int_t (*) (char *, char *, int args, char, value *))fprn);
 add (tsPFUNCn, "prn", (void *)(int_t (*) (char *, char *, int args, char, value *))fprn);
 add (tsPFUNCn, "printf", (void *)(int_t (*) (char *, char *, int args, char, value *))fprn);
 add (tsFPFUNCn, "prnf", (void *)(int_t (*) (char *, char *, int args, char, value *))fprnf, true);
  
 add (tsSIFUNC1, "datatime", (void *)datatime);
 
 add (tsFFUNC1, "sing", (void *)(float__t (*) (float__t))Sing);
 add (tsFFUNC1, "cosg", (void *)(float__t (*) (float__t))Cosg);
 add (tsFFUNC1, "tgg", (void *)(float__t (*) (float__t))Tgg);
 add (tsFFUNC1, "ctgg", (void *)(float__t (*) (float__t))Ctgg);

 add (tsIFUNC1, "not", (void *)Not);
 add (tsIFUNC1, "now", (void *)Now);
 add (tsFFUNC2, "min", (void *)(float__t (*) (float__t, float__t))Min);
 add (tsFFUNC2, "max", (void *)(float__t (*) (float__t, float__t))Max);
 add (tsFFUNC1, "frh", (void *)(float__t (*) (float__t))Farenheit);

 add (tsFFUNC1, "swg", (void *)(float__t (*) (float__t))Swg);
 add (tsFFUNC1, "sswg", (void *)(float__t (*) (float__t))SSwg);
 add (tsFFUNC1, "aswg", (void *)(float__t (*) (float__t))Aswg);
 add (tsFFUNC1, "awg", (void *)(float__t (*) (float__t))Awg);
 add (tsFFUNC1, "sawg", (void *)(float__t (*) (float__t))SAwg);
 add (tsFFUNC1, "aawg", (void *)(float__t (*) (float__t))Aawg);
 add (tsFFUNC1, "cs", (void *)(float__t (*) (float__t))Cs);
 add (tsFFUNC1, "acs", (void *)(float__t (*) (float__t))Acs);
 add (tsFFUNC3, "vout", (void *)(float__t (*) (float__t, float__t, float__t))Vout);
 add (tsFFUNC3, "cmp", (void *)(float__t (*) (float__t, float__t, float__t))Cmp);
 add (tsFFUNC2, "ee", (void *)(float__t (*) (float__t, float__t))Ee);

 add (tsIFUNCF1, "wrgb", (void *)wavelength_to_rgb);
 add (tsIFUNCF1, "trgb", (void *)temperature_to_rgb);
 add (tsSFUNCF1, "winf", (void *)wavelength_info);

 add (tsIFUNCF1, "binf", (void *)binf);
 add (tsIFUNCF1, "bind", (void *)bindbl);

 add (tsFFUNCI1, "floatf", (void *)floatf);
 add (tsFFUNCI1, "floatd", (void *)floatd);

 // Mathematical constants
 #ifdef __BORLANDC__
 addfconst ("pi", M_PId);
 addfconst ("e", M_Ed);
 addfconst ("phi", PHId);
 addfconst ("tau", (float__t)2.0L * M_PId);
 addfconst ("turn", (float__t)2.0L * M_PId);
 addfconst ("gon", M_PId / (float__t)200.0L);
 addfconst ("deg", M_PId / (float__t)180.0L);
 #else
 addfconst ("pi", M_PI);
 addfconst ("e", M_E);
 addfconst ("phi", PHI);
 addfconst ("tau", ((float__t)M_2PI));
 addfconst ("turn",((float__t)M_2PI));
 addfconst ("gon", (M_PI / (float__t)200.0L));
 addfconst ("deg", (M_PI / (float__t)180.0L));
 #endif
 // Other imperial constants
 // Distance and length
 addfconst ("inch", (float__t)0.0254);     // Inch (m)
 addfconst ("mil", (float__t)0.0000254);   // 1/1000Inch (m)
 addfconst ("ft", (float__t)0.3048);       // Foot (m)
 addfconst ("yd", (float__t)0.9144);       // Yard (m)
 addfconst ("foot", (float__t)0.3048);     // Foot (m)
 addfconst ("yard", (float__t)0.9144);     // Yard (m)
 addfconst ("mi", (float__t)1609.344);     // Mile (m)
 addfconst ("mile", (float__t)1609.344);   // Mile (m)
 addfconst ("nmi", (float__t)1852.0);      // Nautical mile
 addfconst ("ptt", (float__t)0.0254 / 72); // Point (m)
 // Mass
 addfconst ("lb", (float__t)0.45359237);     // Pound (kg)
 addfconst ("oz", (float__t)0.028349523125); // Ounce (kg)
 addfconst ("st", (float__t)6.35029318);     // Stone (kg)
 addfconst ("gr", (float__t)0.001);           // Gram (kg)
 addfconst ("kg", (float__t)1.0);            // Kilogram (kg)

 // Volume
 addfconst ("gal", (float__t)0.003785411784);     // US Gallon (m³)
 addfconst ("qt", (float__t)0.000946352946);      // US Quart (m³)
 addfconst ("pt", (float__t)0.000473176473);      // US Pint (m³)
 addfconst ("cup", (float__t)0.0002365882365);    // US Cup (m³)
 addfconst ("floz", (float__t)2.95735295625e-5);  // US Fluid Ounce (m³)
 addfconst ("tbsp", (float__t)1.478676478125e-5); // US Tablespoon (m³)
 addfconst ("tsp", (float__t)4.92892159375e-6);   // US Teaspoon (m³)
 addfconst ("lt", (float__t)0.001);                // liters (m³)
 addfconst ("ml", (float__t)0.000001);             // milliliters (m³)
 addfconst ("cc", (float__t)0.000001);             // milliliters (m³)
 // Energy
 addfconst ("cal", (float__t)4.184);           // Calorie (J)
 addfconst ("kcal", (float__t)4184.0);         // Kilocalorie (J)
 addfconst ("btu", (float__t)1055.05585262);   // British thermal unit (J)
 addfconst ("wh", (float__t)3600.0);           // Watt hour (J)
 addfconst ("kwh", (float__t)3600000.0);       // Kilowatt hour (J)
 addfconst ("mwh", (float__t)3600000000.0);    // Megawatt hour (J)
 addfconst ("gtnt", (float__t)4184);           // Gram of TNT (Joules)
 addfconst ("ttnt", (float__t)4.184e9);        // Tonne of TNT (Joules)
 addfconst ("ktnt", (float__t)4.184e12);       // Kiloton of TNT (Joules)
 addfconst ("mtnt", (float__t)4.184e15);       // Megaton of TNT (Joules)
 addfconst ("ev", (float__t)1.602176634e-19);  // Electronvolt (J)
 addfconst ("kev", (float__t)1.602176634e-16); // Kiloelectronvolt (J)
 addfconst ("mev", (float__t)1.602176634e-13); // Megaelectronvolt (J)
 addfconst ("gev", (float__t)1.602176634e-10); // Gigaelectronvolt (J)

 // Power
 addfconst ("hps", (float__t)745.69987158227022); // Horsepower (W)
 addfconst ("bhp", (float__t)745.69987158227022); // Brake horsepower (W)
 // Pressure
 addfconst ("atm", (float__t)101325.0);           // Standard atmosphere (Pa)
 addfconst ("bar", (float__t)100000.0);           // Bar (Pa)
 addfconst ("psi", (float__t)6894.757293168361); // Pound-force per square inch (Pa)
 // Speed
 addfconst ("kmh", (float__t)0.277777778);  // Kilometers per hour to meters per second
 addfconst ("mph", (float__t)0.44704);      // Miles per hour to meters per second
 addfconst ("knot", (float__t)0.514444444); // Nautical miles per hour to meters per second
 // Time
 addfconst ("hour", (float__t)3600); // Hour in seconds
 addfconst ("hr", (float__t)3600);   // Hour in seconds
 addfconst ("mnt", (float__t)60);    // Minute in seconds
 // Radiation units
 addfconst ("gy", (float__t)1.0);             // Gray (J/kg) - Base SI
 addfconst ("rad", (float__t)0.01);           // Rad (absorbed dose)
 addfconst ("sv", (float__t)1.0);             // Sievert (Equivalent dose) - Base SI
 addfconst ("rem", (float__t)0.01);           // Roentgen Equivalent Man (0.01 Sv)
 addfconst ("rn", (float__t)0.00877);         // Roentgen (approx in air)
 addfconst ("mrn", (float__t)0.00000877);     // Milli-roentgen
 addfconst ("urn", (float__t)0.00000000877);  // Micro-roentgen
 addfconst ("ngnt", (float__t)(3.6 * 0.00877)); // Not great, not terrible (3.6 R)
 // Magnetic units
 addfconst ("tl", (float__t)1.0);  // Magnetic flux density (or magnetic induction) Tesla - Base SI
 addfconst ("wb", (float__t)1.0);  // Magnetic flux Weber - Base SI
 addfconst ("gs", (float__t)1e-4); // Magnetic flux density (or magnetic induction) Gauss to Tesla
 addfconst ("mw", (float__t)1e-8); // Magnetic flux Maxwell to Weber
 addfconst ("oe", (float__t)(1000.0 / (4.0L * M_PI))); // Magnetic field strength (H) Oersted to A/m
 addfconst ("gb", (float__t)(10.0 / (4.0L * M_PI)));   // Magnetomotive force (MMF) Gilbert to Ampere-turn

 // Physical constants (CODATA 2018)
 // Fundamental constants
 addfconst ("c0", (float__t)299792458.0);          // Speed of light in vacuum (m/s)
 addfconst ("hp", (float__t)6.62607015e-34);       // Planck constant (J·s)
 addfconst ("hb", (float__t)1.054571817e-34);      // Reduced Planck constant ℏ (J·s)
 addfconst ("gn", (float__t)6.67430e-11);          // Gravitational constant (m³/(kg·s²))
 addfconst ("na", (float__t)6.02214076e23);        // Avogadro constant (mol⁻¹)
 addfconst ("kb", (float__t)1.380649e-23);         // Boltzmann constant (J/K)
 addfconst ("rg", (float__t)8.314462618);          // Universal gas constant (J/(mol·K))
 addfconst ("sf", (float__t)5.670374419e-8);       // Stefan-Boltzmann constant (W/(m²·K⁴))
 addfconst ("rs", (float__t)8.314462618);          // Ideal gas constant kb*Na (J/(mol·K))
 addfconst ("nae", (float__t)2.1798723611035e-18); // Rydberg energy (J)
 addfconst ("mu", (float__t)1.66053906660e-27);    // Atomic mass constant (kg)
 addfconst ("stdt", (float__t)273.15); // Standard temperature (K)

 // Electromagnetic constants
 addfconst ("e0", (float__t)8.8541878128e-12); // Electric constant, vacuum permittivity (F/m)
 addfconst ("u0", (float__t)1.25663706212e-6); // Magnetic constant, vacuum permeability (H/m)
 addfconst ("z0", (float__t)376.730313668);    // Characteristic impedance of vacuum (Ω)

 // Particle constants
 addfconst ("qe", (float__t)1.602176634e-19);   // Elementary charge (C)
 addfconst ("me", (float__t)9.1093837015e-31);  // Electron mass (kg)
 addfconst ("mp", (float__t)1.67262192369e-27); // Proton mass (kg)
 addfconst ("mn", (float__t)1.67492749804e-27); // Neutron mass (kg)
 addfconst ("rel", (float__t)2.8179403262e-15); // Classical electron radius (m)
 addfconst ("a0", (float__t)5.29177210903e-11); // Bohr radius (m)
 // Astronomical constants
 addfconst ("au", (float__t)1.495978707e11);     // Astronomical unit (m)
 addfconst ("ly", (float__t)9.4607304725808e15); // Light year (m)
 addfconst ("pc", (float__t)3.0856775814914e16); // Parsec (m)
 addfconst ("g0", (float__t)9.80665);             // Standard gravity (m/s²)

 // Additional constants
 addfconst ("ry", (float__t)10973731.568160); // Rydberg constant (m⁻¹)
 addfconst ("sb", (float__t)5.670374419e-8);  // Stefan-Boltzmann constant (W/(m²·K⁴))
 // Rainbow colors
 addfconst ("fir", (float__t)316e-6);
 addfconst ("lwir", (float__t)11.5e-6);
 addfconst ("mwir", (float__t)5.5e-6);
 addfconst ("swir", (float__t)2.2e-6);
 addfconst ("nir", (float__t)1.09e-6);
 addfconst ("red", (float__t)685e-9);
 addfconst ("orange", (float__t)605e-9);
 addfconst ("yellow", (float__t)580e-9);
 addfconst ("green", (float__t)532e-9);
 addfconst ("blue", (float__t)472e-9);
 addfconst ("indigo", (float__t)435e-9);
 addfconst ("violet", (float__t)400e-9);
 addfconst ("uva", (float__t)348e-9);
 addfconst ("uvb", (float__t)298e-9);
 addfconst ("uvc", (float__t)190e-9);

 // Integer Limits:
 addlconst ("max32", (float__t)2147483647.0L, 0x7fffffff);
 addlconst ("maxint", (float__t)2147483647.0L, 0x7fffffff);
 addlconst ("maxu32", (float__t)4294967295.0L, 0xffffffff);
 addlconst ("maxuint", (float__t)4294967295.0L, 0xffffffff);
 addlconst ("max64", (float__t)9223372036854775807.0L, 0x7fffffffffffffffull);
 addlconst ("maxlong", (float__t)9223372036854775807.0L, 0x7fffffffffffffffull);
 addlconst ("maxu64", (float__t)18446744073709551615.0L, 0xffffffffffffffffull);
 addlconst ("maxulong", (float__t)18446744073709551615.0L, 0xffffffffffffffffull);

 addlconst ("fp_sz", sizeof (float__t)*8, sizeof (float__t)*8);
 addlconst ("int_sz", sizeof (int_t)*8, sizeof (int_t)*8);

 addlconst ("fp_dig", LDBL_DIG, LDBL_DIG);             // significant decimal digits
 addlconst ("fp_mant_dig", LDBL_MANT_DIG, LDBL_MANT_DIG);   // bits in the mantissa
 addlconst ("fp_max_exp", LDBL_MAX_EXP, LDBL_MAX_EXP);     // maximum exponent (base 2)
 addlconst ("fp_max_10_exp", LDBL_MAX_10_EXP, LDBL_MAX_10_EXP); // maximum exponent (base 10)

 // System
 //  Get system timezone information
 TIME_ZONE_INFORMATION tzi;
 DWORD tzResult    = GetTimeZoneInformation (&tzi);
 long timezoneBias = tzi.Bias; // in minutes
 int daylight      = (tzResult == TIME_ZONE_ID_DAYLIGHT) ? 1 : 0;
 double tzHours    = -timezoneBias / 60.0;
 double currentTz  = tzHours + (daylight ? -tzi.DaylightBias / 60.0 : 0);

 addlconst ("timezone", (float__t)tzHours, (int)tzHours);
 addlconst ("daylight", (float__t)daylight, daylight);
 addlconst ("tz", (float__t)currentTz, (int)currentTz);
 addfconst ("version", (float__t)_ver_);
 addim (); // Add imaginary unit 'i', 'j' as a predefined constant

 addsvar ("path", ""); // Add a string variable to hold the file path for file operations)
 addivar ("plot_width", 800); // Default plot width in pixels)
 addivar ("plot_height", 600); // Default plot height in pixels)
 addivar ("plot_bgc", 0x00FFFFFF); // Default plot background color (white)
 addivar ("plot_fgc", 0x00000000); // Default plot foreground color (black)
}
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
         if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvSTR))
          {
           sf_free (sp->val.sval); // Free string value using sf_free to ensure it's unregistered
           sp->val.sval = nullptr;
          }
         if ((sp->tag == tsVARIABLE) && (sp->val.tag == tvMATRIX))
          {
           sf_free (sp->val.mval); // Free matrix value using sf_free to ensure it's unregistered
           sp->val.mval = nullptr;
          }
         if (sp->tag == tsUFUNCT && sp->func) 
          {
           free (sp->func); // Free function name using sf_free to ensure it's unregistered
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
    if (dup) register_mem (dup); // Register duplicated string for cleanup
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
       register_mem (new_mval); // Register the new matrix for cleanup
       memcpy (new_mval, val.mval, msize);
       return new_mval;
      }
    }
   return nullptr;
  }
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
           char mstr[256];
           mxprint (sp->val.mrows, sp->val.mcols, sp->val.mval, mstr, false);
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %s\r\n", sp->name, mstr);

          }
         else
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %-.5g\r\n", sp->name,
                               (double)sp->val.fval);
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
   max = 0;
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
 double res  = 0;
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
 const double dms[]   = 
  { (60.0L * 60.0 * 60.0 * 24.0 * 365.25 * 100.0),
    (60.0L * 60.0 * 24.0 * 365.25),
    (60.0L * 60.0 * 24.0 * 7),
    (60.0L * 60.0 * 24.0),
    (60.0L * 60.0),
     60.0L,
     1.0L };
 const char cdt[]     = { 'c', 'y', 'w', 'd', 'h', 'm', 's' };
 double res           = 0;
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
 else
 if ((*fpos == 'i') && (*(fpos + 1) == 'B'))
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
   else engineering (1e30, fpos, fval);
   break;
  case 'R':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.237940039285380274899124224e+27L; // 2**90
   else engineering (1e27, fpos, fval);
   break;
  case 'Y':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.208925819614629174706176e+24L; // 2**80
   else engineering (1e24, fpos, fval);
   break;
  case 'Z':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.180591620717411303424e+21L; // 2**70
   else engineering (1e21, fpos, fval);
   break;
  case 'E':
   fpos++;
   if (isCMP (fpos)) fval *= 1152921504606846976ull; // 2**60
   else engineering (1e18, fpos, fval);
   break;
  case 'P':
   fpos++;
   if (isCMP (fpos)) fval *= 1125899906842624ull; // 2**50
   else engineering (1e15, fpos, fval);
   break;
  case 'T':
   fpos++;
   if (isCMP (fpos)) fval *= 1099511627776ull; // 2**40
   else engineering (1e12, fpos, fval);
   break;
  case 'G':
   fpos++;
   if (isCMP (fpos)) fval *= 1073741824ull; // 2**30
   else engineering (1e9, fpos, fval);
   break;
  case 'M':
   fpos++;
   if (isCMP (fpos)) fval *= 1048576; // 2**20
   else engineering (1e6, fpos, fval);
   break;
  case 'K':
#ifdef _KELVIN_
   fpos++;
   if (isCMP (fpos))  fval *= 1024; // 2**10
   else 
   if ((scfg & FRH) == 0) engineering (1e3, fpos, fval);
   else fpos--;
#else
   fpos++;
   if (isCMP (fpos)) fval *= 1024; // 2**10
   else engineering (1e3, fpos, fval);
#endif //_KELVIN_
   break;
  //case 'R':
  // fpos++;
  // engineering (1, fpos, fval);
  // break;
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

#pragma region ERROR_HANDLING
// Set an error message with the given position and message text
void calculator::error (int pos, const char *msg, terr errt)
{
 sprintf (err, "Error: %s at %i", msg, pos);
 errpos = pos;
 errtype = errt;
}

void calculator::errorf (int pos, const char *fmt, ...)
{
 va_list args;
 va_start(args, fmt);
 vsprintf(err, fmt, args);
 va_end(args);
 errpos = pos;
}

void calculator::mxerror (const char *msg)
{
 strcpy (mxerr, msg);
}

#pragma endregion
//---------------------------------------------------------------------------

#pragma region Solvers and Plotting helpers
// Split expression to comma-separated parts using variadic arguments
// Usage: Split(expr, buf1, size1, buf2, size2, ..., nullptr, 0)
// Each buffer is specified as: char* buffer, int max_size
// Terminate the list with nullptr, 0
bool calculator::Split (const char *expr, ...)
{
 if (!expr || !*expr) return false;

 va_list args;
 va_start (args, expr);

 // Collect all buffer pointers and sizes
 struct BufferInfo
 {
  char *buf;
  int size;
 };
 BufferInfo buffers[16]; // max 16 parts (should be enough)
 int buf_count = 0;

 while (buf_count < 16)
  {
   char *buf = va_arg (args, char *);
   int size  = va_arg (args, int);

   if (!buf || size <= 0) break; // terminator: nullptr, 0

   buffers[buf_count].buf  = buf;
   buffers[buf_count].size = size;
   buf[0]                  = '\0'; // initialize to empty
   buf_count++;
  }

 va_end (args);

 if (buf_count == 0) return false; // no buffers provided

 const char *p    = expr;
 int depth        = 0; // depth of nested parentheses and brackets
 int idx          = 0;
 int current_part = 0;

 // Skip leading whitespace
 while (*p && isspace ((unsigned char)*p & 0x7f)) p++;

 while (*p)
  {
   char ch = *p;

   // Track bracket/parenthesis depth
   if (ch == '(' || ch == '[')
    {
     depth++;
    }
   else if (ch == ')' || ch == ']')
    {
     depth--;
     if (depth < 0)
      {
       error (p - expr, "Unmatched closing bracket");
       return false;
      }
    }
   // Process comma only at depth 0 (outside of all brackets)
   else if (ch == ',' && depth == 0)
    {
     // Terminate current part
     if (current_part < buf_count)
      {
       buffers[current_part].buf[idx] = '\0';
      }

     current_part++;
     if (current_part >= buf_count)
      {
       error (p - expr, "Too many arguments");
       return false;
      }

     idx = 0;
     p++;
     // Skip whitespace after comma
     while (*p && isspace ((unsigned char)*p & 0x7f)) p++;
     continue;
    }

   // Copy character to current buffer
   if (current_part < buf_count)
    {
     if (idx < buffers[current_part].size - 1)
      {
       buffers[current_part].buf[idx++] = ch;
      }
     else
      {
       error (p - expr, "Buffer overflow: part too long");
       return false;
      }
    }

   p++;
  }

 // Terminate the last part
 if (current_part < buf_count)
  {
   buffers[current_part].buf[idx] = '\0';
  }

 if (depth != 0)
  {
   error (0, "Unmatched opening bracket");
   return false;
  }

 if (current_part < buf_count - 1)
  {
   error (0, "Not enough arguments");
   return false;
  }

 return true;
}
// Split expression to the comma separeted parts.

// Function to normalize a path
void calculator::NormalizePath (const char *input, char *output, int outSize)
{
 if (!input || !output || outSize <= 0) return;
 const char *defaultPath = getsvar ("path");
 char tempBuffer[MAX_PATH];

 // 1. Check if the input is just a file name (no slashes)
 bool isJustFileName = (strchr (input, '\\') == NULL && strchr (input, '/') == NULL);

 if (isJustFileName && defaultPath && strlen (defaultPath) > 0)
  {
   // Copy the base path
   strncpy (tempBuffer, defaultPath, MAX_PATH - 1);
   tempBuffer[MAX_PATH - 1] = '\0';

   // Add a slash if it's not present at the end of defaultPath
   int len = (int)strlen (tempBuffer);
   if (len > 0 && tempBuffer[len - 1] != '\\' && tempBuffer[len - 1] != '/')
    {
     strncat (tempBuffer, "\\", MAX_PATH - len - 1);
    }

   // Add the file name
   strncat (tempBuffer, input, MAX_PATH - strlen (tempBuffer) - 1);
  }
 else
  {
   strncpy (tempBuffer, input, MAX_PATH - 1);
   tempBuffer[MAX_PATH - 1] = '\0';
  }

 // 2. Expand environment variables (%ENVVAR%)
 // ExpandEnvironmentStringsA works well in WinAPI
 char expanded[MAX_PATH];
 if (ExpandEnvironmentStringsA (tempBuffer, expanded, MAX_PATH) > 0)
  {
   // 3. Replace all '/' with '\' for consistency
   for (int i = 0; expanded[i] != '\0'; i++)
    {
     if (expanded[i] == '/') expanded[i] = '\\';
    }

   // Copy result to the output buffer
   strncpy (output, expanded, outSize - 1);
   output[outSize - 1] = '\0';
  }
 else
  {
   // If ExpandEnvironmentStringsA failed, return what we have in tempBuffer
   strncpy (output, tempBuffer, outSize - 1);
   output[outSize - 1] = '\0';
  }
}

int calculator::scanmasknum (const char *str)
{
 const char *cp = str;
 int num        = 0;
 while (*cp)
  {
   if (*cp >= '0' && *cp <= '9') num++;
   cp++;
  }
 return num;
}

int calculator::strscan (const char *str, const char *msk, int n, double *v, ...)
{
 const char *cp = str;
 const char *mp = msk;
 int num_count  = 0;
 double **vals  = &v;

 while (*cp && (num_count < n))
  {
   switch (*cp)
    {
    case '-':
    case '+':
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
     {
      char *fpos;
      double val = strtod (cp, &fpos);
      scientific (fpos, val);
      cp = fpos;
      if (mp && *mp) // if mask is provided, check if current character matches the mask
       {
        int idx = -1;
        while (*mp && isspace (*mp)) mp++;             // skip spaces in mask
        if (*mp >= '0' && *mp <= '9') idx = *mp - '0'; // get index from mask
        if (*mp) mp++;
        if (idx >= 0 && idx < n && vals[idx])
         {
          *vals[idx] = val; // store value in the correct variable based on mask
          num_count++;
         }
       }
      else // if no mask, capture all numbers
       {
        if (vals[num_count]) *vals[num_count++] = val;
       }
     }
     break;
    }
   cp++;
  }
 return num_count;
}


t_br_result calculator::check_break (uint64_t init_ms, uint64_t last_gui_check)
{
#ifdef NDEBUG
 uint64_t current_ms = GetTickCount64 ();

 if (current_ms - init_ms > 1000)
  {
   if (!EscFn)
    {
     errorf (pos, "Operation took too long");
     return brTIMEOUT;
    }
   if (EscFn && EscFn ())
    {
     errorf (pos, "Operation cancelled by user");
     return brESC;
    }
   else
    {
     if (current_ms - last_gui_check > 100)
      {
       last_gui_check = current_ms;
       Sleep (1); // Sleep briefly to allow GUI to remain responsive
      }
     if (current_ms - init_ms > TIMEOUT) // 10 second time limit for summation
      {
       errorf (pos, "Operation took too long");
       return brTIMEOUT;
      }
    }
  }
#endif // NDEBUG
 return brNONE;
}
#pragma endregion
//---------------------------------------------------------------------------

#pragma region GRAPHICS_ENGINE
bool calculator::CheckChildRes (calculator *child)
{
 if (child->err[0])
  {
   errorf (pos, "%s", child->err);
   return false;
  }
 if (child->get_res_tag () == tvMATRIX)
  {
   errorf (pos, "Result is a matrix, expected a scalar");
   return false;
  }
 if (child->get_res_tag () == tvSTR)
  {
   errorf (pos, "Result is a string, expected a scalar");
   return false;
  }
 return true;
}

bool calculator::isChildResReal (calculator *child)
{
 float__t re = child->get_re_res ();
 float__t im = child->get_im_res ();
 if (isnan (re) || Abs (im) / AbsC (re, im) > (float__t)1e-12L)
  {
   return false; // treat very small real part with large relative imaginary part as non-real
  }
 return true;
}


void unquote_string (char *str)
{
 if (!str) return;
 char *src = str, *dst = str;
 while (*src)
  {
   if (*src == '"' || *src == '\'') // skip quotes
    {
     src++;
     continue;
    }
   *dst++ = *src++;
  }
 *dst = '\0';
}

// Prepare data for plotting (common part)
bool calculator::PlotPrepare (const char *expr, v_func fidx, char *fname, PlotParams &params)
{
 if (!expr || !*expr)
  {
   errorf (pos, "empty expression");
   return false;
  }

 char sexpr[STRBUF];
 char sexpr_y[STRBUF];
 char sfrom[MAXOP];
 char sto[MAXOP];
 char svar[STRBUF];
 char sz0[MAXOP];

 bool split_ok = false;
 
 // Determine which split pattern to use
 switch (fidx)
 {
  case pl_plot:     // expr, from, to, var
  case pl_plotpol:
  case pl_plotlgx:
  case pl_plotlgy:
  case pl_plotlgxy:
  case pl_plotsmith:
   if (!ShowImageFn) return false; // ShowImageFn must be set for non-file plotting (NULL for CLI)
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
   break;

  case pl_xyplot:  // x_expr, y_expr, from, to, var
   if (!ShowImageFn) return false; // ShowImageFn must be set for non-file plotting (NULL for CLI)
   split_ok = Split (expr, sexpr, STRBUF, sexpr_y, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF,
                     nullptr, 0);
  break;

  case pl_fplot:    // fname, expr, from, to, var
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
  case pl_fplotlgx:
  case pl_oplotlgx:
  case pl_fplotlgy:
  case pl_oplotlgy:
  case pl_fplotlgxy:
  case pl_oplotlgxy:
  case pl_fplotsmith:
  case pl_oplotsmith:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, 
                     sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0);
  break;

  case pl_fxyplot: // fname, x_expr, y_expr, from, to, var
  case pl_oxyplot:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sexpr_y, STRBUF, sfrom, MAXOP, sto, MAXOP,
                     svar, STRBUF, nullptr, 0);
  break;

  case pl_plotsmithz: // expr, from, to, var, z0
   split_ok = Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, sz0, MAXOP,
                     nullptr, 0);
   break;

  case pl_fplotsmithz: // file, expr, from, to, var, z0
  case pl_oplotsmithz:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF,
                     sz0, MAXOP, nullptr, 0);
   break;

  case pl_plotdata: // datafile, mask
  case pl_plotdatal:
   if (!ShowImageFn) return false;
   split_ok = Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0);    // try get mask
   if (!split_ok) split_ok = Split (expr, sexpr, STRBUF, nullptr, 0);    // get datafile
   //if (split_ok) Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0); //try get mask
  break;

  case pl_fplotdata: // bmpfile, datafile, mask
  case pl_oplotdata:
  case pl_fplotdatal:
  case pl_oplotdatal:
   split_ok = Split (expr, fname, STRBUF, sexpr, STRBUF, nullptr, 0); //get bmp and datafile
   if (split_ok) Split (expr, fname, STRBUF, sexpr, STRBUF, svar, STRBUF, nullptr, 0); //try get mask
  break;

  default:
   errorf (pos, "Unknown plot function");
   return false;
  }
 
 if (!split_ok)
  {
   result_fval = qnan;
   return false;
  }

 calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   result_fval = qnan;
   return false;
  }

// Process filename for file-based functions
 switch (fidx)
 {
  case pl_fplot:
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
  case pl_fxyplot:     
  case pl_oxyplot: 
  case pl_fplotlgx:
  case pl_oplotlgx:
  case pl_fplotlgy:
  case pl_oplotlgy:
  case pl_fplotlgxy:
  case pl_oplotlgxy:
  case pl_fplotsmith: 
  case pl_oplotsmith: 
  case pl_fplotsmithz:
  case pl_oplotsmithz: 
  case pl_fplotdata: // bmpfile, datafile, mask
  case pl_oplotdata:
  case pl_fplotdatal:
  case pl_oplotdatal:
   child->setFileDlgFn (FileDlgFn);
   child->evaluate_f (fname);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
   if (child->get_res_tag () != tvSTR)
    {
     errorf (pos, "First argument is not a file name");
     delete child;
     result_fval = qnan;
     return false;
    }
   strncpy (fname, child->get_str_res (), STRBUF - 1);
   fname[STRBUF - 1] = '\0';
  break;
  }

 if (fidx < pl_plotdata)
  {
   // Evaluate from/to parameters
   float__t vfrom = child->evaluate_f (sfrom);
   if (isnan (vfrom) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }

   float__t vto = child->evaluate_f (sto);
   if (isnan (vto) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }

   if (vfrom > vto)
    {
     float__t tmp = vfrom;
     vfrom        = vto;
     vto          = tmp;
    }

   // NEW: Check for zero range (prevent infinite loops)
   if (Abs (vto - vfrom) < 1e-10)
    {
     errorf (pos, "Invalid range: from and to values must be different");
     delete child;
     result_fval = qnan;
     return false;
    }
   // Evaluate Z0 for Smith chart with explicit Z0
   float__t z0 = 50.0; // Default
   if (fidx == pl_plotsmithz || fidx == pl_fplotsmithz || fidx == pl_oplotsmithz)
    {
     z0 = child->evaluate_f (sz0);
     if (isnan (z0) || child->err[0] || !CheckChildRes (child) || !isChildResReal (child)
         || z0 <= 0)
      {
       errorf (pos, "Invalid Z0 (must be positive real number)");
       delete child;
       result_fval = qnan;
       return false;
      }
    }

   // Test evaluate first expression
   child->addfvar (svar, vfrom);
   float__t fvx = child->evaluate_f (sexpr);

   if (!(isnan (fvx) && child->errt () == teMath))
    {
     if (isnan (fvx) || child->err[0] || !CheckChildRes (child))
      {
       errorf (pos, "%s", child->err);
       delete child;
       result_fval = qnan;
       return false;
      }
    }
   else fvx = 0;

   // For parametric plots, test second expression
   if (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot)
    {
     float__t fvy = child->evaluate_f (sexpr_y);
     if (!(isnan (fvy) && child->errt () == teMath))
      {
       if (isnan (fvy) || child->err[0] || !CheckChildRes (child))
        {
         errorf (pos, "%s", child->err);
         delete child;
         result_fval = qnan;
         return false;
        }
      }
     else fvx = 0;
    }

   // Fill in the parameters
   params.sexpr   = strdup (sexpr);
   params.sexpr_y = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot)
                        ? strdup (sexpr_y)
                        : nullptr;
   params.svar    = strdup (svar);
   params.vfrom   = vfrom;
   params.vto     = vto;
   params.ymin    = fvx;
   params.ymax    = fvx;

   params.width = getivar ("plot_width");
   if ((params.width <= 100) || (params.width > 2000)) params.width = 800;

   params.height = getivar ("plot_height");
   if ((params.height <= 100) || (params.height > 2000)) params.height = 600;

   params.pxsize = getivar ("plot_dotsz");
   if (params.pxsize < 1 || params.pxsize > 4) params.pxsize = 4;

   params.dot = false;

   params.bgc     = getivar ("plot_bgc");
   params.fgc     = getivar ("plot_fgc");
   params.padding = 40;

   params.grid_color = 0xC0C0C0;
   params.axis_color = 0x808080;
   params.text_color = ~params.bgc;

   // Set logarithmic flags
   params.log_x = (fidx == pl_plotlgx || fidx == pl_fplotlgx || fidx == pl_oplotlgx
                   || fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);
   params.log_y = (fidx == pl_plotlgy || fidx == pl_fplotlgy || fidx == pl_oplotlgy
                   || fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);

   // Set Z0 for Smith chart
   params.z0 = z0;

   params.child = child;
  }
 else
  {//for plotdata

   child->setFileDlgFn (FileDlgFn);
   child->evaluate_f (sexpr);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
   if (child->get_res_tag () != tvSTR)
    {
     errorf (pos, "First argument is not a file name");
     delete child;
     result_fval = qnan;
     return false;
    }
   strncpy (sexpr, child->get_str_res (), STRBUF - 1);
   sexpr[STRBUF - 1] = '\0';

   unquote_string (svar); // Remove quotes from mask if present
   // Fill in the parameters
   params.sexpr   = strdup (sexpr);
   params.sexpr_y = nullptr;
   params.svar    = strdup (svar);
   params.vfrom   = 0;
   params.vto     = 0;
   params.ymin    = 0;
   params.ymax    = 0;

   params.width = getivar ("plot_width");
   if ((params.width <= 100) || (params.width > 2000)) params.width = 800;

   params.height = getivar ("plot_height");
   if ((params.height <= 100) || (params.height > 2000)) params.height = 600;

   params.pxsize = getivar ("plot_dotsz");
   if (params.pxsize < 1 || params.pxsize > 4) params.pxsize = 4;

   params.dot = (fidx == pl_plotdata || fidx == pl_fplotdata || fidx == pl_oplotdata);

   params.bgc     = getivar ("plot_bgc");
   params.fgc     = getivar ("plot_fgc");
   params.padding = 40;

   params.grid_color = 0xC0C0C0;
   params.axis_color = 0x808080;
   params.text_color = ~params.bgc;

   // Set logarithmic flags
   params.log_x = false;
   params.log_y = false;

   // Set Z0 for Smith chart
   params.z0 = 0;

   params.child = child;

  }
 return true;
}

// Plot Polar graph (specific part)
bool calculator::PlotPolar (bmpdraw *bmp, PlotParams &params)
{
 calculator *child   = params.child;
 float__t angle_from = params.vfrom;
 float__t angle_to   = params.vto;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 // Center of polar plot is always in the middle
 int center_x = width / 2;
 int center_y = height / 2;

 // Calculate appropriate step for smooth circle
 int available_size     = (width < height ? width : height) - 2 * padding;
 float__t circumference = 2.0 * M_PI * (available_size / 2);
 float__t step          = (angle_to - angle_from) / (circumference / 4.0); // ~4 pixels per step

 float__t rmax           = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find maximum radius
 float__t angle = angle_from;
 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    {
     return false;
    }

   child->addfvar (params.svar, angle);
   float__t r = child->evaluate_f (params.sexpr);

   if (!isnan (r) && isChildResReal (child))
    {
     float__t abs_r = Abs (r);
     if (first_point || abs_r > rmax)
      {
       rmax        = abs_r;
       first_point = false;
      }
    }
   angle += step;
  }
 while (angle <= angle_to);

 if (rmax < 1e-10) rmax = 1.0;

 // Add 10% padding to radius
 rmax *= 1.1;

 // Calculate scale: how many pixels per unit radius
 int plot_size  = (width < height ? width : height) - 2 * padding;
 float__t scale = plot_size / (2.0 * rmax);

 // Second pass: draw the curve
 angle                 = angle_from;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE)
    {
     return false;
    }

   child->addfvar (params.svar, angle);
   float__t r = child->evaluate_f (params.sexpr);

   if (!isnan (r) && isChildResReal (child))
    {
     // Convert polar to Cartesian
     float__t x_cart = r * Cos (angle);
     float__t y_cart = r * Sin (angle);

     // Convert to screen coordinates (centered, Y-axis inverted)
     int x_screen = center_x + (int)(x_cart * scale);
     int y_screen = center_y - (int)(y_cart * scale);

     if (has_valid_points)
      {
       bmp->lineTo (x_screen, y_screen, 2, fgc);
      }
     else
      {
       bmp->moveTo (x_screen, y_screen);
       has_valid_points = true;
      }
    }
   else
    {
     has_valid_points = false;
    }

   angle += step;
  }
 while (angle <= angle_to);

 // Store parameters for grid drawing
 params.ymin  = -rmax;
 params.ymax  = rmax;
 params.xmin  = -rmax;
 params.xmax  = rmax;
 params.scale = scale;

 return true;
}

// Draw polar grid and axes
void calculator::PlotDrawAxesPolar (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int center_x = width / 2;
 int center_y = height / 2;

 float__t rmax  = params.ymax; // Maximum radius in data units
 float__t scale = params.scale;

 // Draw radial grid (10 circles)
 for (int i = 1; i <= 10; i++)
  {
   float__t r_data = rmax * i / 10.0;
   int r_pixels    = (int)(r_data * scale);

   // Draw circle
   for (int angle_deg = 0; angle_deg < 360; angle_deg += 2)
    {
     float__t angle_rad = angle_deg * M_PI / 180.0;
     int x              = center_x + (int)(r_pixels * Cos (angle_rad));
     int y              = center_y - (int)(r_pixels * Sin (angle_rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      {
       bmp->drawPixel (x, y, grid_color);
      }
    }

   // Draw radius label (only for outermost circle)
   if (i == 10)
    {
     char label[32];
     d2scistr (label, (double)r_data);
     bmp->drawString (center_x + r_pixels + 5, center_y - 40, label, text_color, 0, 1);
    }
  }

 // Draw angular grid (every 30 degrees)
 int max_r_pixels = (int)(rmax * scale);
 for (int angle_deg = 0; angle_deg < 360; angle_deg += 30)
  {
   float__t angle_rad = angle_deg * M_PI / 180.0;

   // Draw radial line
   for (int r = 0; r <= max_r_pixels; r += 4)
    {
     int x = center_x + (int)(r * Cos (angle_rad));
     int y = center_y - (int)(r * Sin (angle_rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      {
       bmp->drawPixel (x, y, grid_color);
      }
    }

   // Draw angle labels
   int label_r = max_r_pixels + 15;
   int label_x = center_x + (int)(label_r * Cos (angle_rad));
   int label_y = center_y - (int)(label_r * Sin (angle_rad));

   char angle_label[16];
   sprintf (angle_label, "%d°", angle_deg);
   bmp->drawString (label_x - 10, label_y - 5, angle_label, text_color, 0, 1);
  }

 // Draw main axes (0° and 90°)
 bmp->drawLine (center_x - max_r_pixels, center_y, center_x + max_r_pixels, center_y, 1,
                axis_color);
 bmp->drawLine (center_x, center_y - max_r_pixels, center_x, center_y + max_r_pixels, 1,
                axis_color);

 // Draw title
 char title[128];
 snprintf (title, sizeof (title), "r=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (5, 5, title, text_color, 0, 2);

 // Draw variable name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}

// Plot Cartesian graph (specific part)
bool calculator::PlotCartesian (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;
 float__t ymin     = params.ymin;
 float__t ymax     = params.ymax;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 float__t step           = (vto - vfrom) / ((width - 2 * padding) / 4);
 float__t save_vfrom     = vfrom;
 bool has_valid_points   = false;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find ymin/ymax
 for (int pass = 0; pass < 2; pass++)
  {
   do
    {
     if (check_break (init_ms, last_gui_check) != brNONE)
      {
       return false;
      }

     child->addfvar (params.svar, vfrom);
     float__t fvx = child->evaluate_f (params.sexpr);

     if (pass == 0)
      {
       if (!isnan (fvx) && isChildResReal (child))
        {
         if (fvx < ymin) ymin = fvx;
         if (fvx > ymax) ymax = fvx;
        }
      }
     else
      {
       if (!isnan (fvx) && isChildResReal (child))
        {
         float__t x = padding + ((vfrom - save_vfrom) / (vto - save_vfrom)) * (width - 2 * padding);
         float__t y = height - padding - ((fvx - ymin) / (ymax - ymin)) * (height - 2 * padding);
         if (has_valid_points)
          {
           bmp->lineTo ((int)x, (int)y, 2, fgc);
          }
         else
          {
           bmp->moveTo ((int)x, (int)y);
           has_valid_points = true;
          }
        }
       else
        has_valid_points = false;
      }
     vfrom += step;
    }
   while (vfrom <= vto);
   vfrom = save_vfrom;

   if (pass == 0)
    {
     // Include zero in the range
     if (ymin > 0.0) ymin = 0.0;
     if (ymax < 0.0) ymax = 0.0;

     // Add padding
     if (ymin == ymax)
      {
       ymin -= (float__t)1.0L;
       ymax += (float__t)1.0L;
      }
     else
      {
       float__t ypad = (ymax - ymin) * (float__t)0.1L;
       ymin -= ypad;
       ymax += ypad;
      }
    }
  }

 // Update parameters for drawing axes and grid
 params.ymin = ymin;
 params.ymax = ymax;

 return true;
}

// Draw axes and grid (Cartesian coordinates)
void calculator::PlotDrawAxesCartesian (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 float__t ymin       = params.ymin;
 float__t ymax       = params.ymax;
 float__t vfrom      = params.vfrom;
 float__t vto        = params.vto;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 float__t x_range = vto - vfrom;
 float__t y_range = ymax - ymin;

 int plot_width       = width - 2 * padding;
 int plot_height      = height - 2 * padding;
 int grid_step_pixels = (plot_width > plot_height ? plot_width : plot_height) / 10;

 int y_axis_pixel = height - padding - ((0.0 - ymin) / y_range) * plot_height;

 int x_axis_pixel = -1;
 if (vfrom <= 0.0 && vto >= 0.0)
  {
   x_axis_pixel = padding + ((0.0 - vfrom) / x_range) * plot_width;
  }
 
 // Horizontal grid lines
 for (int offset = 0; offset <= plot_height; offset += grid_step_pixels)
  {
   int y_up   = y_axis_pixel - offset;
   int y_down = y_axis_pixel + offset;

   if (y_up >= padding && y_up < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);

     // NEW: Label first division above zero
     if (offset == grid_step_pixels && y_up > padding + 15)
      {
       float__t y_val = ymin + ((height - padding - y_up) / (float__t)plot_height) * y_range;
       char grid_label[32];
       d2scistr (grid_label, (double)y_val);
       bmp->drawString (5, y_up - 5, grid_label, text_color, 0, 1);
      }
    }

   if (offset > 0 && y_down >= padding && y_down < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);

     // NEW: Label first division below zero
     if (offset == grid_step_pixels && y_down < height - padding - 15)
      {
       float__t y_val = ymin + ((height - padding - y_down) / (float__t)plot_height) * y_range;
       char grid_label[32];
       d2scistr (grid_label, (double)y_val);
       bmp->drawString (5, y_down - 5, grid_label, text_color, 0, 1);
      }
    }
  }

 // Vertical grid lines (when x_axis exists)
 if (x_axis_pixel >= 0)
  {
   for (int offset = 0; offset <= plot_width; offset += grid_step_pixels)
    {
     int x_left  = x_axis_pixel - offset;
     int x_right = x_axis_pixel + offset;

     if (x_left >= padding && x_left < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
      }

     if (offset > 0 && x_right >= padding && x_right < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);

       // NEW: Label first division right of zero
       if (offset == grid_step_pixels && x_right < width - padding - 30)
        {
         float__t x_val = vfrom + ((x_right - padding) / (float__t)plot_width) * (vto - vfrom);
         char grid_label[32];
         d2scistr (grid_label, (double)x_val);
         bmp->drawString (x_right - 15, height - padding + 5, grid_label, text_color, 0, 1);
        }
      }
    }
  }
 else
  {
   // No zero axis - grid every N pixels
   for (int x = padding; x < width - padding; x += grid_step_pixels)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x, y, grid_color);

     // NEW: Label first grid line
     if (x == padding + grid_step_pixels && x < width - padding - 30)
      {
       float__t x_val = vfrom + ((x - padding) / (float__t)plot_width) * (vto - vfrom);
       char grid_label[32];
       d2scistr (grid_label, (double)x_val);
       bmp->drawString (x - 15, height - padding + 5, grid_label, text_color, 0, 1);
      }
    }
  }
 // Axes
 if (x_axis_pixel >= 0)
  {
   bmp->drawLine (x_axis_pixel, padding, x_axis_pixel, height - padding, 1, axis_color);
  }
 bmp->drawLine (padding, y_axis_pixel, width - padding, y_axis_pixel, 1, axis_color);

 // Axis labels
 char label[64];

 d2scistr (label, (double)vfrom);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)vto);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 char title[128];
 snprintf (title, sizeof (title), "y=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (5, 5, title, text_color, 0, 2);
}

bool calculator::PlotParametric (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t t_from   = params.vfrom;
 float__t t_to     = params.vto;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 float__t step = (t_to - t_from) / ((width - 2 * padding) * 2);

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find bounding box
 float__t t = t_from;
 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, t);

   float__t x = child->evaluate_f (params.sexpr);
   if (!(isnan (x) && child->errt () == teMath))
    {
     if (isnan (x) || child->err[0] || !CheckChildRes (child))
      {
       t += step;
       continue;
      }
    }
   else
    x = 0;

   float__t y = child->evaluate_f (params.sexpr_y);
   if (!(isnan (y) && child->errt () == teMath))
    {
     if (isnan (y) || child->err[0] || !CheckChildRes (child))
      {
       t += step;
       continue;
      }
    }
   else
    y = 0;

   if (!isChildResReal (child))
    {
     t += step;
     continue;
    }

   if (first_point)
    {
     xmin = xmax = x;
     ymin = ymax = y;
     first_point = false;
    }
   else
    {
     if (x < xmin) xmin = x;
     if (x > xmax) xmax = x;
     if (y < ymin) ymin = y;
     if (y > ymax) ymax = y;
    }

   t += step;
  }
 while (t <= t_to);

 // Add padding
 if (xmin == xmax)
  {
   xmin -= 1.0;
   xmax += 1.0;
  }
 if (ymin == ymax)
  {
   ymin -= 1.0;
   ymax += 1.0;
  }

 float__t x_pad = (xmax - xmin) * 0.1;
 float__t y_pad = (ymax - ymin) * 0.1;
 xmin -= x_pad;
 xmax += x_pad;
 ymin -= y_pad;
 ymax += y_pad;

 // Calculate UNIFORM scale (like in polar plots)
 float__t x_range = xmax - xmin;
 float__t y_range = ymax - ymin;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;

 float__t scale_x = plot_width / x_range;
 float__t scale_y = plot_height / y_range;

 // Use smaller scale to fit both dimensions
 float__t scale = (scale_x < scale_y) ? scale_x : scale_y;

 // Center the plot
 int center_x = width / 2;
 int center_y = height / 2;

 float__t x_center = (xmin + xmax) / 2.0;
 float__t y_center = (ymin + ymax) / 2.0;

 // Second pass: draw the curve
 t                     = t_from;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, t);

   float__t x   = child->evaluate_f (params.sexpr);
   bool x_valid = true;
   if (isnan (x) && child->errt () == teMath)
    x = 0;
   else if (isnan (x) || !isChildResReal (child))
    x_valid = false;

   float__t y   = child->evaluate_f (params.sexpr_y);
   bool y_valid = true;
   if (isnan (y) && child->errt () == teMath)
    y = 0;
   else if (isnan (y) || !isChildResReal (child))
    y_valid = false;

   if (x_valid && y_valid)
    {
     // Use uniform scale and center
     int x_screen = center_x + (int)((x - x_center) * scale);
     int y_screen = center_y - (int)((y - y_center) * scale);

     if (has_valid_points)
      {
       bmp->lineTo (x_screen, y_screen, 2, fgc);
      }
     else
      {
       bmp->moveTo (x_screen, y_screen);
       has_valid_points = true;
      }
    }
   else
    {
     has_valid_points = false;
    }

   t += step;
  }
 while (t <= t_to);

 // Store parameters for axis drawing
 params.xmin  = xmin;
 params.xmax  = xmax;
 params.ymin  = ymin;
 params.ymax  = ymax;
 params.scale = scale;

 return true;
}

void calculator::PlotDrawAxesParametric (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 float__t ymin       = params.ymin;
 float__t ymax       = params.ymax;
 float__t xmin       = params.xmin;
 float__t xmax       = params.xmax;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int plot_width       = width - 2 * padding;
 int plot_height      = height - 2 * padding;
 int grid_step_pixels = (plot_width > plot_height ? plot_width : plot_height) / 10;

 int center_x = width / 2;
 int center_y = height / 2;

 float__t x_center = (xmin + xmax) / 2.0;
 float__t y_center = (ymin + ymax) / 2.0;
 float__t scale    = params.scale;

 // Calculate axis positions using same formula as point drawing
 int y_axis_pixel = center_x + (int)((0.0 - x_center) * scale); // X=0 position
 int x_axis_pixel = center_y - (int)((0.0 - y_center) * scale); // Y=0 position

 // FIX: Draw grid centered on data center (not canvas center)
 // Horizontal grid lines (centered on Y=0 axis, not canvas center)
 for (int offset = 0; offset <= plot_height / 2; offset += grid_step_pixels)
  {
   int y_up   = x_axis_pixel - offset;
   int y_down = x_axis_pixel + offset;

   if (y_up >= padding && y_up < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);
    }

   if (offset > 0 && y_down >= padding && y_down < height - padding)
    {
     for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);
    }
  }

 // Vertical grid lines (centered on X=0 axis, not canvas center)
 for (int offset = 0; offset <= plot_width / 2; offset += grid_step_pixels)
  {
   int x_left  = y_axis_pixel - offset;
   int x_right = y_axis_pixel + offset;

   if (x_left >= padding && x_left < width - padding)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
    }

   if (offset > 0 && x_right >= padding && x_right < width - padding)
    {
     for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);
    }
  }

 // Draw axes (at zero crossings)
 if (x_axis_pixel >= padding && x_axis_pixel < height - padding)
  {
   bmp->drawLine (padding, x_axis_pixel, width - padding, x_axis_pixel, 1, axis_color);
  }
 if (y_axis_pixel >= padding && y_axis_pixel < width - padding)
  {
   bmp->drawLine (y_axis_pixel, padding, y_axis_pixel, height - padding, 1, axis_color);
  }

 // Axis labels
 char label[64];

 d2scistr (label, (double)xmin);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)xmax);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 // Draw parameter name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 // Draw titles for both functions (on two lines or shortened)
 char title_x[128];
 char title_y[128];

 // Shorten expressions if too long
 int max_len = 30;
 if ((int)strlen (params.sexpr) > max_len)
  snprintf (title_x, sizeof (title_x), "x=%.27s...", params.sexpr);
 else
  snprintf (title_x, sizeof (title_x), "x=%s", params.sexpr);

 if ((int)strlen (params.sexpr_y) > max_len)
  snprintf (title_y, sizeof (title_y), "y=%.27s...", params.sexpr_y);
 else
  snprintf (title_y, sizeof (title_y), "y=%s", params.sexpr_y);

 title_x[sizeof (title_x) - 1] = '\0';
 title_y[sizeof (title_y) - 1] = '\0';

 // Draw on two lines
 bmp->drawString (5, 5, title_x, text_color, 0, 1);
 bmp->drawString (5, 17, title_y, text_color, 0, 1);
}

// PlotLogarithmic:
bool calculator::PlotLogarithmic (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;

 bool log_x = params.log_x;
 bool log_y = params.log_y;

 // Calculate step
 float__t step = (vto - vfrom) / ((width - 2 * padding) * 10);

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point        = true;
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // First pass: find min/max, skipping invalid values
 float__t x = vfrom;
 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, x);
   float__t y = child->evaluate_f (params.sexpr);

   // Skip NaN and complex results
   if (!(isnan (y) && child->errt () == teMath))
    {
     if (isnan (y) || !isChildResReal (child))
      {
       x += step;
       continue;
      }
    }
   else
    y = 0;

   // Skip invalid values for logarithmic axes
   if ((log_x && x <= 0) || (log_y && y <= 0))
    {
     x += step;
     continue;
    }

   if (first_point)
    {
     xmin = xmax = x;
     ymin = ymax = y;
     first_point = false;
    }
   else
    {
     if (x < xmin) xmin = x;
     if (x > xmax) xmax = x;
     if (y < ymin) ymin = y;
     if (y > ymax) ymax = y;
    }

   x += step;
  }
 while (x <= vto);

 if (first_point)
  {
   errorf (pos, "No valid points to plot");
   return false;
  }

 // Add padding (in linear or log space)
 if (log_x)
  {
   float__t log_range = Lg (xmax) - Lg (xmin);
   float__t pad       = log_range * 0.1;
   xmin               = Pow (10.0, Lg (xmin) - pad);
   xmax               = Pow (10.0, Lg (xmax) + pad);
  }
 else
  {
   if (xmin == xmax)
    {
     xmin -= 1.0;
     xmax += 1.0;
    }
   else
    {
     float__t x_pad = (xmax - xmin) * 0.1;
     xmin -= x_pad;
     xmax += x_pad;
    }
   // Include zero in range for linear axis
   if (xmin > 0.0) xmin = 0.0;
   if (xmax < 0.0) xmax = 0.0;
  }

 if (log_y)
  {
   float__t log_range = Lg (ymax) - Lg (ymin);
   float__t pad       = log_range * 0.1;
   ymin               = Pow (10.0, Lg (ymin) - pad);
   ymax               = Pow (10.0, Lg (ymax) + pad);
  }
 else
  {
   if (ymin == ymax)
    {
     ymin -= 1.0;
     ymax += 1.0;
    }
   else
    {
     float__t y_pad = (ymax - ymin) * 0.1;
     ymin -= y_pad;
     ymax += y_pad;
    }
   // Include zero in range for linear axis
   if (ymin > 0.0) ymin = 0.0;
   if (ymax < 0.0) ymax = 0.0;
  }

 // Second pass: draw the curve
 x                     = vfrom;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, x);
   float__t y = child->evaluate_f (params.sexpr);

   bool valid = true;
   if (isnan (y) && child->errt () == teMath)
    y = 0;
   else if (isnan (y) || !isChildResReal (child))
    valid = false;

   if ((log_x && x <= 0) || (log_y && y <= 0)) valid = false;

   if (valid)
    {
     int x_screen, y_screen;

     if (log_x)
      {
       float__t log_x_norm = (Lg (x) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
       x_screen            = padding + (int)(log_x_norm * (width - 2 * padding));
      }
     else
      {
       x_screen = padding + (int)(((x - xmin) / (xmax - xmin)) * (width - 2 * padding));
      }

     if (log_y)
      {
       float__t log_y_norm = (Lg (y) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
       y_screen            = height - padding - (int)(log_y_norm * (height - 2 * padding));
      }
     else
      {
       y_screen = height - padding - (int)(((y - ymin) / (ymax - ymin)) * (height - 2 * padding));
      }

     if (has_valid_points)
      {
       bmp->lineTo (x_screen, y_screen, 2, fgc);
      }
     else
      {
       bmp->moveTo (x_screen, y_screen);
       has_valid_points = true;
      }
    }
   else
    {
     has_valid_points = false;
    }

   x += step;
  }
 while (x <= vto);

 // Store parameters for axis drawing
 params.xmin = xmin;
 params.xmax = xmax;
 params.ymin = ymin;
 params.ymax = ymax;

 return true;
}

// PlotDrawAxesLog:
// Replace PlotDrawAxesLog function - remove lambda functions
void calculator::PlotDrawAxesLog (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 float__t ymin       = params.ymin;
 float__t ymax       = params.ymax;
 float__t xmin       = params.xmin;
 float__t xmax       = params.xmax;
 bool log_x          = params.log_x;
 bool log_y          = params.log_y;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;

 // Draw Y axis grid
 if (log_y)
  {
   // Logarithmic Y grid
   int decade_min = (int)Floor (Lg (ymin));
   int decade_max = (int)Ceil (Lg (ymax));

   for (int decade = decade_min; decade <= decade_max; decade++)
    {
     float__t base = Pow (10.0, decade);

     // Major grid line at each decade (1, 10, 100, ...)
     if (base >= ymin && base <= ymax)
      {
       // Convert Y to screen coordinate (logarithmic)
       float__t log_norm = (Lg (base) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
       int y_screen      = height - padding - (int)(log_norm * plot_height);

        // Draw major grid line
       if (y_screen >= padding && y_screen < height - padding)
        {
         for (int x = padding; x < width - padding; x++) bmp->drawPixel (x, y_screen, axis_color);
        }
        // NEW: Add label on the left side (skip if too close to edges)
        if (y_screen > padding + 15 && y_screen < height - padding - 15)
         {
          char grid_label[32];
          d2scistr (grid_label, (double)base);
          bmp->drawString (5, y_screen - 5, grid_label, text_color, 0, 1);
         }
       }

     // Minor grid lines (2, 3, 4, 5, 6, 7, 8, 9)
     for (int minor = 2; minor <= 9; minor++)
      {
       float__t value = base * minor;
       if (value >= ymin && value <= ymax)
        {
         // Convert Y to screen coordinate (logarithmic)
         float__t log_norm = (Lg (value) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
         int y_screen      = height - padding - (int)(log_norm * plot_height);

         if (y_screen >= padding && y_screen < height - padding)
          {
           for (int x = padding; x < width - padding; x += 4)
            bmp->drawPixel (x, y_screen, grid_color);
          }
        }
      }
    }
  }
 else
  {
   // Linear Y grid
   int grid_step_pixels = plot_height / 10;

   // Convert Y=0 to screen coordinate (linear)
   int y_axis_pixel = height - padding - (int)(((0.0 - ymin) / (ymax - ymin)) * plot_height);

   for (int offset = 0; offset <= plot_height; offset += grid_step_pixels)
    {
     int y_up   = y_axis_pixel - offset;
     int y_down = y_axis_pixel + offset;

     if (y_up >= padding && y_up < height - padding)
      {
       for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_up, grid_color);
      }

     if (offset > 0 && y_down >= padding && y_down < height - padding)
      {
       for (int x = padding; x < width - padding; x += 4) bmp->drawPixel (x, y_down, grid_color);
      }
    }
  }

 // Draw X axis grid
 if (log_x)
  {
   // Logarithmic X grid
   int decade_min = (int)Floor (Lg (xmin));
   int decade_max = (int)Ceil (Lg (xmax));

   for (int decade = decade_min; decade <= decade_max; decade++)
    {
     float__t base = Pow (10.0, decade);

     // Major grid line at each decade
     if (base >= xmin && base <= xmax)
      {
       // Convert X to screen coordinate (logarithmic)
       float__t log_norm = (Lg (base) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
       int x_screen      = padding + (int)(log_norm * plot_width);

       if (x_screen >= padding && x_screen < width - padding)
        {
         // Draw major grid line
         for (int y = padding; y < height - padding; y++) bmp->drawPixel (x_screen, y, axis_color);

         // NEW: Add label on the bottom (skip if too close to edges)
         if (x_screen > padding + 30 && x_screen < width - padding - 30)
          {
           char grid_label[32];
           d2scistr (grid_label, (double)base);
           bmp->drawString (x_screen - 15, height - padding + 5, grid_label, text_color, 0, 1);
          }
        }

      }

     // Minor grid lines
     for (int minor = 2; minor <= 9; minor++)
      {
       float__t value = base * minor;
       if (value >= xmin && value <= xmax)
        {
         // Convert X to screen coordinate (logarithmic)
         float__t log_norm = (Lg (value) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
         int x_screen      = padding + (int)(log_norm * plot_width);

         if (x_screen >= padding && x_screen < width - padding)
          {
           for (int y = padding; y < height - padding; y += 4)
            bmp->drawPixel (x_screen, y, grid_color);
          }
        }
      }
    }
  }
 else
  {
   // Linear X grid
   int grid_step_pixels = plot_width / 10;

   // Convert X=0 to screen coordinate (linear)
   int x_axis_pixel = padding + (int)(((0.0 - xmin) / (xmax - xmin)) * plot_width);

   for (int offset = 0; offset <= plot_width; offset += grid_step_pixels)
    {
     int x_left  = x_axis_pixel - offset;
     int x_right = x_axis_pixel + offset;

     if (x_left >= padding && x_left < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_left, y, grid_color);
      }

     if (offset > 0 && x_right >= padding && x_right < width - padding)
      {
       for (int y = padding; y < height - padding; y += 4) bmp->drawPixel (x_right, y, grid_color);
      }
    }
  }

 // Draw main axes
 // Calculate axis positions
 int x_axis_pixel;
 int y_axis_pixel;

 if (log_y)
  {
   float__t log_norm = (Lg (0.0) - Lg (ymin)) / (Lg (ymax) - Lg (ymin));
   x_axis_pixel      = height - padding - (int)(log_norm * plot_height);
  }
 else
  {
   x_axis_pixel = height - padding - (int)(((0.0 - ymin) / (ymax - ymin)) * plot_height);
  }

 if (log_x)
  {
   float__t log_norm = (Lg (0.0) - Lg (xmin)) / (Lg (xmax) - Lg (xmin));
   y_axis_pixel      = padding + (int)(log_norm * plot_width);
  }
 else
  {
   y_axis_pixel = padding + (int)(((0.0 - xmin) / (xmax - xmin)) * plot_width);
  }

 if (x_axis_pixel >= padding && x_axis_pixel < height - padding)
  {
   bmp->drawLine (padding, x_axis_pixel, width - padding, x_axis_pixel, 1, axis_color);
  }
 if (y_axis_pixel >= padding && y_axis_pixel < width - padding)
  {
   bmp->drawLine (y_axis_pixel, padding, y_axis_pixel, height - padding, 1, axis_color);
  }

 // Axis labels
 char label[64];

 d2scistr (label, (double)xmin);
 bmp->drawString (padding - 10, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)xmax);
 bmp->drawString (width - padding - 30, height - padding + 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymin);
 bmp->drawString (5, height - padding - 5, label, text_color, 0, 1);

 d2scistr (label, (double)ymax);
 bmp->drawString (5, padding + 5, label, text_color, 0, 1);

 // Variable name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);

 // Title
 char title[128];
 snprintf (title, sizeof (title), "y=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 //bmp->drawString (width / 2 - 50, 5, title, text_color, 0, 2);
 bmp->drawString (10, 5, title, text_color, 0, 2);
}

// PlotSmith with frequency labels:
bool calculator::PlotSmith (bmpdraw *bmp, PlotParams &params)
{
 calculator *child = params.child;
 float__t vfrom    = params.vfrom;
 float__t vto      = params.vto;
 float__t z0       = params.z0;

 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 uint32_t fgc        = params.fgc;
 uint32_t text_color = params.text_color;

 // Center of Smith chart
 int center_x = width / 2;
 int center_y = height / 2;

 // Calculate radius (use smaller dimension)
 int plot_size = (width < height ? width : height) - 2 * padding;
 int radius    = plot_size / 2;

 // Calculate step for smooth curve
 float__t step           = (vto - vfrom) / (radius * 50);
 uint64_t init_ms        = GetTickCount64 ();
 uint64_t last_gui_check = 0;

 // Storage for axis crossings
 struct AxisCrossing
 {
  float__t freq;
  int x, y;
  bool is_real_axis; // true = real axis, false = imaginary axis
 };

 AxisCrossing crossings[100];
 int crossing_count = 0;

 // Previous gamma values for detecting crossings
 float__t prev_gamma_re = 0, prev_gamma_im = 0;
 bool has_prev = false;

 // Draw the impedance trace
 float__t param        = vfrom;
 bool has_valid_points = false;

 do
  {
   if (check_break (init_ms, last_gui_check) != brNONE) return false;

   child->addfvar (params.svar, param);

   // Evaluate impedance (can be complex)
   child->evaluate_f (params.sexpr);

   float__t z_re = child->get_re_res ();
   float__t z_im = child->get_im_res ();

   // Check for valid result
   bool valid = true;
   if (isnan (z_re) && child->errt () == teMath)
    {
     param += step;
     continue;
    }
   else if (isnan (z_re))
    {
     valid = false;
    }

   if (valid)
    {
     // Calculate reflection coefficient: Γ = (Z - Z0) / (Z + Z0)
     float__t z_norm_re = z_re / z0;
     float__t z_norm_im = z_im / z0;

     // Γ = (Z/Z0 - 1) / (Z/Z0 + 1)
     float__t num_re = z_norm_re - 1.0;
     float__t num_im = z_norm_im;
     float__t den_re = z_norm_re + 1.0;
     float__t den_im = z_norm_im;

     // Complex division: (num_re + i*num_im) / (den_re + i*den_im)
     float__t den_mag2 = den_re * den_re + den_im * den_im;

     if (den_mag2 < 1e-15)
      {
       param += step;
       continue;
      }

     float__t gamma_re = (num_re * den_re + num_im * den_im) / den_mag2;
     float__t gamma_im = (num_im * den_re - num_re * den_im) / den_mag2;

     // Check if Γ is within unit circle
     float__t gamma_mag2 = gamma_re * gamma_re + gamma_im * gamma_im;
     if (gamma_mag2 > 1.0)
      {
       param += step;
       continue;
      }

     // Convert Γ to screen coordinates
     int x_screen = center_x + (int)(gamma_re * radius);
     int y_screen = center_y - (int)(gamma_im * radius);

     // Detect axis crossings
     if (has_prev && crossing_count < 100)
      {
       // Real axis crossing (gamma_im changes sign)
       if ((prev_gamma_im < 0 && gamma_im >= 0) || (prev_gamma_im > 0 && gamma_im <= 0))
        {
         // Linear interpolation to find exact crossing point
         float__t t              = Abs (prev_gamma_im) / (Abs (prev_gamma_im) + Abs (gamma_im));
         float__t cross_freq     = param - step + t * step;
         float__t cross_gamma_re = prev_gamma_re + t * (gamma_re - prev_gamma_re);

         crossings[crossing_count].freq         = cross_freq;
         crossings[crossing_count].x            = center_x + (int)(cross_gamma_re * radius);
         crossings[crossing_count].y            = center_y;
         crossings[crossing_count].is_real_axis = true;
         crossing_count++;
        }

       // Imaginary axis crossing (gamma_re changes sign)
       if ((prev_gamma_re < 0 && gamma_re >= 0) || (prev_gamma_re > 0 && gamma_re <= 0))
        {
         // Linear interpolation to find exact crossing point
         float__t t              = Abs (prev_gamma_re) / (Abs (prev_gamma_re) + Abs (gamma_re));
         float__t cross_freq     = param - step + t * step;
         float__t cross_gamma_im = prev_gamma_im + t * (gamma_im - prev_gamma_im);

         crossings[crossing_count].freq         = cross_freq;
         crossings[crossing_count].x            = center_x;
         crossings[crossing_count].y            = center_y - (int)(cross_gamma_im * radius);
         crossings[crossing_count].is_real_axis = false;
         crossing_count++;
        }
      }

     prev_gamma_re = gamma_re;
     prev_gamma_im = gamma_im;
     has_prev      = true;

     if (has_valid_points)
      {
       bmp->lineTo (x_screen, y_screen, 2, fgc);
      }
     else
      {
       bmp->moveTo (x_screen, y_screen);
       has_valid_points = true;
      }
    }
   else
    {
     has_valid_points = false;
     has_prev         = false;
    }

   param += step;
  }
 while (param <= vto);

 // Draw markers at axis crossings
 for (int i = 0; i < crossing_count; i++)
  {
   int mx = crossings[i].x;
   int my = crossings[i].y;

   // Draw small circle marker
   for (int angle = 0; angle < 360; angle += 30)
    {
     float__t rad = angle * M_PI / 180.0;
     int dx       = (int)(4 * Cos (rad));
     int dy       = (int)(4 * Sin (rad));
     bmp->drawPixel (mx + dx, my + dy, text_color);
    }

   // Format frequency label
   char label[64];
   float__t freq = crossings[i].freq;

   if (freq >= 1e9)
    sprintf (label, "%.2fG", (double)(freq / 1e9));
   else if (freq >= 1e6)
    sprintf (label, "%.1fM", (double)(freq / 1e6));
   else if (freq >= 1e3)
    sprintf (label, "%.1fk", (double)(freq / 1e3));
   else
    sprintf (label, "%.0f", (double)freq);

   // Position label (offset from marker)
   int label_x = mx + 8;
   int label_y = my - 8;

   // Adjust label position to avoid going off-screen
   if (label_x > width - 60) label_x = mx - 50;
   if (label_y < 20) label_y = my + 15;

   // NEW: Avoid Z0 label area (top-right corner)
   if (mx > center_x + radius - 70 && my > center_y - 40 && my < center_y - 5)
    {
     label_y = my + 15; // Move label below marker instead of above
    }

   bmp->drawString (label_x, label_y, label, text_color, 0, 1);
  }

 // Store parameters for grid drawing
 params.scale = (float__t)radius;

 return true;
}

//PlotDrawAxesSmith - drawing Smith chart grid:
void calculator::PlotDrawAxesSmith (bmpdraw *bmp, PlotParams &params)
{
 int width           = params.width;
 int height          = params.height;
 int padding         = params.padding;
 uint32_t grid_color = params.grid_color;
 uint32_t axis_color = params.axis_color;
 uint32_t text_color = params.text_color;
 float__t z0         = params.z0;

 int center_x = width / 2;
 int center_y = height / 2;
 int radius   = (int)params.scale;

 // Draw outer circle (|Γ| = 1)
 for (int angle = 0; angle < 360; angle++)
  {
   float__t rad = angle * M_PI / 180.0;
   int x        = center_x + (int)(radius * Cos (rad));
   int y        = center_y - (int)(radius * Sin (rad));
   bmp->drawPixel (x, y, axis_color);
  }

 // Draw horizontal axis (real axis of Γ)
 bmp->drawLine (center_x - radius, center_y, center_x + radius, center_y, 1, axis_color);

 // Constant resistance circles (normalized r = R/Z0)
 // Draw circles for r = 0.2, 0.5, 1.0, 2.0, 5.0
 float__t r_values[] = { 0.2, 0.5, 1.0, 2.0, 5.0 };
 for (int i = 0; i < 5; i++)
  {
   float__t r = r_values[i];

   // Circle center and radius for constant resistance
   // Center: (r/(1+r), 0) in Γ plane
   // Radius: 1/(1+r)
   float__t circle_center_x = r / (1.0 + r);
   float__t circle_radius   = 1.0 / (1.0 + r);

   int screen_center_x = center_x + (int)(circle_center_x * radius);
   int screen_radius   = (int)(circle_radius * radius);

   // Draw circle (upper and lower halves)
   for (int angle = 0; angle < 360; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x        = screen_center_x + (int)(screen_radius * Cos (rad));
     int y        = center_y - (int)(screen_radius * Sin (rad));

     if (x >= padding && x < width - padding && y >= padding && y < height - padding)
      bmp->drawPixel (x, y, grid_color);
    }
  }

 // Constant reactance arcs (normalized x = X/Z0)
 // Draw arcs for x = ±0.2, ±0.5, ±1.0, ±2.0, ±5.0
 float__t x_values[] = { 0.2, 0.5, 1.0, 2.0, 5.0 };
 for (int i = 0; i < 5; i++)
  {
   float__t x = x_values[i];

   // Arc center and radius for constant reactance
   // Center: (1, 1/x) in Γ plane
   // Radius: 1/x
   float__t arc_center_x     = 1.0;
   float__t arc_center_y_pos = 1.0 / x;
   float__t arc_center_y_neg = -1.0 / x;
   float__t arc_radius       = 1.0 / x;

   int screen_center_x = center_x + (int)(arc_center_x * radius);

   // Positive reactance (inductive, upper half)
   int screen_center_y_pos = center_y - (int)(arc_center_y_pos * radius);
   int screen_arc_radius   = (int)(arc_radius * radius);

   // Draw arc from left intersection to right edge
   for (int angle = 90; angle <= 270; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x_pos    = screen_center_x + (int)(screen_arc_radius * Cos (rad));
     int y_pos    = screen_center_y_pos - (int)(screen_arc_radius * Sin (rad));

     // Check if inside outer circle
     int dx = x_pos - center_x;
     int dy = y_pos - center_y;
     if (dx * dx + dy * dy <= radius * radius)
      {
       if (x_pos >= padding && x_pos < width - padding && y_pos >= padding
           && y_pos < height - padding)
        bmp->drawPixel (x_pos, y_pos, grid_color);
      }
    }

   // Negative reactance (capacitive, lower half)
   int screen_center_y_neg = center_y - (int)(arc_center_y_neg * radius);

   for (int angle = 90; angle <= 270; angle += 2)
    {
     float__t rad = angle * M_PI / 180.0;
     int x_neg    = screen_center_x + (int)(screen_arc_radius * Cos (rad));
     int y_neg    = screen_center_y_neg - (int)(screen_arc_radius * Sin (rad));

     // Check if inside outer circle
     int dx = x_neg - center_x;
     int dy = y_neg - center_y;
     if (dx * dx + dy * dy <= radius * radius)
      {
       if (x_neg >= padding && x_neg < width - padding && y_neg >= padding
           && y_neg < height - padding)
        bmp->drawPixel (x_neg, y_neg, grid_color);
      }
    }
  }

 // Add labels
 char label[64];

 // Z0 label
 sprintf (label, "Z0=%.0fΩ", (double)z0);
 bmp->drawString (center_x + radius + 5, center_y - 30, label, text_color, 0, 1);

 // Open circuit (right edge)
 bmp->drawString (center_x + radius - 20, center_y + 5, "∞", text_color, 0, 1);

 // Short circuit (left edge)
 bmp->drawString (center_x - radius + 5, center_y + 5, "0", text_color, 0, 1);

 // Matched load (center)
 bmp->drawString (center_x - 10, center_y + 15, "Z0", text_color, 0, 1);

 // Title
 char title[128];
 snprintf (title, sizeof (title), "Z=%s", params.sexpr);
 title[sizeof (title) - 1] = '\0';
 bmp->drawString (5, 5, title, text_color, 0, 2);

 // Parameter name
 bmp->drawString (width / 2 - 10, height - padding + 5, params.svar, text_color, 0, 2);
}

// Plot data from file (two-pass like PlotCartesian)
bool calculator::PlotData (bmpdraw *bmp, PlotParams &params)
{
 const char *datafile = params.sexpr; // data file name
 const char *mask     = params.svar;  // mask (can be empty or nullptr)

 int width    = params.width;
 int height   = params.height;
 int padding  = params.padding;
 uint32_t fgc = params.fgc;
 bool dots    = params.dot; // points or lines

 // Determine the number of parameters in the mask
 int param_count = scanmasknum (mask);
 if (param_count > 2)
  {
   errorf (pos, "Mask has more than 2 parameters (max is 2: x and y)");
   return false;
  }
 if (param_count == 0) param_count = 1; // If no mask - one parameter

 // Normalize file path
 char fnamebuf[STRBUF];
 NormalizePath (datafile, fnamebuf, STRBUF);

 float__t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
 bool first_point = true;
 int line_number  = 0;

 // First pass: find min/max
 FILE *f = fopen (fnamebuf, "r");
 if (!f)
  {
   errorf (pos, "Cannot open data file: %s", fnamebuf);
   return false;
  }

 char line[1024];
 while (fgets (line, sizeof (line), f))
  {
   line_number++;

   float__t x, y;

   if (param_count == 1)
    {
     // One parameter: X = line number, Y = value
     double dy   = qnan;
     int scanned = strscan (line, mask, 1, &dy);

     if (scanned < 1) continue; // Not enough data in the line
     x = (float__t)line_number;
     y = (float__t)dy;
    }
   else // param_count == 2
    {
     // Two parameters: X = first value, Y = second value
     double dx = qnan, dy = qnan;
     int scanned = strscan (line, mask, 2, &dx, &dy);

     if (scanned < 2) continue; // Not enough data in the line
     x = (float__t)dx;
     y = (float__t)dy;
    }

   // Update boundaries
   if (first_point)
    {
     xmin = xmax = x;
     ymin = ymax = y;
     first_point = false;
    }
   else
    {
     if (x < xmin) xmin = x;
     if (x > xmax) xmax = x;
     if (y < ymin) ymin = y;
     if (y > ymax) ymax = y;
    }
  }

 fclose (f);

 if (first_point)
  {
   errorf (pos, "No valid data points found in file");
   return false;
  }

 // Add padding (as in PlotCartesian)
 if (xmin == xmax)
  {
   xmin -= (float__t)1.0L;
   xmax += (float__t)1.0L;
  }
 else
  {
   float__t xpad = (xmax - xmin) * (float__t)0.1L;
   xmin -= xpad;
   xmax += xpad;
  }

 if (ymin == ymax)
  {
   ymin -= (float__t)1.0L;
   ymax += (float__t)1.0L;
  }
 else
  {
   float__t ypad = (ymax - ymin) * (float__t)0.1L;
   ymin -= ypad;
   ymax += ypad;
  }

 // Include zero in Y range
 if (ymin > 0.0) ymin = 0.0;
 if (ymax < 0.0) ymax = 0.0;

 // Calculate ranges for screen mapping
 float__t x_range = xmax - xmin;
 float__t y_range = ymax - ymin;

 int plot_width  = width - 2 * padding;
 int plot_height = height - 2 * padding;

 // Second pass: draw points/lines
 f = fopen (fnamebuf, "r");
 if (!f)
  {
   errorf (pos, "Cannot reopen data file");
   return false;
  }

 line_number           = 0;
 bool has_valid_points = false;

 while (fgets (line, sizeof (line), f))
  {
   line_number++;

   float__t x, y;

   if (param_count == 1)
    {
     double dy   = qnan;
     int scanned = strscan (line, mask, 1, &dy);

     if (scanned < 1) continue;

     x = (float__t)line_number;
     y = (float__t)dy;
    }
   else // param_count == 2
    {
     double dx = qnan, dy = qnan;
     int scanned = strscan (line, mask, 2, &dx, &dy);

     if (scanned < 2) continue;

     x = (float__t)dx;
     y = (float__t)dy;
    }

   // Convert to screen coordinates
   int x_screen = padding + (int)(((x - xmin) / x_range) * plot_width);
   int y_screen = height - padding - (int)(((y - ymin) / y_range) * plot_height);

   if (dots)
    {
     // Draw dot (circle)
     int point_size = params.pxsize;
     for (int dy = -point_size; dy <= point_size; dy++)
      {
       for (int dx = -point_size; dx <= point_size; dx++)
        {
         if (dx * dx + dy * dy <= point_size * point_size) // Circle
          {
           int px = x_screen + dx;
           int py = y_screen + dy;
           if (px >= padding && px < width - padding && py >= padding && py < height - padding)
            {
             bmp->drawPixel (px, py, fgc);
            }
          }
        }
      }
    }
   else
    {
     // Draw line
     if (has_valid_points)
      {
       bmp->lineTo (x_screen, y_screen, 2, fgc);
      }
     else
      {
       bmp->moveTo (x_screen, y_screen);
       has_valid_points = true;
      }
    }
  }

 fclose (f);

 // Store parameters for grid drawing
 params.xmin  = xmin;
 params.xmax  = xmax;
 params.ymin  = ymin;
 params.ymax  = ymax;
 params.vfrom = xmin;
 params.vto   = xmax;

 return true;
}


// Main plotting function
bool calculator::Plot (const char *expr, v_func fidx)
{
 char fname[STRBUF];
 fname[0] = '\0';

 PlotParams params;
 memset (&params, 0, sizeof (params));

 // 1. Prepare data
 if (!PlotPrepare (expr, fidx, fname, params))
  {
   return false;
  }

 NormalizePath (fname, fname, STRBUF);
 
  // 2. Create or load bitmap
 bmpdraw *bmp    = new bmpdraw ();
 bool is_overlay     = ((fidx == pl_oplot || fidx == pl_oplotpol || fidx == pl_oxyplot
                     || fidx == pl_oplotlgx || fidx == pl_oplotlgy || fidx == pl_oplotlgxy
                     || fidx == pl_oplotsmith || fidx == pl_oplotsmithz 
                     || fidx == pl_oplotdata || fidx == pl_oplotdatal)
                    && fname[0]);

 bool is_polar       = (fidx == pl_plotpol || fidx == pl_fplotpol || fidx == pl_oplotpol);

 bool is_parametric = (fidx == pl_xyplot || fidx == pl_fxyplot || fidx == pl_oxyplot);

 bool is_logarithmic = (fidx == pl_plotlgx || fidx == pl_fplotlgx || fidx == pl_oplotlgx ||
                        fidx == pl_plotlgy || fidx == pl_fplotlgy || fidx == pl_oplotlgy ||
                        fidx == pl_plotlgxy || fidx == pl_fplotlgxy || fidx == pl_oplotlgxy);

 bool is_smith       = (fidx == pl_plotsmith || fidx == pl_fplotsmith || fidx == pl_oplotsmith
                  || fidx == pl_plotsmithz || fidx == pl_fplotsmithz || fidx == pl_oplotsmithz);

 bool is_data = (fidx >= pl_plotdata && fidx <= pl_oplotdatal);

 if (is_overlay)
  {
   // Try to load existing image for overlay
   if (bmp->load (fname))
    {
     // Successfully loaded — get dimensions and dominant background color
     params.width      = bmp->getWidth ();
     params.height     = bmp->getHeight ();
     params.bgc        = bmp->getDominantColor ();
     params.text_color = ~params.bgc;
    }
   else
    {
     // Failed to load — create new bitmap (fallback to fplot behavior)
     is_overlay = false;
     if (!bmp->newbmp (params.width, params.height, params.bgc))
      {
       errorf (pos, "Failed to create bitmap for plotting");
       delete bmp;
       if (params.child) delete params.child;
       if (params.sexpr) free (params.sexpr);
       if (params.sexpr_y) free (params.sexpr_y);
       if (params.svar) free (params.svar);
       result_fval = qnan;
       return false;
      }
    }
  }
 else
  {
   // Normal plot or fplot — create new bitmap
   if (!bmp->newbmp (params.width, params.height, params.bgc))
    {
     errorf (pos, "Failed to create bitmap for plotting");
     delete bmp;
     if (params.child) delete params.child;
     if (params.sexpr) free (params.sexpr);
     if (params.sexpr_y) free (params.sexpr_y);
     if (params.svar) free (params.svar);
     result_fval = qnan;
     return false;
    }
  }

 // 3. Draw plot (Cartesian or Polar coordinates)
 bool plot_success;
 if (is_data)
  {
   plot_success = PlotData (bmp, params);
  }
 else
 if (is_smith)
  {
   plot_success = PlotSmith (bmp, params);
  }
 else
 if (is_logarithmic)
  {
   plot_success = PlotLogarithmic (bmp, params);
  }
 else
 if (is_parametric)
  {
   plot_success = PlotParametric (bmp, params);
  }
 else
 if (is_polar)
  {
   plot_success = PlotPolar (bmp, params);
  }
 else
  {
   plot_success = PlotCartesian (bmp, params);
  }

 if (!plot_success)
  {
   delete bmp;
   delete params.child;
   if (params.sexpr) free (params.sexpr);
   if (params.sexpr_y) free (params.sexpr_y);
   if (params.svar) free (params.svar);
   result_fval = qnan;
   return false;
  }

 // 4. Draw axes and grid (skip for overlay mode)
 if (!is_overlay)
  {
   if (is_smith)
    {
     PlotDrawAxesSmith (bmp, params);
    }
   else 
   if (is_logarithmic)
    {
     PlotDrawAxesLog (bmp, params);
    }
   else
   if (is_parametric)
    {
     PlotDrawAxesParametric (bmp, params);
    }
   else 
   if (is_polar)
    {
     PlotDrawAxesPolar (bmp, params);
    }
   else
    {
     PlotDrawAxesCartesian (bmp, params);
    }
  }

// 5. Save or display
 switch (fidx)
 {
  case pl_fplot:
  case pl_oplot:
  case pl_fplotpol:
  case pl_oplotpol:
  case pl_fxyplot:
  case pl_oxyplot:
  case pl_fplotlgx:
  case pl_oplotlgx:
  case pl_fplotlgy:
  case pl_oplotlgy:
  case pl_fplotlgxy:
  case pl_oplotlgxy:
  case pl_fplotsmith:
  case pl_oplotsmith:
  case pl_fplotsmithz:
  case pl_oplotsmithz:
  case pl_fplotdata:
  case pl_oplotdata:
   // Already handled above with is_overlay logic
   bmp->save (fname);
  break;
  case pl_plot:
  case pl_plotpol:
  case pl_xyplot:
  case pl_plotlgx:
  case pl_plotlgy:
  case pl_plotlgxy:
  case pl_plotsmith:
  case pl_plotsmithz:
  case pl_plotdata:
  case pl_plotdatal:
   // Show in GUI
   if (ShowImageFn)
    {
     ShowImageFn ((void *)bmp);
    }
  break;
  }

 // 6. Cleanup
 delete bmp;
 fflags |= params.child->isfflags ();
 delete params.child;
 if (params.sexpr) free (params.sexpr);
 if (params.sexpr_y) free (params.sexpr_y);
 if (params.svar) free (params.svar);

 return true;
}
#pragma endregion
//---------------------------------------------------------------------------

#pragma region SOLVERS

// Newton-Raphson solution of the equation solve(x(2x+2)-2, x:=0)
// expr -> x(2x+2)-2, x:=0
// Complex support
// #define _DAMPED_
bool calculator::Solve (const char *expr, t_symbol tag, float__t &re_res, float__t &im_res)
{
 if (!expr || !*expr)
  {
   errorf (0, "empty expression");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 char sexpr[STRBUF], svar[STRBUF], nvar[MAXOP];

 if (!Split (expr, sexpr, STRBUF, svar, STRBUF, nullptr, 0))
  {
   errorf (pos, "solve: invalid expression");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   errorf (pos, "Out of memory");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 child->evaluate_f (svar);
 if (child->err[0])
  {
   errorf (pos, "%s", child->err);
   delete child;
   re_res = qnan;
   im_res = qnan;
   return false;
  }
 strcpy (nvar, (char *)child->get_last_var ());
 float__t xr = child->get_re_res ();
 float__t xi = child->get_im_res (); // 0 if real initial approximation

 if (tag == tsCALC)
  {
   child->addfvar (nvar, xr, xi);
   child->evaluate_f (sexpr);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     re_res = qnan;
     im_res = qnan;
     return false;
    }
   re_res = child->get_re_res ();
   im_res = child->get_im_res ();
   fflags |= child->isfflags ();
   delete child;
   return true;
  }

#ifdef _float128_
 const float__t tol   = 1e-28Q;
 const float__t delta = 1.5e-17Q;
#else
 const float__t tol   = 1e-12L;
 const float__t delta = 1.5e-10L;
#endif
 const int maxIter = 100;
 bool converged    = false;

 for (int i = 0; i < maxIter; i++)
  {
   // f(z)
   child->addfvar (nvar, xr, xi);
   child->evaluate_f (sexpr);
   if (child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     re_res = qnan;
     im_res = qnan;
     return false;
    }
   float__t fr = child->get_re_res ();
   float__t fi = child->get_im_res ();

   // |f(z)|
   float__t fabs_f = Sqrt (fr * fr + fi * fi);
   if (fabs_f < tol)
    {
     converged = true;
     break;
    }

    // step d = max(|z|, 1) * delta
#ifdef _float128_
   float__t d = fmaxq (sqrtq (xr * xr + xi * xi), 1.0Q) * delta;
#else
#ifdef __BORLANDC__
   float__t ax = Sqrt (xr * xr + xi * xi);
   float__t d  = ((ax > 1.0L) ? ax : 1.0L) * delta;
#else
   float__t d = fmaxl (Sqrt (xr * xr + xi * xi), 1.0L) * delta;
#endif
#endif
   // f(z+d) and f(z-d)  on the real axis
   child->addfvar (nvar, xr + d, xi);
   child->evaluate_f (sexpr);
   float__t fpr_r = child->get_re_res (), fpr_i = child->get_im_res ();

   child->addfvar (nvar, xr - d, xi);
   child->evaluate_f (sexpr);
   float__t fmr_r = child->get_re_res (), fmr_i = child->get_im_res ();

   // f'(z) ≈ (f(z+d) - f(z-d)) / (2d)
   float__t fp_r = (fpr_r - fmr_r) / ((float__t)2.0 * d);
   float__t fp_i = (fpr_i - fmr_i) / ((float__t)2.0 * d);

   float__t fp2 = fp_r * fp_r + fp_i * fp_i;
   if (fp2 < tol * tol)
    {
     xr += d * (float__t)1000.0;
     continue;
    }

   // Newton step: z -= f(z)/f'(z)
   float__t step_r = (fr * fp_r + fi * fp_i) / fp2;
   float__t step_i = (fi * fp_r - fr * fp_i) / fp2;
   float__t xr_new = xr - step_r;
   float__t xi_new = xi - step_i;

   if (isnan (xr_new) || isnan (xi_new))
    {
     errorf (pos, "Solution diverged");
     delete child;
     re_res = qnan;
     im_res = qnan;
     return false;
    }

#ifdef _DAMPED_
   // damped Newton:
   float__t step_abs = Sqrt (step_r * step_r + step_i * step_i);
   float__t z_abs    = Sqrt (xr * xr + xi * xi);
   float__t max_step = ((float__t)1.0 + z_abs) * (float__t)10.0; // step limit
   if (step_abs > max_step)
    {
     float__t scale = max_step / step_abs;
     step_r *= scale;
     step_i *= scale;
    }
   xr = xr - step_r;
   xi = xi - step_i;
#else
   float__t step_abs = Sqrt (step_r * step_r + step_i * step_i);
   float__t z_abs    = Sqrt (xr * xr + xi * xi);
   if (step_abs < tol * ((float__t)1.0 + z_abs))
    {
     xr        = xr_new;
     xi        = xi_new;
     converged = true;
     break;
    }
   xr = xr_new;
   xi = xi_new;
#endif
  }

 fflags |= child->isfflags ();
 delete child;

 if (!converged)
  {
   errorf (pos, "No solution found");
   re_res = qnan;
   im_res = qnan;
   return false;
  }

 re_res = xr;
 im_res = xi;
 return true;
}

// Gauss-Kronrod G7/K15 adaptive quadrature
// C++98 compatible (BCB6 / VS2022)
//
// K15 nodes (positive half, index 0 = center node = 0.0)
// G7 uses nodes at indices 0, 2, 4, 6  (every other K15 node)

static const float__t GK_NODES[8] = {
 0.0L,
 0.20778495500789846760L,
 0.40584515137739716691L,
 0.58608723546769113029L,
 0.74153118559939443986L,
 0.86486442335976907279L,
 0.94910791234275852453L,
 0.99145537112081263921L,
};

// K15 weights: index i corresponds to nodes +-GK_NODES[i]
// (index 0 has no symmetry — centre point)
static const float__t K15_WEIGHTS[8] = {
 0.20948214108472782801L, 0.20443294007529889241L, 0.19035057806478540991L, 0.16900472663926790283L,
 0.14065325971552591875L, 0.10479001032224928880L, 0.06309209262997855329L, 0.02293532201052922497L,
};

// G7 weights: 4 values for nodes at GK_NODES[0,2,4,6]
static const float__t G7_WEIGHTS[4] = {
 0.41795918367346938776L,
 0.38183005050511894495L,
 0.27970539148927664160L,
 0.12948496616886732340L,
};

// G7 node indices into GK_NODES[]
static const int G7_IDX[4] = { 0, 2, 4, 6 };

// ---------------------------------------------------------------------------

// Evaluate f(x) = sexpr with variable svar set to x in child calculator
// Returns qnan on any error
float__t calculator::gkEval (calculator *pCalc, char *sexpr, const char *svar, float__t x)
{
 pCalc->addfvar (svar, x);
 float__t val = pCalc->evaluate_f (sexpr);
 if (isnan (val) && pCalc->errt () == teMath) return (float__t)0.0L;

 if (pCalc->err[0]) return qnan;
 return val;
}

// Single G7/K15 panel on [a, b], no recursion
GKResult calculator::gkPanel (calculator *pCalc, char *sexpr, const char *svar, float__t a,
                              float__t b)
{
 GKResult res    = { 0.0L, 0.0L, true };
 float__t center = (a + b) / 2.0L;
 float__t half   = (b - a) / 2.0L;

 // f values at all 15 points: fL[i] = f(center - half*node[i])
 //                            fR[i] = f(center + half*node[i])
 // index 0: fL[0] == fR[0] == f(center)
 float__t fL[8], fR[8];
 fL[0] = fR[0] = gkEval (pCalc, sexpr, svar, center);
 if (isnan (fL[0]))
  {
   res.ok = false;
   return res;
  }

 for (int i = 1; i < 8; i++)
  {
   fL[i] = gkEval (pCalc, sexpr, svar, center - half * GK_NODES[i]);
   if (isnan (fL[i]))
    {
     res.ok = false;
     return res;
    }
   fR[i] = gkEval (pCalc, sexpr, svar, center + half * GK_NODES[i]);
   if (isnan (fR[i]))
    {
     res.ok = false;
     return res;
    }
  }

 // K15: sum over all 8 node pairs (index 0 counted once)
 float__t k15 = K15_WEIGHTS[0] * fL[0];
 for (int i = 1; i < 8; i++) k15 += K15_WEIGHTS[i] * (fL[i] + fR[i]);
 k15 *= half;

 // G7: sum over node indices 0, 2, 4, 6
 float__t g7 = G7_WEIGHTS[0] * fL[0];
 for (int i = 1; i < 4; i++)
  {
   int idx = G7_IDX[i];
   g7 += G7_WEIGHTS[i] * (fL[idx] + fR[idx]);
  }
 g7 *= half;

 res.value = k15;
 res.error = fabsl (k15 - g7);
 return res;
}

// Adaptive G7/K15: recursively subdivide until error < tol or maxDepth reached
GKResult calculator::gkAdaptive (calculator *pCalc, char *sexpr, const char *svar, float__t a,
                                 float__t b,
                                 float__t tol, // not divided!
                                 int depth, int maxDepth, int &callCount, int maxCalls)
{
 if (++callCount > maxCalls) // hard stop
  {
   GKResult res = gkPanel (pCalc, sexpr, svar, a, b);
   res.ok       = true;
   return res;
  }

 GKResult res = gkPanel (pCalc, sexpr, svar, a, b);
 if (!res.ok) return res;

 if (res.error <= tol || depth >= maxDepth) return res;

 float__t mid = (a + b) / 2.0L;

 GKResult left
     = gkAdaptive (pCalc, sexpr, svar, a, mid, tol, depth + 1, maxDepth, callCount, maxCalls);
 GKResult right
     = gkAdaptive (pCalc, sexpr, svar, mid, b, tol, depth + 1, maxDepth, callCount, maxCalls);

 if (!left.ok || !right.ok)
  {
   GKResult bad = { qnan, qnan, false };
   return bad;
  }

 GKResult combined;
 combined.value = left.value + right.value;
 combined.error = left.error + right.error;
 combined.ok    = true;
 return combined;
}


bool calculator::For (const char *expr, value &res)
{
 if (expr && *expr)
  {
   char sexpr[STRBUF];
   char sfrom[MAXOP];
   char sto[MAXOP];
   char svar[STRBUF];

   float__t vfrom  = qnan;
   float__t vto    = qnan;
   float__t fvx    = qnan;
   float__t result = 0;
   int callCount   = 0;

   if (!Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0))
    {
     result_fval = qnan;
     return false;
    }

   calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
   if (!child)
    {
     errorf (pos, "Out of memory");
     result_fval = qnan;
     return false;
    }

   vfrom = child->evaluate_f (sfrom);
   if (isnan (vfrom) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
   vto = child->evaluate_f (sto);
   if (isnan (vto) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     result_fval = qnan;
     return false;
    }
   {
    float__t fvx            = (float__t)0.0L;
    uint64_t init_ms        = GetTickCount64 ();
    uint64_t last_gui_check = 0;

    if (vfrom > vto)
     {
      do
       {
        if (check_break (init_ms, last_gui_check) != brNONE)
         {
          delete child;
          result_fval = qnan;
          return false;
         }

        child->addfvar (svar, vfrom);
        fvx = child->evaluate_f (sexpr); // evaluate the function for
                                         // the syntax check before starting the integration

        if (isnan (fvx) || child->err[0])
         {
          errorf (pos, "%s", child->err);
          delete child;
          result_fval = qnan;
          return false;
         }
        vfrom -= (float__t)1.0L; // increment by 1 for summation, this can be modified to support
                                 // different step sizes
       }
      while (vfrom >= vto);
     }
    else
     {
      do
       {
        if (check_break (init_ms, last_gui_check) != brNONE)
         {
          delete child;
          result_fval = qnan;
          return qnan;
         }

        child->addfvar (svar, vfrom);
        fvx = child->evaluate_f (sexpr); // evaluate the function for
                                         // the syntax check before starting the integration

        if (isnan (fvx) || child->err[0])
         {
          errorf (pos, "%s", child->err);
          delete child;
          result_fval = qnan;
          return false;
         }
        vfrom += (float__t)1.0L; // increment by 1 for summation, this can be modified to support
                                 // different step sizes
       }
      while (vfrom <= vto);
     }
   }

   res.tag = child->get_res_tag ();
   if (res.tag == tvMATRIX)
    {
     res.fval       = child->get_re_res ();
     res.ival       = child->get_int_res ();
     res.imval      = child->get_im_res ();
     res.sval       = dupString (get_str_res ());
     mxresult_t mxr = child->get_mx_res ();
     res.mcols      = mxr.cols;
     res.mrows      = mxr.rows;
     int msize      = mxr.rows * mxr.cols * sizeof (float__t);
     if (msize)
      {
       float__t *new_mval = (float__t *)sf_alloc (msize);
       if (new_mval)
        {
         memcpy (new_mval, mxr.mval, msize);
         res.mval = new_mval;
        }
       else
        {
         errorf (res.pos, "Out of memory");
         delete child;
         result_fval = qnan;
         return false;
        }
      }
    }
   else
    {
     res.fval  = child->get_re_res ();
     res.ival  = child->get_int_res ();
     res.imval = child->get_im_res ();
     res.sval  = dupString (get_str_res ());
    }

   fflags |= child->isfflags ();
   delete child;
   return true;
  }
 return false;
}



// integr(expr(x), from, to, x) integr(sin(x)/x, 0.001, pi, x)
// sum(expr(x), from, to, x) 
// expr -> sin(x)/x, x
float__t calculator::Integr (const char *expr, t_symbol tag)
{
 if (expr && *expr)
  {
   char sexpr[STRBUF];
   char sfrom[MAXOP];
   char sto[MAXOP];
   char svar[STRBUF];

   float__t vfrom = qnan;
   float__t vto   = qnan;
   float__t fvx   = qnan;
   float__t result = 0;
   int callCount   = 0;

   if (!Split (expr, sexpr, STRBUF, sfrom, MAXOP, sto, MAXOP, svar, STRBUF, nullptr, 0))
    {
     return result_fval = qnan;
    }

   calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
   if (!child)
    {
     errorf (pos, "Out of memory");
     return result_fval = qnan;
    }

   vfrom = child->evaluate_f (sfrom);
   if (isnan (vfrom) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }
   vto = child->evaluate_f (sto);
   if (isnan (vto) || child->err[0])
    {
     errorf (pos, "%s", child->err);
     delete child;
     return result_fval = qnan;
    }

   if (tag == tsINTEGR)
    {
     {
      child->addfvar (svar, vfrom);
      float__t fvx = child->evaluate_f (sexpr); // evaluate the function for
                                           // the syntax check before starting the integration
      if (isnan (fvx) && child->errt () == teMath) 
       {
        fvx = 0; // treat math error at the starting point as zero (e.g. sin(x)/x at x=0)
       }
      else
      if (isnan (fvx) || child->err[0])
       {
        errorf (pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
       }
     }

     GKResult gkresult = gkAdaptive (child,
                                     sexpr,     // expression
                                     svar,      // variable name ("x")
                                     vfrom,     // lower limit
                                     vto,       // upper limit
                                     1e-10L,    // tolerance
                                     0,         // initial depth
                                     20,        // maximum depth
                                     callCount, // call count
                                     1000);     // maximum calls
     if (!gkresult.ok)
      {
       errorf (pos, "Integration failed");
       delete child;
       return result_fval = qnan;
      }
     result = gkresult.value;
    }
   else if (tag == tsSUM)
    {
     uint64_t init_ms = GetTickCount64 ();
     uint64_t last_gui_check = 0;

     float__t fvx = 0.0;
     if (vfrom > vto)
      {
       do
        {
         if (check_break (init_ms, last_gui_check) != brNONE)
          {
           delete child;
           return result_fval = qnan;
          }

         child->addfvar (svar, vfrom);
         fvx += child->evaluate_f (sexpr); 
         if (isnan (fvx) && child->errt () == teMath)
          {
           fvx = 0; // treat math error at the starting point as zero (e.g. sin(x)/x at x=0)
          }
         else
         if (isnan (fvx) || child->err[0])
          {
           errorf (pos, "%s", child->err);
           delete child;
           return result_fval = qnan;
          }
         vfrom -= 1.0; // increment by 1 for summation, this can be modified to support different
                       // step sizes
        }
       while (vfrom >= vto);
      }
     else
      {
       do
        {
         if (check_break (init_ms, last_gui_check) != brNONE)
          {
           delete child;
           return result_fval = qnan;
          }

         child->addfvar (svar, vfrom);
         fvx += child->evaluate_f (sexpr); 
         if (isnan (fvx) && child->errt () == teMath)
          {
           fvx = 0; // treat math error at the starting point as zero (e.g. sin(x)/x at x=0)
          }
         else
         if (isnan (fvx) || child->err[0])
          {
           errorf (pos, "%s", child->err);
           delete child;
           return result_fval = qnan;
          }
         vfrom += 1.0; // increment by 1 for summation, this can be modified to support different
                       // step sizes
        }
       while (vfrom <= vto);
      }
     result = fvx;
    }
   else
    {
     errorf (pos, "Unknown tag");
     result = qnan;
    }
   fflags |= child->isfflags ();
   delete child;
   return result;
  }
 return result_fval = qnan; // placeholder for actual integration result
}

// Numerical differentiation using central difference
// diff(expr(x), point, x) diff(sin(x)/x, 0.01, x)
float__t calculator::Diff (const char *expr)
{
 if (expr && *expr)
  {
     char sexpr[STRBUF];
     char sfrom[MAXOP];
     char svar[STRBUF];

     float__t x  = qnan;
     float__t fvx    = qnan;
     float__t result = 0;

     if (!Split (expr, sexpr, STRBUF, sfrom, MAXOP,  svar, STRBUF, nullptr, 0))
      {
       return result_fval = qnan;
      }

     calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
     if (!child)
      {
       errorf (pos, "Out of memory");
       return result_fval = qnan;
      }

     x = child->evaluate_f (sfrom);
     if (isnan (x) || child->err[0])
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     // Numerical derivative (central difference)
#ifdef _float128_
     float__t delta = fmaxq (fabsq (x), 1.0Q) * 1.5e-17Q;
#elif defined(__BORLANDC__)
     float__t ax    = fabsl (x);
     float__t delta = ((ax > 1.0L) ? ax : 1.0L) * 1.5e-10L;
#else
     float__t delta = fmaxl (fabsl (x), 1.0L) * 1.5e-10L;
#endif
     child->addfvar (svar, x + delta);
     float__t fxp = child->evaluate_f (sexpr);
     if (isnan (fxp) || child->err[0])
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     child->addfvar (svar, x - delta);
     float__t fxm = child->evaluate_f (sexpr);
     if (isnan (fxm) || child->err[0])
      {
       errorf (pos, "%s", child->err);
       delete child;
       return result_fval = qnan;
      }

     float__t fp = (fxp - fxm) / ((float__t)2.0 * delta);

     fflags |= child->isfflags ();
     delete child;
     return fp;
    }
  return result_fval = qnan; 
}
#pragma endregion
//---------------------------------------------------------------------------

#pragma region MATRIX OPERATIONS

// matrixbin: binary operations, matrixuno: unary operations
// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Allocate a new matrix rows*cols, register for auto-free
// Returns NULL on failure (error already set)
float__t *calculator::mxAlloc (int rows, int cols)
{
 float__t *mval = (float__t *)sf_alloc (rows * cols * sizeof (float__t));
 if (!mval)
  {
   mxerror ("memory allocation failed");
   return NULL;
  }
 return mval;
}

// Fill res from left op right, element-wise, both must be same-size matrices
// op: 0=add, 1=sub, 2=mul, 3=div
bool calculator::mxElemOp (value &res, value &left, value &right, int op)
{
 if (left.mrows != right.mrows || left.mcols != right.mcols)
  {
   mxerror ("dimensions must match");
   return false;
  }
 int rows       = left.mrows;
 int cols       = left.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 switch (op)
  {
  case 0:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] + right.mval[i];
   break;
  case 1:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] - right.mval[i];
   break;
  case 2:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] * right.mval[i];
   break;
  case 3:
   for (int i = 0; i < n; i++) mval[i] = left.mval[i] / right.mval[i];
   break;
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// Fill res from matrix op scalar or scalar op matrix
// side: 0 = matrix op scalar, 1 = scalar op matrix
bool calculator::mxScalarOp (value &res, value &mx, float__t scalar, int op, bool scalar_left)
{
 int rows       = mx.mrows;
 int cols       = mx.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 switch (op)
  {
  case 0: // add: scalar+M == M+scalar
   for (int i = 0; i < n; i++) mval[i] = mx.mval[i] + scalar;
   break;
  case 1: // sub: M-scalar vs scalar-M
   if (scalar_left)
    for (int i = 0; i < n; i++) mval[i] = scalar - mx.mval[i];
   else
    for (int i = 0; i < n; i++) mval[i] = mx.mval[i] - scalar;
   break;
  case 2: // mul: scalar*M == M*scalar
   for (int i = 0; i < n; i++) mval[i] = mx.mval[i] * scalar;
   break;
  case 3: // div: M/scalar vs scalar/M (element-wise)
   if (scalar_left)
    for (int i = 0; i < n; i++) mval[i] = scalar / mx.mval[i];
   else
    for (int i = 0; i < n; i++) mval[i] = mx.mval[i] / scalar;
   break;
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// True matrix multiplication: res = left * right
bool calculator::mxMatMul (value &res, value &left, value &right)
{
 if (left.mcols != right.mrows)
  {
   mxerror ("dimensions incompatible for multiplication");
   return false;
  }
 int rows       = left.mrows;
 int cols       = right.mcols;
 int inner      = left.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++)
   {
    float__t sum = 0.0L;
    for (int k = 0; k < inner; k++) sum += left.mval[r * inner + k] * right.mval[k * cols + c];
    mval[r * cols + c] = sum;
   }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}
// Matrix scalar and matrix->matrix functions
// All use class fields: error(), mxAlloc(), registerString()
// Scalar results return float__t (qnan on error)
// Matrix results: bool fn(value &res, value &M) — false on error

// ---------------------------------------------------------------------------
// INTERNAL: Gauss-Jordan elimination on a square matrix
// Computes inverse and determinant simultaneously.
// aug[n][2n] = [M | I], modified in place.
// Returns true on success, false if singular.
// det is accumulated during elimination.
// ---------------------------------------------------------------------------
bool calculator::mxGaussJordan (float__t *aug, int n, float__t &det)
{
 det = 1.0L;
 for (int col = 0; col < n; col++)
  {
   // find pivot row (partial pivoting for numerical stability)
   int pivot_row = -1;
   float__t best = 0.0L;
   for (int row = col; row < n; row++)
    {
     float__t v = fabsl (aug[row * 2 * n + col]);
     if (v > best)
      {
       best      = v;
       pivot_row = row;
      }
    }
   if (pivot_row < 0 || best < 1e-15L)
    {
     det = 0.0L;
     mxerror ("is singular");
     return false;
    }
   // swap rows
   if (pivot_row != col)
    {
     for (int k = 0; k < 2 * n; k++)
      {
       float__t tmp = aug[col * 2 * n + k];
       aug[col * 2 * n + k] = aug[pivot_row * 2 * n + k];
       aug[pivot_row * 2 * n + k] = tmp;
      }
     det = -det; // row swap flips sign of determinant
    }
   // accumulate determinant
   float__t pivot = aug[col * 2 * n + col];
   det *= pivot;
   // scale pivot row
   for (int k = 0; k < 2 * n; k++) aug[col * 2 * n + k] /= pivot;
   // eliminate column in all other rows
   for (int row = 0; row < n; row++)
    {
     if (row == col) continue;
     float__t factor = aug[row * 2 * n + col];
     if (factor == 0.0L) continue;
     for (int k = 0; k < 2 * n; k++) aug[row * 2 * n + k] -= factor * aug[col * 2 * n + k];
    }
  }
 return true;
}

// ---------------------------------------------------------------------------
// INTERNAL: build augmented matrix [M | I] for Gauss-Jordan
// Returns allocated float__t[n * 2n] or NULL on error
// ---------------------------------------------------------------------------
float__t *calculator::mxMakeAug (value &M)
{
 int n         = M.mrows; // must be square, caller checks
 float__t *aug = (float__t *)malloc (n * 2 * n * sizeof (float__t));
 if (!aug)
  {
   mxerror ("memory allocation failed");
   return NULL;
  }
 for (int r = 0; r < n; r++)
  {
   for (int c = 0; c < n; c++) aug[r * 2 * n + c] = M.mval[r * n + c];          // left half: M
   for (int c = 0; c < n; c++) aug[r * 2 * n + n + c] = (r == c) ? 1.0L : 0.0L; // right half: I
  }
 return aug;
}

// ---------------------------------------------------------------------------
// SCALAR FUNCTIONS
// ---------------------------------------------------------------------------

// tr(M) — trace: sum of diagonal elements, defined for any matrix (min dimension)
float__t calculator::mxTrace (value &M)
{
 int n = (M.mrows < M.mcols) ? M.mrows : M.mcols;
 float__t sum = 0.0L;
 for (int i = 0; i < n; i++) sum += M.mval[i * M.mcols + i];
 return sum;
}

// det(M) — determinant, square matrix only
float__t calculator::mxDet (value &M)
{
 if (M.mrows != M.mcols)
  {
   mxerror ("det: matrix must be square");
   return qnan;
  }
 int n = M.mrows;
 // special cases for speed
 if (n == 1) return M.mval[0];
 if (n == 2) return M.mval[0] * M.mval[3] - M.mval[1] * M.mval[2];
 float__t *aug = mxMakeAug (M);
 if (!aug) return qnan;
 float__t det = 1.0L;
 bool ok = mxGaussJordan (aug, n, det);
 free (aug);
 return ok ? det : qnan;
}

// norm(M) — Frobenius norm: sqrt(sum of squares of all elements)
float__t calculator::mxNorm (value &M)
{
 float__t sum = (float__t)0.0L;
 int n = M.mrows * M.mcols;
 for (int i = 0; i < n; i++) sum += M.mval[i] * M.mval[i];
 return Sqrt (sum);
}

float__t calculator::mxDim (value & M, t_mxDim dim)
{ 
  switch (dim)
  {
   case mxRows:
   return M.mrows;
    break;
   case mxCols:
    return M.mcols;
    break;
   case mxSize:
    return M.mrows * M.mcols;
    break;
   default:
    return 0; // should not happen, caller checks
    break;
  }
}

// ---------------------------------------------------------------------------
// MATRIX -> MATRIX FUNCTIONS
// ---------------------------------------------------------------------------

// mxInv: inverse matrix via Gauss-Jordan, result stored in res
// Used for both inv(M) function and !M operator
bool calculator::mxInv (value &res, value &M)
{
 if (M.mrows != M.mcols)
  {
   mxerror ("inv: matrix must be square");
   return false;
  }
 int n = M.mrows;
 // special case 1x1
 if (n == 1)
  {
   if (fabsl (M.mval[0]) < 1e-15L)
    {
     mxerror ("inv: matrix is singular");
     return false;
    }
   float__t *mval = mxAlloc (1, 1);
   if (!mval) return false;
   mval[0]   = 1.0L / M.mval[0];
   res.tag   = tvMATRIX;
   res.mrows = 1;
   res.mcols = 1;
   res.mval  = mval;
   return true;
  }
 float__t *aug = mxMakeAug (M);
 if (!aug) return false;
 float__t det = 1.0L;
 if (!mxGaussJordan (aug, n, det))
  {
   free (aug);
   return false; // error already set by mxGaussJordan
  }
 // extract right half of aug -> result
 float__t *mval = mxAlloc (n, n);
 if (!mval)
  {
   free (aug);
   return false;
  }
 for (int r = 0; r < n; r++)
  for (int c = 0; c < n; c++) mval[r * n + c] = aug[r * 2 * n + n + c];
 free (aug);
 res.tag   = tvMATRIX;
 res.mrows = n;
 res.mcols = n;
 res.mval  = mval;
 return true;
}

// mxElem: element-wise matrix function
bool calculator::mxElemFn (v_func fidx, value &res, value &M)
{

 if (fidx == vf_pol_rt)
  {
   return mxPolyRoots (res, M);
  }

 bool result = true;
 int rows    = M.mrows;
 int cols    = M.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval)
  {
   mxerror ("memory allocation failed");
   return false;
  }
 int n = rows * cols;
 if (fidx == vf_rand)
  {
   float__t rmax = 0;
   for (int i = 0; i < n; i++) rmax += M.mval[i] * M.mval[i];
   rmax = Sqrt (rmax)/Sqrt(n);
   for (int i = 0; i < n; i++) mval[i] = Random(rmax);
  }
 else
 for (int i = 0; i < n; i++)
  {
   switch (fidx)
    {
    case vf_abs:
     mval[i] = Abs (M.mval[i]);
     break;
    case vf_floor:
     mval[i] = Floor (M.mval[i]);
     break;
    case vf_ceil:
     mval[i] = Ceil (M.mval[i]);
     break;
    case vf_round:
     mval[i] = Round (M.mval[i]);
     break;
    case vf_frac:
     mval[i] = Frac (M.mval[i]);
     break;
    case vf_int:
     mval[i] = Int (M.mval[i]);
     break;
    case vf_float:
     mval[i] = Float (M.mval[i]);
     break;
    default:
     {
      mxerror ("Illegal operation");
      result = false;
     }
     break;
    }
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return result;
}

// Find roots of polynomial given coefficients in a row-matrix
// Coefficients ordered from highest degree to constant: [a_n, a_(n-1), ..., a_1, a_0]
// For polynomial: a_n*x^n + a_(n-1)*x^(n-1) + ... + a_1*x + a_0 = 0
// Returns a matrix of complex roots (N rows × 2 cols: [real_part, imag_part])
// Supports polynomials up to degree 4
bool calculator::mxPolyRoots (value &res, value &coeffs)
{
 // Validate input: must be a row vector (1 row, N columns)
 if (coeffs.tag != tvMATRIX || coeffs.mrows != 1)
  {
   mxerror ("polynomial coefficients must be a row vector");
   return false;
  }

 int n = coeffs.mcols; // number of coefficients
 if (n < 2)
  {
   mxerror ("polynomial must have at least 2 coefficients");
   return false;
  }

 int degree = n - 1; // polynomial degree

 // Check if leading coefficient is non-zero
 if (coeffs.mval[0] == 0.0L)
  {
   mxerror ("leading coefficient must be non-zero");
   return false;
  }

 // Normalize coefficients (divide by leading coefficient)
 float__t *c = (float__t *)alloca (n * sizeof (float__t));
 //float__t c[8]; // max degree 4 => max 5 coefficients

 for (int i = 0; i < n; i++) c[i] = coeffs.mval[i] / coeffs.mval[0];

 // Allocate result matrix: degree rows × 2 columns (real, imag)
 float__t *roots = mxAlloc (degree, 2);
 if (!roots) return false;

 // Initialize all roots to zero
 for (int i = 0; i < degree * 2; i++) roots[i] = 0.0L;

 switch (degree)
  {
  case 1: // Linear: ax + b = 0 => x = -b/a
   {
    roots[0] = -c[1]; // real part
    roots[1] = 0.0L;  // imaginary part
   }
   break;

  case 2: // Quadratic: ax^2 + bx + c = 0
   {
    float__t b  = c[1];
    float__t cc = c[2];
    float__t D  = b * b - 4.0L * cc; // discriminant

    if (D >= 0.0L)
     {
      // Two real roots
      float__t sqrtD = Sqrt (D);
      roots[0]       = (-b + sqrtD) / 2.0L; // x1 real
      roots[1]       = 0.0L;                // x1 imag
      roots[2]       = (-b - sqrtD) / 2.0L; // x2 real
      roots[3]       = 0.0L;                // x2 imag
     }
    else
     {
      // Two complex conjugate roots
      float__t sqrtD = Sqrt (-D);
      roots[0]       = -b / 2.0L;     // x1 real
      roots[1]       = sqrtD / 2.0L;  // x1 imag
      roots[2]       = -b / 2.0L;     // x2 real
      roots[3]       = -sqrtD / 2.0L; // x2 imag
     }
   }
   break;

  case 3: // Cubic: x^3 + ax^2 + bx + c = 0 (Cardano's formula)
   {
    float__t a  = c[1];
    float__t b  = c[2];
    float__t cc = c[3];

    // Convert to depressed cubic: t^3 + pt + q = 0
    // where x = t - a/3
    float__t p = b - a * a / 3.0L;
    float__t q = 2.0L * a * a * a / 27.0L - a * b / 3.0L + cc;

    // Discriminant for cubic
    float__t D = -(4.0L * p * p * p + 27.0L * q * q);

    float__t offset = -a / 3.0L;

    if (D > 0)
     {
      // Three distinct real roots (casus irreducibilis)
      float__t m   = 2.0L * Sqrt (-p / 3.0L);
      float__t phi = Acos (3.0L * q / (p * m)) / 3.0L;

      roots[0] = m * Cos (phi) + offset;
      roots[1] = 0.0L;
      roots[2] = m * Cos (phi - 2.0L * M_PI / 3.0L) + offset;
      roots[3] = 0.0L;
      roots[4] = m * Cos (phi - 4.0L * M_PI / 3.0L) + offset;
      roots[5] = 0.0L;
     }
    else
     {
      // One real root and two complex conjugate roots
      float__t delta = q * q / 4.0L + p * p * p / 27.0L;
      float__t u, v;

      if (delta >= 0)
       {
        float__t sqrtDelta = Sqrt (delta);
        float__t A         = -q / 2.0L + sqrtDelta;
        float__t B         = -q / 2.0L - sqrtDelta;

        u = (A >= 0) ? Pow (A, 1.0L / 3.0L) : -Pow (-A, 1.0L / 3.0L);
        v = (B >= 0) ? Pow (B, 1.0L / 3.0L) : -Pow (-B, 1.0L / 3.0L);
       }
      else
       {
        float__t r     = Sqrt (-p * p * p / 27.0L);
        float__t theta = Acos (-q / (2.0L * r));
        u              = 2.0L * Pow (r, 1.0L / 3.0L) * Cos (theta / 3.0L);
        v              = 0.0L;
       }

      // Real root
      roots[0] = u + v + offset;
      roots[1] = 0.0L;

      // Complex conjugate pair
      float__t re_part = -(u + v) / 2.0L + offset;
      float__t im_part = (u - v) * Sqrt (3.0L) / 2.0L;

      roots[2] = re_part;
      roots[3] = im_part;
      roots[4] = re_part;
      roots[5] = -im_part;
     }
   }
   break;

   case 4: // Quartic: x^4 + ax^3 + bx^2 + cx + d = 0 (Ferrari's method)
   {
    float__t a  = c[1];
    float__t b  = c[2];
    float__t cc = c[3];
    float__t d  = c[4];

    // Depressed quartic: y^4 + py^2 + qy + r = 0
    // where x = y - a/4
    float__t offset = -a / 4.0L;
    float__t a2     = a * a;
    float__t p      = b - 3.0L * a2 / 8.0L;
    float__t q      = a2 * a / 8.0L - a * b / 2.0L + cc;
    float__t r      = -3.0L * a2 * a2 / 256.0L + a2 * b / 16.0L - a * cc / 4.0L + d;

    if (fabsl (q) < 1e-10L)
     {
      // Biquadratic: y^4 + py^2 + r = 0
      // Substitute z = y^2: z^2 + pz + r = 0
      float__t D = p * p - 4.0L * r;

      float__t sqrtD = (D >= 0) ? Sqrt (D) : Sqrt (-D);
      float__t z1    = (-p + (D >= 0 ? sqrtD : 0.0L)) / 2.0L;
      float__t z2    = (-p - (D >= 0 ? sqrtD : 0.0L)) / 2.0L;

      if (D >= 0)
       {
        // Two real values for z
        // y = ±sqrt(z)
        if (z1 >= 0)
         {
          float__t sqrtz1 = Sqrt (z1);
          roots[0]        = sqrtz1 + offset;
          roots[1]        = 0.0L;
          roots[2]        = -sqrtz1 + offset;
          roots[3]        = 0.0L;
         }
        else
         {
          float__t sqrtz1 = Sqrt (-z1);
          roots[0]        = offset;
          roots[1]        = sqrtz1;
          roots[2]        = offset;
          roots[3]        = -sqrtz1;
         }

        if (z2 >= 0)
         {
          float__t sqrtz2 = Sqrt (z2);
          roots[4]        = sqrtz2 + offset;
          roots[5]        = 0.0L;
          roots[6]        = -sqrtz2 + offset;
          roots[7]        = 0.0L;
         }
        else
         {
          float__t sqrtz2 = Sqrt (-z2);
          roots[4]        = offset;
          roots[5]        = sqrtz2;
          roots[6]        = offset;
          roots[7]        = -sqrtz2;
         }
       }
      else
       {
        // D < 0: z is complex, so z = (-p ± i*sqrt(-D))/2
        // Both z values are complex conjugates
        float__t z_re = -p / 2.0L;
        float__t z_im = sqrtD / 2.0L;

        // For z1 = z_re + i*z_im, we need y = ±sqrt(z)
        // sqrt(a + bi) = sqrt(r)*[cos(θ/2) + i*sin(θ/2)]
        // where r = |z| = sqrt(a^2 + b^2), θ = atan2(b, a)

        float__t z1_abs = Sqrt (z_re * z_re + z_im * z_im);
        float__t z1_arg = Atan2l (z_im, z_re);
        float__t sqrt_r = Sqrt (z1_abs);

        // y1 = sqrt(z1) = sqrt_r * [cos(θ/2) + i*sin(θ/2)]
        float__t y1_re = sqrt_r * Cos (z1_arg / 2.0L);
        float__t y1_im = sqrt_r * Sin (z1_arg / 2.0L);

        roots[0] = y1_re + offset;
        roots[1] = y1_im;
        roots[2] = -y1_re + offset;
        roots[3] = -y1_im;

        // For z2 = z_re - i*z_im (complex conjugate)
        float__t z2_arg = Atan2l (-z_im, z_re);

        // y3 = sqrt(z2)
        float__t y3_re = sqrt_r * Cos (z2_arg / 2.0L);
        float__t y3_im = sqrt_r * Sin (z2_arg / 2.0L);

        roots[4] = y3_re + offset;
        roots[5] = y3_im;
        roots[6] = -y3_re + offset;
        roots[7] = -y3_im;
       }
     }
    else
     {
      // General case: solve resolvent cubic
      // m^3 - 2pm^2 + (p^2 - 4r)m + q^2 = 0
      // We need to find the largest positive real root m
      float__t ac  = -2.0L * p;
      float__t bc  = p * p - 4.0L * r;
      float__t ccc = q * q;

      // Depressed cubic for m: t^3 + pt + q = 0
      float__t pc = bc - ac * ac / 3.0L;
      float__t qc = 2.0L * ac * ac * ac / 27.0L - ac * bc / 3.0L + ccc;

      float__t delta = qc * qc / 4.0L + pc * pc * pc / 27.0L;
      float__t m;

      if (delta >= 0)
       {
        // One real root and two complex conjugate roots
        float__t sqrtDelta = Sqrt (delta);
        float__t A         = -qc / 2.0L + sqrtDelta;
        float__t B         = -qc / 2.0L - sqrtDelta;

        float__t u = (A >= 0) ? Pow (A, 1.0L / 3.0L) : -Pow (-A, 1.0L / 3.0L);
        float__t v = (B >= 0) ? Pow (B, 1.0L / 3.0L) : -Pow (-B, 1.0L / 3.0L);

        m = u + v - ac / 3.0L;
       }
      else
       {
        // Three real roots - choose the largest positive one
        float__t rr     = Sqrt (-pc * pc * pc / 27.0L);
        float__t theta  = Acos (-qc / (2.0L * rr));
        float__t cbrt_r = Pow (rr, 1.0L / 3.0L);

        float__t m1 = 2.0L * cbrt_r * Cos (theta / 3.0L) - ac / 3.0L;
        float__t m2 = 2.0L * cbrt_r * Cos ((theta - 2.0L * M_PI) / 3.0L) - ac / 3.0L;
        float__t m3 = 2.0L * cbrt_r * Cos ((theta - 4.0L * M_PI) / 3.0L) - ac / 3.0L;

        // Choose the largest positive root
        m = m1;
        if (m2 > m && m2 > 0) m = m2;
        if (m3 > m && m3 > 0) m = m3;
       }

      // Ensure m is positive (or at least non-negative)
      if (m < 0.0L) m = 0.0L;

      // Now solve two quadratics using the resolvent root m
      // y^4 + py^2 + qy + r = (y^2 + s1*y + t1)(y^2 + s2*y + t2)
      // where s1 = sqrt(m), s2 = -sqrt(m)
      // and t1, t2 are determined from p, q, r

      float__t sqrtm = Sqrt (m);

      // From Ferrari's method:
      // s1 + s2 = 0 => s1 = sqrt(m), s2 = -sqrt(m)
      // t1 + t2 = p + m
      // s1*t2 + s2*t1 = q => sqrt(m)*(t2 - t1) = q
      // t1*t2 = r

      float__t t_sum  = p + m;
      float__t t_diff = (sqrtm != 0.0L) ? q / sqrtm : 0.0L;

      float__t t1 = (t_sum - t_diff) / 2.0L;
      float__t t2 = (t_sum + t_diff) / 2.0L;

      // First quadratic: y^2 + sqrt(m)*y + t1 = 0
      float__t D1 = sqrtm * sqrtm - 4.0L * t1;
      if (D1 >= 0)
       {
        float__t sqrtD1 = Sqrt (D1);
        roots[0]        = (-sqrtm + sqrtD1) / 2.0L + offset;
        roots[1]        = 0.0L;
        roots[2]        = (-sqrtm - sqrtD1) / 2.0L + offset;
        roots[3]        = 0.0L;
       }
      else
       {
        float__t sqrtD1 = Sqrt (-D1);
        roots[0]        = -sqrtm / 2.0L + offset;
        roots[1]        = sqrtD1 / 2.0L;
        roots[2]        = -sqrtm / 2.0L + offset;
        roots[3]        = -sqrtD1 / 2.0L;
       }

      // Second quadratic: y^2 - sqrt(m)*y + t2 = 0
      float__t D2 = sqrtm * sqrtm - 4.0L * t2;
      if (D2 >= 0)
       {
        float__t sqrtD2 = Sqrt (D2);
        roots[4]        = (sqrtm + sqrtD2) / 2.0L + offset;
        roots[5]        = 0.0L;
        roots[6]        = (sqrtm - sqrtD2) / 2.0L + offset;
        roots[7]        = 0.0L;
       }
      else
       {
        float__t sqrtD2 = Sqrt (-D2);
        roots[4]        = sqrtm / 2.0L + offset;
        roots[5]        = sqrtD2 / 2.0L;
        roots[6]        = sqrtm / 2.0L + offset;
        roots[7]        = -sqrtD2 / 2.0L;
       }
     }
   }
   break;
   default: // Polynomial degree > 4: use Durand-Kerner method (numerical)
    {
     // Durand-Kerner (Weierstrass) method for finding all roots simultaneously
     const int max_iterations = 100;
     const float__t tolerance = 1e-12L;

     // Allocate temporary arrays for current and next approximations
     float__t *roots_re = (float__t *)alloca (degree * sizeof (float__t));
     float__t *roots_im = (float__t *)alloca (degree * sizeof (float__t));
     float__t *next_re  = (float__t *)alloca (degree * sizeof (float__t));
     float__t *next_im  = (float__t *)alloca (degree * sizeof (float__t));

     // Initialize with spiral pattern: r_k = r^k * e^(2πik/n)
     // where r = max(1, |a_(n-1)/a_n|^(1/n)) to get good initial radius
     float__t init_radius = 1.0L;
     if (degree > 1 && fabsl (c[1]) > 0.0L) init_radius = Pow (fabsl (c[1]), 1.0L / degree);
     if (init_radius < 1.0L) init_radius = 1.0L;

     for (int k = 0; k < degree; k++)
      {
       float__t angle = 2.0L * M_PI * k / degree;
       float__t r     = init_radius * Pow (0.9L + 0.1L * k / degree, (float__t)k);
       roots_re[k]    = r * Cos (angle);
       roots_im[k]    = r * Sin (angle);
      }

     // Durand-Kerner iterations
     bool converged = false;
     for (int iter = 0; iter < max_iterations && !converged; iter++)
      {
       converged = true;

       // For each root approximation
       for (int i = 0; i < degree; i++)
        {
         // Evaluate P(z_i) where P is the polynomial
         float__t p_re = c[0];
         float__t p_im = 0.0L;

         for (int j = 1; j <= degree; j++)
          {
           // Multiply by z_i: (p_re + i*p_im) * (roots_re[i] + i*roots_im[i])
           float__t temp_re = p_re * roots_re[i] - p_im * roots_im[i];
           float__t temp_im = p_re * roots_im[i] + p_im * roots_re[i];

           // Add coefficient c[j]
           p_re = temp_re + c[j];
           p_im = temp_im;
          }

         // Compute product of (z_i - z_j) for all j ≠ i
         float__t prod_re = 1.0L;
         float__t prod_im = 0.0L;

         for (int j = 0; j < degree; j++)
          {
           if (j != i)
            {
             // (z_i - z_j)
             float__t diff_re = roots_re[i] - roots_re[j];
             float__t diff_im = roots_im[i] - roots_im[j];

             // Multiply: prod *= diff
             float__t temp_re = prod_re * diff_re - prod_im * diff_im;
             float__t temp_im = prod_re * diff_im + prod_im * diff_re;

             prod_re = temp_re;
             prod_im = temp_im;
            }
          }

         // Divide P(z_i) by product: correction = P(z_i) / prod
         float__t denom = prod_re * prod_re + prod_im * prod_im;

         if (denom > 1e-30L)
          {
           float__t corr_re = (p_re * prod_re + p_im * prod_im) / denom;
           float__t corr_im = (p_im * prod_re - p_re * prod_im) / denom;

           // Update: z_i = z_i - correction
           next_re[i] = roots_re[i] - corr_re;
           next_im[i] = roots_im[i] - corr_im;

           // Check convergence
           float__t change = Sqrt (corr_re * corr_re + corr_im * corr_im);
           if (change > tolerance) converged = false;
          }
         else
          {
           // Product too small (roots too close), keep current value
           next_re[i] = roots_re[i];
           next_im[i] = roots_im[i];
          }
        }

       // Copy next to current
       for (int i = 0; i < degree; i++)
        {
         roots_re[i] = next_re[i];
         roots_im[i] = next_im[i];
        }
      }

     // Clean up near-zero imaginary parts (numerical noise)
     for (int i = 0; i < degree; i++)
      {
       if (fabsl (roots_im[i]) < tolerance * fabsl (roots_re[i])) roots_im[i] = 0.0L;
      }

     // Sort roots: real roots first (sorted by real part), then complex pairs
     // Use simple bubble sort (degree is small)
     for (int i = 0; i < degree - 1; i++)
      {
       for (int j = i + 1; j < degree; j++)
        {
         bool swap = false;

         // Real roots come before complex
         bool i_is_real = (fabsl (roots_im[i]) < tolerance);
         bool j_is_real = (fabsl (roots_im[j]) < tolerance);

         if (j_is_real && !i_is_real)
          swap = true;
         else if (i_is_real && j_is_real)
          {
           // Both real: sort by real part
           if (roots_re[j] < roots_re[i]) swap = true;
          }
         else if (!i_is_real && !j_is_real)
          {
           // Both complex: sort by real part, then by imaginary part
           if (roots_re[j] < roots_re[i])
            swap = true;
           else if (fabsl (roots_re[j] - roots_re[i]) < tolerance && roots_im[j] < roots_im[i])
            swap = true;
          }

         if (swap)
          {
           // Swap roots[i] and roots[j]
           float__t temp = roots_re[i];
           roots_re[i]   = roots_re[j];
           roots_re[j]   = temp;

           temp        = roots_im[i];
           roots_im[i] = roots_im[j];
           roots_im[j] = temp;
          }
        }
      }

     // Copy results to output matrix
     for (int i = 0; i < degree; i++)
      {
       roots[i * 2]     = roots_re[i];
       roots[i * 2 + 1] = roots_im[i];
      }
    }
    break;
   }

 res.tag   = tvMATRIX;
 res.mrows = degree;
 res.mcols = 2;
 res.mval  = roots;
 return true;
}

double calculator::Median (const char *fname, const char *msk,double totalN, double minV, double maxV)
{
 double low         = minV;
 double high        = maxV;
 double targetCount = totalN / 2.0;
 double mid         = low;

 // 45-50 iterations are sufficient to exhaust the precision of double
 for (int i = 0; i < 50; i++)
  {
   mid                 = (low + high) / 2.0;
   double currentCount = 0;

   FILE *f = nullptr;
   f=fopen (fname, "r");
   if (f)
    {
     char line[1024];
     double v = qnan;
     while (fgets (line, sizeof (line), f))
      {
       // Use "all-purpose" scanner
       if (strscan (line,msk, 1, &v) == 1)
        {
         if (v <= mid) currentCount++;
        }
      }
     fclose (f);
    }
   else
    {
     mxerror ("cannot open data file");
     return qnan;
    }

   if (currentCount <= targetCount)
    low = mid;
   else
    high = mid;

   // If the interval becomes negligibly small, exit
   if (fabs (high - low) < 1e-9) break;
  }
 return mid;
}

// StatFn: compute statistical function on a column of numbers read from a file
float__t calculator::StatFn (const char *fname, const char *msk, sfntype sfn, float__t x)
{
 FILE *f    = nullptr;
 double n = 0, sumX = 0, sumX2 = 0, mean = 0, M2 = 0;
 double minVal = qnan, maxVal = qnan;

 f = fopen(fname, "r");
 if (f)
  {
   char line[1024];
   while (fgets (line, sizeof (line), f))
    {
     double xx = qnan;
     if (strscan (line, msk, 1, &xx) == 1)
      { // Read only the first number in the line
       if (n == 0)
        {
         minVal = maxVal = xx;
        }
       else
        {
         if (xx < minVal) minVal = xx;
         if (xx > maxVal) maxVal = xx;
        }
       // Accumulation (Welford's method or simple sums)
       n++;
       double delta = xx - mean;
       mean += delta / n;
       M2 += delta * (xx - mean);
       sumX += xx;
       sumX2 += xx * xx;
      }
    }
   fclose (f);
  }
 else
  {
   mxerror ("cannot open data file");
   return qnan;
  }

 switch (sfn)
  {
  case sfNum: // Number of values
   return (float__t)n;
  case sfMean: // Arithmetic mean
   return (float__t)mean;
  case sfMedian: // Median (not implemented, requires storing all values)
   return (float__t)Median(fname, msk, n, minVal, maxVal);
  case sfRMS:                             // Root mean square
   return (float__t)((n > 0) ? sqrt (sumX2 / n) : 0.0L); // Root mean square 
  case sfSumX:                            // Sum of values
   return (float__t)sumX;
  case sfStdDevP:         // Population standard deviation
   return (float__t)((n > 0) ? sqrt (M2 / n) : 0.0L); // Population standard deviation (sigma)
  case sfStdDevS:         // Sample standard deviation
   return (float__t)((n > 1) ? sqrt (M2 / (n - 1)) : 0.0L); // Sample standard deviation (s)
  case sfMin:             // Minimum value
   return (float__t)minVal;
  case sfMax:             // Maximum value
   return (float__t)maxVal;

  case sfNormP: // Probability P(t): P(X <= x)
  case sfNormQ: // Probability Q(t): P(0 <= X <= t) or P(mean <= X <= x)
  case sfNormR: // Probability R(t): P(X > x)
   {
    float__t m = (float__t)mean;
    float__t s = (float__t)((n > 1) ? sqrt (M2 / (n - 1)) : 0.0L);

    if (s == 0)
     {
      if (sfn == sfNormP) return (x >= m) ? 1.0 : 0.0;
      if (sfn == sfNormQ) return 0.0;
      if (sfn == sfNormR) return (x > m) ? 0.0 : 1.0;
     }

    // 1. Evaluate Z-score (normalized deviation)
    // t = (x - mean) / sigma
    float__t t_score = (x - m) / s;

    // 2. Compute P(t) - basic integral function
    // Using Erf(t / sqrt(2))
    float__t p_val = 0.5 * (1.0 + Erf (t_score / sqrt (2.0)));

    if (sfn == sfNormP) return p_val;

    if (sfn == sfNormQ)
     {
      // Q(t) in Casio — this is the probability that the value lies between
      // the mean and x. Mathematically, this is |P(t) - 0.5|
      return fabsl (p_val - 0.5);
     }

    if (sfn == sfNormR)
     {
      // R(t) — this is the "right tail", the probability that the value > x
      // Mathematically, this is 1.0 - P(t)
      return 1.0 - p_val;
     }
   }
   break;
  case sfInvNorm: // Inverse Normal Distribution
   {
    // x here — this is the probability P (from 0 to 1) provided by the user
    float__t P = x;
    if (P <= 0.0L || P >= 1.0L)
     {
      mxerror ("probability must be between 0 and 1");
      return qnan;
     }

    float__t m = (float__t)mean;
    float__t s = (float__t)((n > 1) ? Sqrt (M2 / (n - 1)) : 0.0L);

    // 1. Find the Z-score using erfinv
    // Use Erfinv_refine for higher accuracy if needed
    float__t z = Sqrt (2.0) * Erfinv (2.0 * P - 1.0);
    //float__t z = Sqrt (2.0) * Erfinv_refine (2.0 * P - 1.0, 50);

    // 2. Denormalize: x = mu + z * sigma
    return m + z * s;
   }
  case sfNormPD:
   {
    float__t m = (float__t)mean;
    float__t s = (float__t)((n > 1) ? Sqrt (M2 / (n - 1)) : 0.0L);
    if (s == 0) return (x == m) ? inf : 0.0;

    float__t diff = x - m;
    // Classical Gaussian formula
    return (1.0 / (s * Sqrt (M_2PI))) * Exp (-(diff * diff) / (2.0 * s * s));
   }
  default:
   return (float__t)0.0L;
  }
 return qnan; // Should never reach here
}

bool calculator::mxRegrFn (const char *fname, const char *msk, int n, rtype rt, value &res)
{
 FILE *f = nullptr;
 
 if (n<=0)
  {
   mxerror ("degree must be positive");
   return false;
  }
 // Linearized types always have degree 1 (line y=a+bx)
 int degree = (rt == rtPoly) ? n : 1;

 if (degree > 6)
  {
   mxerror ("degree too big (max 6)");
   return false;
  }

 // Initialize accumulators on the temporary stack
 int s_size   = 2 * degree + 1;
 int sy_size  = degree + 1;
 float__t *S  = (float__t *)alloca (s_size * sizeof (float__t));
 float__t *SY = (float__t *)alloca (sy_size * sizeof (float__t));

 for (int i = 0; i < s_size; i++) S[i] = 0;
 for (int i = 0; i < sy_size; i++) SY[i] = 0;

 // 1. Stream reading of the file and accumulation of sums
 f = fopen (fname, "r");
 if (f)
  {
   char line[1024];
   while (fgets (line, sizeof (line), f))
    {
     double xd = qnan, yd = qnan;
     // strscan ignores garbage and understands suffixes (100k, 5m)
     if (strscan (line, msk, 2, &xd, &yd) == 2)
      {
       float__t x = (float__t)xd;
       float__t y = (float__t)yd;
       // Linearization of data before least squares
       switch (rt)
        {
        case rtExp:
         if (y <= 0) continue;
         y = Log (y);
         break;
        case rtLg:
         if (x <= 0) continue;
         x = Log (x);
         break;
        case rtPow:
         if (x <= 0 || y <= 0) continue;
         x = Log (x);
         y = Log (y);
         break;
        case rtInv:
         if (x == 0) continue;
         x = (float__t)(1.0L / x);
         break;
        default:
         break;
        }

       // Accumulate sums of powers
       float__t px = (float__t)1.0L;
       for (int i = 0; i < s_size; i++)
        {
         S[i] += px;
         if (i < sy_size) SY[i] += y * px;
         px *= x;
        }
      }
    }
   fclose (f);
  }
 else
  {
   mxerror ("cannot open data file");
   return false;
  }

 // 2. Forming matrices to solve the system M * A = B
 value valM, valB, valInvM, valCoeff;
 int dim = degree + 1;

 valM.tag   = tvMATRIX;
 valM.mrows = dim;
 valM.mcols = dim;
 valM.mval  = mxAlloc (dim, dim);

 valB.tag   = tvMATRIX;
 valB.mrows = dim;
 valB.mcols = 1;
 valB.mval  = mxAlloc (dim, 1);

 for (int r = 0; r < dim; r++)
  {
   for (int c = 0; c < dim; c++)
    {
     valM.mval[r * dim + c] = S[r + c];
    }
   valB.mval[r] = SY[r];
  }

 // 3. Solving the system using internal functions
 // A = inv(M) * B
 if (!mxInv (valInvM, valM))
  {
   mxerror ("Matrix is singular (not enough distinct points?)");
   return false;
  }

 if (!mxMatMul (valCoeff, valInvM, valB)) return false;

 // 4. Post-processing and preparing the result
 // mval for the result (matrix-row 1 x n+1)
 float__t *res_mval = mxAlloc (1, dim);

 // Transfer coefficients in order from highest degree to lowest (as in polynomial)
 for (int i = 0; i < dim; i++)
  {
   res_mval[i] = valCoeff.mval[degree - i];
  }

 // Inverse transformation for exponential and power types: ln(a) -> a
 if (rt == rtExp || rt == rtPow)
  {
   // Coefficient 'a' is at the end of the res_mval array (corresponds to a0)
   res_mval[dim - 1] = expl (res_mval[dim - 1]);
  }

 res.tag   = tvMATRIX;
 res.mrows = 1;
 res.mcols = dim;
 res.mval  = res_mval;

 return true;
}

// mxCalcFn: calculate the value of the regression function at a given x using the coefficients in M
float__t calculator::mxCalcFn(value M, rtype rt, float__t x)
{
  if (M.tag != tvMATRIX)
   {
    mxerror ("matrix required");
    return qnan;
   }
 
  switch (rt)
   {
    case rtPoly:
    {
     if (M.mrows == 1 && M.mcols <= MAX_C)
      {
       // Coefficients ordered from highest degree to constant: [a_n, a_(n-1), ..., a_1, a_0]
       // For polynomial: a_n*x^n + a_(n-1)*x^(n-1) + ... + a_1*x + a_0 = 0
       // Horner's method for evaluating polynomial at x 
       float__t result = 0.0L;
       for (int i = 0; i < M.mcols; i++)
        {
         result = result * x + M.mval[i];
        }
       return result;
      }
     else
     {
       mxerror ("polynomial coefficients must be a row vector");
       return qnan;
     }
    }
    break;
    case rtExp: //y=a*exp(b*x), [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a * Exp (b * x);
       }
      else
       {
        mxerror ("exponential regression requires exactly 2 coefficients");
        return qnan;
       }
     }
     break;
     case rtPow: //y=a*x^b, [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a * Pow (x, b);
       }
      else
       {
        mxerror ("power regression requires exactly 2 coefficients");
        return qnan;
       }
     }
     break;
     case rtLg: // y=a+b*ln(x), [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a + b * Log (x);
       }
      else
       {
        mxerror ("logarithmic regression requires exactly 2 coefficients");
        return qnan;
       }
     }
    break;
    case rtInv: // y=a+b/x, [b, a]
     {
      if (M.mrows == 1 && M.mcols == 2)
       {
        float__t a = M.mval[1]; // constant term (a0)
        float__t b = M.mval[0]; // linear term (a1)
        return a + b / x;
       }
      else
       {
        mxerror ("inverse regression requires exactly 2 coefficients");
        return qnan;
       }
     }
     break;
   }
  return qnan; // Should not reach here
}

// mxNeg: element-wise negation (unary minus) — also in matrixuno, here for completeness
bool calculator::mxNeg (value &res, value &M)
{
 int rows = M.mrows;
 int cols = M.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = -M.mval[i];
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxTranspose: transpose — also in matrixuno via ~, here for completeness
bool calculator::mxTranspose (value &res, value &M)
{
 int rows = M.mrows;
 int cols = M.mcols;
 float__t *mval = mxAlloc (cols, rows); // note swapped dimensions
 if (!mval) return false;
 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++) mval[c * rows + r] = M.mval[r * cols + c];
 res.tag   = tvMATRIX;
 res.mrows = cols;
 res.mcols = rows;
 res.mval  = mval;
 return true;
}

// mxZeros: matrix of zeros
bool calculator::mxZeros (value &res, int rows, int cols)
{
 if (rows <= 0 || cols <= 0)
  {
   mxerror ("dimensions must be positive");
   return false;
  }
 if (rows > MAX_R || cols > MAX_C)
  {
   mxerror ("dimensions too large");
   return false;
  }
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) 
  {
   mxerror ("out of memory");
   return false;
  }
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = (float__t)0.0L;
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxDiag: diagonal matrix 
bool calculator::mxDiag (value &res, int rows, int cols)
{
 if (rows <= 0 || cols <= 0)
  {
   mxerror ("dimensions must be positive");
   return false;
  }
 if (rows > MAX_R || cols > MAX_C)
  {
   mxerror ("dimensions too large");
   return false;
  }
 float__t *mval = mxAlloc (rows, cols);
 if (!mval)
  {
   mxerror ("out of memory");
   return false;
  }
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = (float__t)0.0L;
 for (int i = 0; i < rows && i < cols; i++) mval[i * cols + i] = (float__t)1.0L;
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxDot: dot product of two vectors (1xN or Nx1)
// Returns scalar result in res.fval
bool calculator::mxDot (value & res, value & A, value & B)
{
 if (A.tag != tvMATRIX || B.tag != tvMATRIX)
  {
   mxerror ("dot: both arguments must be matrices");
   return false;
  }
 // both must be vectors (one dimension == 1) and same total size
 bool aVec = (A.mrows == 1 || A.mcols == 1);
 bool bVec = (B.mrows == 1 || B.mcols == 1);
 if (!aVec || !bVec)
  {
   mxerror ("dot: arguments must be vectors (1xN or Nx1)");
   return false;
  }
 int na = A.mrows * A.mcols;
 int nb = B.mrows * B.mcols;
 if (na != nb)
  {
   mxerror ("dot: vector sizes must match");
   return false;
  }
 float__t sum = 0.0L;
 for (int i = 0; i < na; i++) sum += A.mval[i] * B.mval[i];
 res.tag   = tvFLOAT;
 res.fval  = sum;
 res.imval = ((float__t)0.0L);
 return true;
}

// mxCross: cross product of two 3D vectors (1x3 or 3x1)
// Result is a vector of the same shape as A
bool calculator::mxCross (value & res, value & A, value & B)
{
 if (A.tag != tvMATRIX || B.tag != tvMATRIX)
  {
   mxerror ("cross: both arguments must be matrices");
   return false;
  }
 bool aVec = (A.mrows == 1 || A.mcols == 1);
 bool bVec = (B.mrows == 1 || B.mcols == 1);
 if (!aVec || !bVec)
  {
   mxerror ("cross: arguments must be vectors (1x3 or 3x1)");
   return false;
  }
 int na = A.mrows * A.mcols;
 int nb = B.mrows * B.mcols;
 if (na != 3 || nb != 3)
  {
   mxerror ("cross: cross product requires 3-element vectors");
   return false;
  }
 float__t *mval = mxAlloc (A.mrows, A.mcols); // same shape as A
 if (!mval) return false;
 mval[0]   = A.mval[1] * B.mval[2] - A.mval[2] * B.mval[1];
 mval[1]   = A.mval[2] * B.mval[0] - A.mval[0] * B.mval[2];
 mval[2]   = A.mval[0] * B.mval[1] - A.mval[1] * B.mval[0];
 res.tag   = tvMATRIX;
 res.mrows = A.mrows;
 res.mcols = A.mcols;
 res.mval  = mval;
 return true;
}
// ---------------------------------------------------------------------------
// matrixbin — binary operations
// ---------------------------------------------------------------------------
t_mresult calculator::matrixbin (value &res, value &left, value &right, t_operator cop)
{
 if (left.tag != tvMATRIX && right.tag != tvMATRIX) return mrSKIP; // not a matrix operation

 bool lm = (left.tag == tvMATRIX);
 bool rm = (right.tag == tvMATRIX);
 bool ls = left.is_scalar ();
 bool rs = right.is_scalar ();

 switch (cop)
  {
  // ---- ADD ----
  case toADD:
  case toSETADD:
   if (lm && rm)
    {
     if (!mxElemOp (res, left, right, 0)) return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 0, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 0, true)) return mrERROR;
    }
   return mrDONE;

  // ---- SUB ----
  case toSUB:
  case toSETSUB:
   if (lm && rm)
    {
     if (!mxElemOp (res, left, right, 1)) return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 1, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 1, true)) return mrERROR;
    }
   return mrDONE;

  // ---- MUL ----
  case toMUL:
  case toSETMUL:
   if (lm && rm)
    {
     if (!mxMatMul (res, left, right)) return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 2, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 2, true)) return mrERROR;
    }
   return mrDONE;

  // ---- DIV ----
  case toDIV:
  case toSETDIV:
   if (lm && rm)
    {
     mxerror ("division is not defined (use inv())");
     return mrERROR;
    }
   else if (lm)
    {
     if (!mxScalarOp (res, left, right.fval, 3, false)) return mrERROR;
    }
   else
    {
     if (!mxScalarOp (res, right, left.fval, 3, true)) return mrERROR;
    }
   return mrDONE;

  // ---- POW ----
  case toPOW:
  case toSETPOW:
   // M^n — repeated multiplication, only integer n, square matrix
   if (lm && rs)
    {
     if (left.mrows != left.mcols)
      {
       mxerror ("Matrix power requires a square matrix");
       return mrERROR;
      }
     long long n = (long long)right.fval;
     if ((float__t)n != right.fval || n < 0)
      {
       mxerror ("Matrix power requires a non-negative integer exponent");
       return mrERROR;
      }
     // start with identity matrix
     // deep copy of left before we start writing to res
     // (res and left may alias the same v_stack entry)
     int sz = left.mrows;
     int sz2 = sz * sz;
     float__t *curbuf = (float__t *)malloc (sz2 * sizeof (float__t));
     if (!curbuf)
      {
       mxerror ("memory allocation failed");
       return mrERROR;
      }
     memcpy (curbuf, left.mval, sz2 * sizeof (float__t));
     value cur = left;
     cur.mval  = curbuf; // cur now has its own independent copy

     // start with identity matrix
     float__t *mval = mxAlloc (sz, sz);
     if (!mval)
      {
       free (curbuf);
       return mrERROR;
      }
     for (int r = 0; r < sz; r++)
      for (int c = 0; c < sz; c++) mval[r * sz + c] = (r == c) ? 1.0L : 0.0L;
     res.tag   = tvMATRIX;
     res.mrows = sz;
     res.mcols = sz;
     res.mval  = mval;

     for (long long i = 0; i < n; i++)
      {
       float__t *tmpbuf = (float__t *)malloc (sz2 * sizeof (float__t));
       if (!tmpbuf)
        {
         free (curbuf);
         mxerror ("memory allocation failed");
         return mrERROR;
        }
       memcpy (tmpbuf, res.mval, sz2 * sizeof (float__t));
       value tmp = res;
       tmp.mval  = tmpbuf;
       if (!mxMatMul (res, tmp, cur))
        {
         free (tmpbuf);
         free (curbuf);
         return mrERROR;
        }
       free (tmpbuf);
      }
     free (curbuf);
     return mrDONE;
    }
   error ("Matrix power: left must be matrix, right must be non-negative integer scalar");
   return mrERROR;

  // ---- parallel resistors M//M or M//scalar ----
  case toPAR:
   if (lm && rm)
    {
     // element-wise: (a*b)/(a+b)
     if (left.mrows != right.mrows || left.mcols != right.mcols)
      {
       mxerror ("dimensions must match for // operator");
       return mrERROR;
      }
     int rows = left.mrows;
     int cols = left.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.mval[i] * right.mval[i]) / (left.mval[i] + right.mval[i]);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   else if (lm && rs)
    {
     int rows = left.mrows;
     int cols = left.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.mval[i] * right.fval) / (left.mval[i] + right.fval);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   else if (ls && rm)
    {
     int rows = right.mrows;
     int cols = right.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.fval * right.mval[i]) / (left.fval + right.mval[i]);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   return mrSKIP;

  // ---- comparison, bitwise, shifts — not defined for matrices ----
  case toEQ:
  case toNE:
   if (lm && rm)
    {
     if (left.mrows != right.mrows || left.mcols != right.mcols)
      {
       error ("Matrix dimensions must match for comparison");
       return mrERROR;
      }
     int n = left.mrows * left.mcols;
     bool equal = true;
     for (int i = 0; i < n; i++)
      if (left.mval[i] != right.mval[i])
       {
        equal = false;
        break;
       }
     res.tag  = tvINT;
     res.ival = (cop == toEQ) ? (equal ? 1 : 0) : (equal ? 0 : 1);
     return mrDONE;
    }
   else if (lm && rs)
    {
     // A==scalar: true if ALL elements equal scalar
     int n      = left.mrows * left.mcols;
     bool equal = true;
     for (int i = 0; i < n; i++)
      if (left.mval[i] != right.fval)
       {
        equal = false;
        break;
       }
     res.tag  = tvINT;
     res.ival = (cop == toEQ) ? (equal ? 1 : 0) : (equal ? 0 : 1);
     return mrDONE;
    }
   else if (ls && rm)
    {
     // scalar==A: same
     int n = right.mrows * right.mcols;
     bool equal = true;
     for (int i = 0; i < n; i++)
      if (right.mval[i] != left.fval)
       {
        equal = false;
        break;
       }
     res.tag  = tvINT;
     res.ival = (cop == toEQ) ? (equal ? 1 : 0) : (equal ? 0 : 1);
     return mrDONE;
    }
   return mrSKIP;
  case toGT:
  case toGE:
  case toLT:
  case toLE:
  case toAND:
  case toOR:
  case toXOR:
  case toASL:
  case toASR:
  case toLSR:
  case toSETOR:
  case toSETXOR:
  case toSETASL:
  case toSETASR:
  case toSETLSR:
   mxerror ("operation not defined");
   return mrERROR;

  default:
   break;
  }
 return mrSKIP;
}

// ---------------------------------------------------------------------------
// matrixuno — unary operations
// ---------------------------------------------------------------------------
t_mresult calculator::matrixuno (value &res, value &operand, t_operator cop)
{
 if (operand.tag != tvMATRIX) return mrSKIP;

 int rows = operand.mrows;
 int cols = operand.mcols;
 int n    = rows * cols;

 switch (cop)
  {
  // ---- unary minus ----
  case toMINUS:
   {
    float__t *mval = mxAlloc (rows, cols);
    if (!mval) return mrERROR;
    for (int i = 0; i < n; i++) mval[i] = -operand.mval[i];
    res.tag   = tvMATRIX;
    res.mrows = rows;
    res.mcols = cols;
    res.mval  = mval;
    return mrDONE;
   }

  // ---- unary plus — no-op ----
  case toPLUS:
   res = operand;
   return mrDONE;

  // ---- ~ transpose (for real matrix = conjugate transpose) ----
  case toCOM:
   {
    float__t *mval = mxAlloc (cols, rows); // note: transposed dimensions
    if (!mval) return mrERROR;
    for (int r = 0; r < rows; r++)
     for (int c = 0; c < cols; c++) mval[c * rows + r] = operand.mval[r * cols + c];
    res.tag   = tvMATRIX;
    res.mrows = cols; // transposed
    res.mcols = rows;
    res.mval  = mval;
    return mrDONE;
   }

  // ---- ! logical not — not defined ----
  case toNOT:
   {
    if (!mxInv (res, operand)) return mrERROR; 
    res.tag   = tvMATRIX;
    return mrDONE;
   }

  default:
   mxerror ("not defined for matrices");
   return mrERROR;
   break;
  }
 return mrSKIP;
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
 calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
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
 calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
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

 float__t *mval = (float__t *)sf_alloc (rows * cols * sizeof (float__t));

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
       error ("Duplicate name");
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
   if ((sym->tag == tsSOLVE)    // solve (x(2x+2)-2,x:=0)
       || (sym->tag == tsCALC)) // calc (x(2x+2)-2,x:=0)
    {
     if (parenthesis_count == 0 && comma_count == 1)
      {
       v_stack[v_sp].tag = tvSOLVE;
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
   else if (sym->tag == tsPLOT) // plot(fname, expr)
    {
     bool error_in_args = false;
     switch (sym->fidx)
      {
      case pl_fplot:    // fplot(file, expr, from, to, var)
      case pl_oplot:    // oplot(file, expr, from, to, var)
      case pl_fplotpol: // fplotpol(file, expr, from, to, var)
      case pl_oplotpol: // oplotpol(file, expr, from, to, var)
      case pl_fplotlgx:
      case pl_oplotlgx:
      case pl_fplotlgy:
      case pl_oplotlgy:
      case pl_fplotlgxy:
      case pl_oplotlgxy:
      case pl_fplotsmith: // fplotsmith(file, expr, from, to, var)
      case pl_oplotsmith: // oplotsmith(file, expr, from, to, var)
       if (parenthesis_count == 0 && comma_count == 4)
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      case pl_plot:    // plot(expr, from, to, var)
      case pl_plotpol: // plotpol(expr, from, to, var)
      case pl_plotlgx:
      case pl_plotlgy:
      case pl_plotlgxy:
      case pl_plotsmith: // plotsmith(expr, from, to, var)
       if (parenthesis_count == 0 && comma_count == 3)
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      case pl_xyplot: // xyplot(xexpr, yexpr, from, to, var)
       if (parenthesis_count == 0 && comma_count == 4)
        v_stack[v_sp].tag = tvPLOT;
       else
        error_in_args = true;
       break;

      case pl_fxyplot: // fxyplot(file, xexpr, yexpr, from, to, var)
      case pl_oxyplot: // oxyplot(file, xexpr, yexpr, from, to, var)
       if (parenthesis_count == 0 && comma_count == 5)
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

      case pl_fplotsmithz: // fplotsmithz(file, expr, from, to, var, Z0)
      case pl_oplotsmithz: // oplotsmithz(file, expr, from, to, var, Z0)
       if (parenthesis_count == 0 && comma_count == 5)
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

      case pl_fplotdata:  // fplotdata(bmpfile, datafile, mask)
      case pl_oplotdata:  // oplotdata(bmpfile, datafile, mask)
      case pl_fplotdatal: // fplotdatal(bmpfile, datafile, mask)
      case pl_oplotdatal: // oplotdatal(bmpfile, datafile, mask)
       if (parenthesis_count == 0 && comma_count <= 2)
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
          ival               = *(unsigned char *)(ipos - 1);
          v_stack[v_sp].sval = (char *)sf_alloc (2);
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
#pragma endregion
//---------------------------------------------------------------------------

#pragma region Expression Evaluation
// Left precedence for operators, used to determine when to push operators onto the stack during
// expression evaluation. Higher values indicate higher precedence.
static int lpr[toTERMINALS] = {
 2,  0,   0,  0,              // BEGIN, OPERAND, ERROR, END,
 4,  4,                       // LPAR, RPAR
 5,  5,   98, 98, 98,         // FUNC, SOLVE, POSTINC, POSTDEC, FACT
 98, 98,  98, 98, 98, 98,     // PREINC, PREDEC, PLUS, MINUS, NOT, COM,
 95,                          // POW,
 80, 80,  80, 80, 80,         // toPERCENT, MUL, DIV, MOD, PAR
 70, 70,                      // ADD, SUB,
 60, 60,  60,                 // ASL, ASR, LSR,
 50, 50,  50, 50,             // GT, GE, LT, LE,
 40, 40,                      // EQ, NE,
 38,                          // AND,
 36,                          // XOR,
 34, 100,                     // OR, MX_ELEM
 20, 20,  20, 20, 20, 20, 20, // SET, SETADD, SETSUB, SETMUL, SETDIV, SETMOD,
 20, 20,  20, 20, 20, 20,     // SETASL, SETASR, SETLSR, SETAND, SETXOR, SETOR,
 8,                           // SEMI
 10                           // COMMA
};

// Right precedence for operators, used to determine when to pop operators from the stack during
// expression evaluation. Higher values indicate higher precedence.
static int rpr[toTERMINALS] = {
 0,   0,   0,  1,              // BEGIN, OPERAND, ERROR, END,
 110, 3,                       // LPAR, RPAR
 120, 120, 99, 99, 99,         // FUNC, SOLVE, POSTINC, POSTDEC, FACT
 99,  99,  99, 99, 99, 99,     // PREINC, PREDEC, PLUS, MINUS, NOT, COM,
 100,                          // POW,
 80,  80,  80, 80, 80,         // toPERCENT, MUL, DIV, MOD, PAR
 70,  70,                      // ADD, SUB,
 60,  60,  60,                 // ASL, ASR, LSR,
 50,  50,  50, 50,             // GT, GE, LT, LE,
 40,  40,                      // EQ, NE,
 38,                           // AND,
 36,                           // XOR,
 34,  130,                     // OR, MX_ELEM
 25,  25,  25, 25, 25, 25, 25, // SET, SETADD, SETSUB, SETMUL, SETDIV, SETMOD,
 25,  25,  25, 25, 25, 25,     // SETASL, SETASR, SETLSR, SETAND, SETXOR, SETOR,
 10,                           // SEMI
 15                            // COMMA
};

bool calculator::CheckOperand (int sp, uint32_t mask)
{
 t_value tag = v_stack[v_sp - sp].tag;
 int pos     = v_stack[v_sp - sp].pos;
 if (!((1 << tag) & mask)) // valid type for this operand
  {
   switch (tag)
    {
    case tvERR:
     error (pos, "Undefined operand");
     break;
    case tvCOMPLEX:
     error (pos, "Complex operand required");
     break;
    case tvSTR:
     error (pos, "String operand required");
     break;
    case tvMATRIX:
     error (pos, "Matrix operand required");
     break;
    default:
     error (pos, "Invalid operand type");
     break;
    }
   return false;
  }
 return true;
}

// Check that the number of arguments on the stack matches the expected count for a function, and
// that their types are valid according to the provided mask. Returns true if checks pass, false if
// there's an error (with error message set).
// mask is an array of bitmasks for each argument position, where each bit corresponds to a t_value
// tag type that is invalid for that argument. For example, if mask[0] has bit 1<<tvSTR set, then a
// string is not allowed for the first argument.
// First mask is for the last argument (top of stack), second mask for the previous argument down,
// etc.
bool calculator::CheckFnArgs (int n_args, int expected_args, const uint32_t mask[3])
{
 if (n_args != expected_args)
  {
   switch (expected_args)
    {
    case 1:
     error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
     break;
    case 2:
     error (v_stack[v_sp - n_args - 1].pos, "Function should take two arguments");
     break;
    case 3:
     error (v_stack[v_sp - n_args - 1].pos, "Function should take three arguments");
     break;
    }
   return false;
  }

 for (int i = 0; i < expected_args; i++)
  {
   t_value tag = v_stack[v_sp - 1 - i].tag;
   int pos     = v_stack[v_sp - 1 - i].pos;
   if ((1 << tag) & mask[i]) // invalid type for this argument
    {
     switch (tag)
      {
      case tvERR:
       error (pos, "Undefined operand");
       break;
      case tvCOMPLEX:
       error (pos, "Illegal complex operand");
       break;
      case tvSTR:
       error (pos, "Illegal string operand");
       break;
      case tvMATRIX:
       error (pos, "Illegal matrix operand");
       break;
      default:
       error (pos, "Invalid argument type");
       break;
      }
     return false;
    }
  }
 return true;
}

// Check that the number of arguments on the stack matches the expected count for an operator, and
// that their types are valid according to the provided mask. Returns true if checks pass, false if
// there's an error (with error message set).
bool calculator::CheckOpArgs (int n_args, const uint32_t mask[2])
{
 for (int i = 0; i < n_args; i++)
  {
   t_value tag = v_stack[v_sp - 1 - i].tag;
   int pos     = v_stack[v_sp - 1 - i].pos;
   if ((1 << tag) & mask[i]) // invalid type for this operand
    {
     switch (tag)
      {
      case tvERR:
       error (pos, "Undefined operand");
       break;
      case tvCOMPLEX:
       error (pos, "Illegal complex operand");
       break;
      case tvSTR:
       error (pos, "Illegal string operand");
       break;
      case tvMATRIX:
       error (pos, "Illegal matrix operand");
       break;
      default:
       error (pos, "Invalid operand type");
       break;
      }
     return false;
    }
  }
 return true;
}


// Check if the top of the value stack is a matrix with valid dimensions (used for indexing like
// A[i,j])
bool calculator::isMxIdx1 ()
{
 if ((v_stack[v_sp - 1].tag == tvFLOAT && // A[i,j]
      v_stack[v_sp - 1].mval && v_stack[v_sp - 1].mcols + v_stack[v_sp - 1].mrows))
  return true;
 else
  return false;
}

// Check if the top two entries on the value stack are matrices with valid dimensions (used for
// indexing like A[i,j])
bool calculator::isMxIdx2 ()
{
 if ((v_stack[v_sp - 1].tag == tvFLOAT && // A[i,j]
      v_stack[v_sp - 1].mval && v_stack[v_sp - 1].mcols + v_stack[v_sp - 1].mrows)
     || (v_stack[v_sp - 2].tag == tvFLOAT && // B[i,j]
         v_stack[v_sp - 2].mval && v_stack[v_sp - 2].mcols + v_stack[v_sp - 2].mrows))
  return true;
 else
  return false;
}

// Perform assignment operation for the top value on the stack, checking for variable and constant
// rules. Used for operators like '++', '--', '+=', '-=', etc. that assign to a variable.
bool calculator::set_op ()
{
 value &v = v_stack[v_sp - 1];
 if (v.tag == tvFLOAT && v.mval && v.mcols + v.mrows)
  {
   int row                     = v.irows;
   int col                     = v.icols;
   v.mval[row * v.mcols + col] = v.fval;
   return true;
  }
 else if (v.var == nullptr)
  {
   error (v.pos, "variable expected");
   return false;
  }
 else
  {
   if (v.var->tag == tsCONSTANT)
    {
     error (v.pos, "assignment to constant");
     return false;
    }
   v.var->val = v;
   return true;
  }
}

// Clear the value stack by resetting all entries to default values and
// setting the stack pointer to 0
void calculator::clear_v_stack ()
{
 for (int i = 0; i < max_stack_size; ++i)
  {
   v_stack[i].tag   = tvINT;
   v_stack[i].sval  = nullptr;
   v_stack[i].var   = nullptr;
   v_stack[i].pos   = 0;
   v_stack[i].ival  = 0;
   v_stack[i].fval  = (float__t)0.0L;
   v_stack[i].imval = (float__t)0.0L;
   v_stack[i].mval  = nullptr;
   v_stack[i].mrows = 0;
   v_stack[i].mcols = 0;
  }
 v_sp = 0;
}

// Evaluate the given expression and return the result as a floating-point value. The expression is
// parsed and evaluated according to the rules defined in the calculator class,
// using operator precedence
float__t calculator::evaluate_f (char *expression, __int64 *piVal, float__t *pimval)
{
 char var_name[MAXOP]; // maximum length of operator or function name
 bool operand = true;
 bool percent = false;
 int n_args = 0;
 t_operator saved_oper = toBEGIN;
 value saved_val;
 bool has_saved_val = false;
 expr = (expression && expression[0]); 
 buf  = expression; // Set the input buffer to the provided expression
 v_sp = 0;// Clear the value stack pointer
 o_sp = 0; // Clear the operator stack pointer
 pos = 0; // Reset the input buffer position
 t_mresult mr = mrERROR; // Initialize matrix result to error
 err[0] = '\0'; // Clear the error buffer
 errtype = teMath;
 mxerr[0] = '\0';    // Clear the error buffer
 result_fval  = qnan;    // Clear the floating-point result
 result_imval = 0.0; // Clear the imaginary result
 result_ival = 0;   // Clear the integer result
 //blockflag = false; // Clear the block flag

 if (deep > MAXSTK)
  {
   errorf (pos, "Too deep (%d) recursion.", deep);
   return qnan;
  }

 //double v1=0, v2=0, v3=0;
 //int n = strscan ("2026-04-22 10:00:05 102.5 0.985", "* * * * * * 1 0", 2, &v1,&v2,&v3);
 //n = strscan ("20`C, 125.4k", "01", 2, &v1, &v2, &v3);
 //test_bmp ();

 //init_mem_list ();
 clear_v_stack (); // Clear the value stack before evaluation
 res_cols = 0;       // Clear the result columns count
 res_rows = 0;       // Clear the result rows count
 if (res_mval) free (res_mval); // Free any previously allocated matrix result
 res_mval = nullptr; // Clear the matrix result pointer

 if (!expr) return qnan;

 o_stack[o_sp++] = toBEGIN;

 memset (sres, 0, STRBUF); // Clear the string result buffer
 while (true)              // Loop to process each token in the expression
  {
 next_token:
   t_operator oper;
   int op_pos = pos;
   
   if (has_saved_val)
    {
     v_stack[v_sp++] = saved_val;
     has_saved_val   = false;
    }
   if (saved_oper != toBEGIN)
    {
     oper = saved_oper;
     saved_oper = toBEGIN;
    }
   else
    {
     do
      {
       if (o_sp > 1 && v_sp &&
           o_stack[o_sp-1] == toLPAR && 
           o_stack[o_sp-2] == toSOLVE) 
        oper = sscan (v_stack[v_sp - 1].var);
       else
       oper = scan (operand, percent);
      }
     while (oper == toCONTINUE); // Skip semicolons
    }
   if (oper == toERROR)
    {
     return result_fval = qnan;
    }
  loper:
   switch (oper)
    {
    case toMUL:
    case toDIV:
    case toMOD:
    case toPOW:
    case toPAR:
    case toADD:
    case toSUB:
    case toCOMMA:
     percent = true;
     break;
    default:
     percent = false;
    }
   if (!operand)
    {
     if (!BINARY (oper) && oper != toEND && 
         oper != toPOSTINC && oper != toPOSTDEC && 
         oper != toRPAR  && oper != toFACT)
      {
       if (scfg & IMUL)
        {
         // Implicit multiplication: cases like 2sin(x), 3(4+5), (1+2)(3+4)
         // Allow only if next token is: FUNC, LPAR, or OPERAND (not after scientific suffix)
         if (oper == toSOLVE || oper == toFUNC || oper == toLPAR || oper == toOPERAND)
          {
           saved_oper = oper;
           if (oper != toLPAR && v_sp > 0)
            {
             saved_val     = v_stack[--v_sp];
             has_saved_val = true;
            }
           oper = toMUL;
           goto loper;
          }
         else
          {
           error (op_pos, "operator expected");
           return result_fval = qnan;
          }
        }
       else
        {
         error (op_pos, "operator expected");
         return result_fval = qnan;
        }
      }
     if (oper != toPOSTINC && 
         oper != toPOSTDEC && 
         oper != toRPAR && 
         oper != toFACT &&
         oper != toMX_ELEM)
      {
       operand = true;
      }
    }
   else
    {
     if ((oper == toOPERAND))
      {
       operand = false;
       n_args += 1;
       continue;
      }
     if ((oper != toSET) && (BINARY (oper) || oper == toRPAR))
      {
       error (op_pos, "operand expected");
       return result_fval = qnan;
      }
    }
   n_args = 1;
   while (o_sp && (lpr[o_stack[o_sp - 1]] >= rpr[oper])) 
    {
     t_operator cop = o_stack[--o_sp];
     if ((UNARY (cop) && (v_sp < 1)) || (BINARY (cop) && (v_sp < 2)))
      {
       error ("Unexpected end of expression");
       return result_fval = qnan;
      }

     if (BINARY(cop))
     {
       v_stack[v_sp - 2].info |= v_stack[v_sp - 1].info;
     }

     switch (cop)
      {
      case toBEGIN:
       if (oper == toRPAR)
        {
         error ("Unmatched ')'");
         return result_fval = qnan;
        }
       if (oper != toEND) error ("Unexpected end of input");
       if (v_sp == 1) // Final result should be on the stack
        {
         blockflag = false; 
         if (scfg & UTMP)
          {
           sprintf (var_name, "@%d", ++tmp_var_count);
           add (tsVARIABLE, var_name)->val = v_stack[0];
          }
         register_mem (v_stack[0].sval);
         register_mem (v_stack[0].mval);

         result_fval = v_stack[0].get ();
         result_imval = v_stack[0].imval;
         result_ival  = v_stack[0].ival;
         result_tag   = v_stack[0].tag;
         result_info  = v_stack[0].info;
         if (piVal) *piVal = v_stack[0].ival;
         if (pimval) *pimval = v_stack[0].imval;

         if (v_stack[0].tag == tvMATRIX)
          {
           res_cols = v_stack[0].mcols;
           res_rows = v_stack[0].mrows;
           res_mval = (float__t *)malloc (res_cols * res_rows * sizeof (float__t));
           if (!res_mval)
            {
             error ("Memory allocation failed for matrix result");
             return result_fval = qnan;
            }
           memcpy (res_mval, v_stack[0].mval, res_cols * res_rows * sizeof (float__t)); 
           v_stack[0].mval = nullptr; // Prevent freeing the matrix result in clear_v_stack
           result_fval     = 0;
          }

         if ((v_stack[0].tag == tvINT) && (v_stack[0].imval == 0.0))
          {
           result_ival = v_stack[0].ival;
           if (piVal) *piVal = v_stack[0].ival;
           if (pimval) *pimval = (float__t)0.0L;
           if (v_stack[0].sval)
            {
             strncpy (sres, v_stack[0].sval, STRBUF - 1); // Ensuring no buffer overflow
             sres[STRBUF - 1] = '\0'; // Ensure null-termination
             v_stack[0].sval  = nullptr;
            }
           return v_stack[0].ival;
          }
         else
          {
           result_ival = (__int64)v_stack[0].fval;
           if (piVal) *piVal = (__int64)v_stack[0].fval;
           if (v_stack[0].sval)
            {
             strncpy (sres, v_stack[0].sval, STRBUF - 1); // Ensuring no buffer overflow
             sres[STRBUF - 1] = '\0';                     // Ensure null-termination
             v_stack[0].sval  = nullptr;
            }
           else sres[0] = '\0'; // Clear sres if not a string result

           v_stack[v_sp - 1].var = nullptr;
           v_stack[0].imval = (float__t)0.0L;
           v_stack[0].fval  = (float__t)0.0L;
           v_stack[0].ival  = 0;
           v_stack[0].tag   = tvERR;

           save_vars_mem ();  // Save variables to prevent them from being freed during clear_v_stack
           clear_mem_list (); // Clear the memory list to free any temporary variables created
                              // during evaluation
           return result_fval;
          }
        }
       else if (v_sp != 0)
       {
        error ("Unexpected end of expression");
        return result_fval = qnan;
       }
       else
       {
         //error ("Empty expression");
        expr        = false;
        result_fval = 0;
        return 0;
       }

      case toCOMMA: // ,
       n_args += 1; // Just separate arguments, no calculation, return value of the last one
       continue;

      case toSEMI: // ;
       // For sub-expressions separated by ';', return the value of the last one
       if (((v_stack[v_sp - 1].tag == tvERR) || (v_stack[v_sp - 2].tag == tvERR)))
        {
         error (v_stack[v_sp - 2].pos, "Undefined operand");
         return result_fval = qnan;
        }
       register_mem (v_stack[v_sp - 1].sval);
       register_mem (v_stack[v_sp - 1].mval);
       v_stack[v_sp - 2] = v_stack[v_sp - 1];
       v_stack[v_sp - 1].var = nullptr;
       v_stack[v_sp - 1].sval = nullptr;
       v_sp -= 1;
       break;
       
      case toCONTINUE: //{}
       continue;

      case toADD:    // +
      case toSETADD: // +=
       {
        const uint32_t masks[] = { MSK_ERR, MSK_ERR};
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
         {
          v_stack[v_sp - 2].ival += v_stack[v_sp - 1].ival;
         }
        else if ((v_stack[v_sp - 1].tag == tvSTR) && (v_stack[v_sp - 2].tag == tvSTR))
         {
          int_t new_len = strlen (v_stack[v_sp - 2].sval) + strlen (v_stack[v_sp - 1].sval) + 1;
          if (new_len > STRBUF - 1)
           {
            error (v_stack[v_sp - 2].pos, "Resulting string is too long");
            return result_fval = qnan;
           }
          {
           char *new_s = (char *)sf_alloc (new_len);
           if (!new_s)
            {
             error (v_stack[v_sp - 2].pos, "Memory allocation failed");
             return result_fval = qnan;
            }
           strcpy (new_s, v_stack[v_sp - 2].sval);
           strcat (new_s, v_stack[v_sp - 1].sval);
           v_stack[v_sp - 2].sval = new_s;
          }
         }
        else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
         {
          error (v_stack[v_sp - 2].pos, "Illegal string operation");
          return result_fval = qnan;
         }
        else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
         {
          v_stack[v_sp - 2].ival += v_stack[v_sp - 1].ival;
          v_stack[v_sp - 2].fval += v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].imval += v_stack[v_sp - 1].imval;
          v_stack[v_sp - 2].tag = tvCOMPLEX;
         }
        else
         {
          if (v_stack[v_sp - 1].tag == tvPERCENT)
           {
            float__t left = v_stack[v_sp - 2].get ();
            float__t right = v_stack[v_sp - 1].get ();
            v_stack[v_sp - 2].fval = left + (left * right / ((float__t)100.0));
            v_stack[v_sp - 2].tag  = tvFLOAT;
           }
          else
           {
            v_stack[v_sp - 2].ival += v_stack[v_sp - 1].ival;
            v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () + v_stack[v_sp - 1].get ();
            v_stack[v_sp - 2].imval += v_stack[v_sp - 1].imval;
            v_stack[v_sp - 2].tag = tvFLOAT;
           }
          if (v_stack[v_sp - 2].imval != 0)
           v_stack[v_sp - 2].tag = tvCOMPLEX;
          else
           v_stack[v_sp - 2].tag = tvFLOAT;
         }
        v_sp -= 1;
        if (cop == toSETADD)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toSUB:    // -
      case toSETSUB: // -=
       {
        const uint32_t masks[] = { MSK_ERR, MSK_ERR };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
         {
          error (v_stack[v_sp - 2].pos, "Illegal string operation");
          return result_fval = qnan;
         }
        else if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
         {
          v_stack[v_sp - 2].ival -= v_stack[v_sp - 1].ival;
         }
        else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
         {
          v_stack[v_sp - 2].ival -= v_stack[v_sp - 1].ival;
          v_stack[v_sp - 2].fval -= v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].imval -= v_stack[v_sp - 1].imval;
          v_stack[v_sp - 2].tag = tvCOMPLEX;
         }
        else
         {
          if (v_stack[v_sp - 1].tag == tvPERCENT)
           {
            float__t left = v_stack[v_sp - 2].get ();
            float__t right = v_stack[v_sp - 1].get ();
            v_stack[v_sp - 2].fval = left - (left * right / ((float__t)100.0));
            v_stack[v_sp - 2].tag  = tvFLOAT;
           }
          else
           {
            v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () - v_stack[v_sp - 1].get ();
            v_stack[v_sp - 2].ival -= v_stack[v_sp - 1].ival;
            v_stack[v_sp - 2].imval -= v_stack[v_sp - 1].imval;
            v_stack[v_sp - 2].tag = tvFLOAT;
           }
          if (v_stack[v_sp - 2].imval != 0)
           v_stack[v_sp - 2].tag = tvCOMPLEX;
          else
           v_stack[v_sp - 2].tag = tvFLOAT;
         }
        v_sp -= 1;
        if (cop == toSETSUB)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

       case toMUL:    // *
       case toSETMUL: // *=
        {
         const uint32_t masks[] = { MSK_ERR|MSK_STR, MSK_ERR|MSK_STR };
         if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
         {
          v_stack[v_sp - 2].ival *= v_stack[v_sp - 1].ival;
          v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () * v_stack[v_sp - 1].get ();
         }
        else if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
                 || ((v_stack[v_sp - 1].imval != (float__t)0.0)
                     || (v_stack[v_sp - 2].imval != (float__t)0.0)))
         {
          // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i

          float__t a = v_stack[v_sp - 2].get ();
          float__t b = v_stack[v_sp - 2].imval; //  a + bi
          float__t c = v_stack[v_sp - 1].get ();
          float__t d = v_stack[v_sp - 1].imval; //  c + di

          v_stack[v_sp - 2].fval  = a * c - b * d;
          v_stack[v_sp - 2].imval = a * d + b * c;
          v_stack[v_sp - 2].tag   = tvCOMPLEX;
         }
        else if (v_stack[v_sp - 2].tag != tvCOMPLEX)
         {
          if (v_stack[v_sp - 1].tag == tvPERCENT)
           {
            float__t left = v_stack[v_sp - 2].get ();
            float__t right = v_stack[v_sp - 1].get ();
            //v_stack[v_sp - 2].fval = left * (left * right / ((float__t)100.0));
            v_stack[v_sp - 2].fval = left * (right / ((float__t)100.0));
            v_stack[v_sp - 2].tag  = tvFLOAT;
           }
          else
           {
            v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () * v_stack[v_sp - 1].get ();
           }
          v_stack[v_sp - 2].tag = tvFLOAT;
         }
        v_sp -= 1;
        if (cop == toSETMUL)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toDIV:    // /
      case toSETDIV: // /=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, MSK_ERR | MSK_STR };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
                 || ((v_stack[v_sp - 1].imval != 0.0) || (v_stack[v_sp - 2].imval != 0.0)))
         {
          // (a + bi) / (c + di) = [(ac + bd) + (bc - ad)i] / (c^2 + d^2)
          float__t a = v_stack[v_sp - 2].get ();
          float__t b = v_stack[v_sp - 2].imval;
          float__t c = v_stack[v_sp - 1].get ();
          float__t d = v_stack[v_sp - 1].imval;
          float__t denom = c * c + d * d;
          if (denom == (float__t)0.0L)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero");
            return result_fval = qnan;
           }
          v_stack[v_sp - 2].fval  = (a * c + b * d) / denom;
          v_stack[v_sp - 2].imval = (b * c - a * d) / denom;
          v_stack[v_sp - 2].tag   = tvCOMPLEX;
         }
        else if (v_stack[v_sp - 1].get () == (float__t)0.0L)
         {
          error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
          return result_fval = qnan;
         }
        if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival /= v_stack[v_sp - 1].ival;
          v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () / v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        else if (v_stack[v_sp - 2].tag != tvCOMPLEX)
         {
          if (v_stack[v_sp - 1].tag == tvPERCENT)
           {
            float__t left = v_stack[v_sp - 2].get ();
            float__t right = v_stack[v_sp - 1].get ();
            //v_stack[v_sp - 2].fval = left / (left * right / ((float__t)100.0));
            v_stack[v_sp - 2].fval = left / (right / ((float__t)100.0));
            v_stack[v_sp - 2].tag  = tvFLOAT;
           }
          else
           {
            v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () / v_stack[v_sp - 1].get ();
           }
          v_stack[v_sp - 2].tag = tvFLOAT;
         }
        v_sp -= 1;
        if (cop == toSETDIV)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPAR: // // parallel resistors
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, MSK_ERR | MSK_STR };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].get () == (float__t)0.0L)
                 || (v_stack[v_sp - 2].get () == (float__t)0.0L))
         {
          error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
          return result_fval = qnan;
         }
        if (v_stack[v_sp - 1].tag == tvPERCENT)
         {
          float__t left = v_stack[v_sp - 2].get ();
          float__t right = v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].fval = 1 / (1 / left + 1 / (left * right / ((float__t)100.0)));
          v_stack[v_sp - 2].tag  = tvFLOAT;
         }
        else if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
                 || ((v_stack[v_sp - 1].imval != (float__t)0.0)
                     || (v_stack[v_sp - 2].imval != (float__t)0.0)))
         {
          float__t ar = v_stack[v_sp - 2].get ();
          float__t ai = v_stack[v_sp - 2].imval;
          float__t br = v_stack[v_sp - 1].get ();
          float__t bi = v_stack[v_sp - 1].imval;

          // 1/a
          float__t a_norm2 = ar * ar + ai * ai;
          if (a_norm2 == (float__t)0.0L)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero");
            return result_fval = qnan;
           }
          float__t inv_a_r = ar / a_norm2;
          float__t inv_a_i = -ai / a_norm2;

          // 1/b
          float__t b_norm2 = br * br + bi * bi;
          if (b_norm2 == (float__t)0.0L)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
            return result_fval = qnan;
           }
          float__t inv_b_r = br / b_norm2;
          float__t inv_b_i = -bi / b_norm2;

          // sum = 1/a + 1/b
          float__t sum_r = inv_a_r + inv_b_r;
          float__t sum_i = inv_a_i + inv_b_i;

          // 1 / sum
          float__t sum_norm2 = sum_r * sum_r + sum_i * sum_i;
          if (sum_norm2 == (float__t)0.0L)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
            return result_fval = qnan;
           }
          v_stack[v_sp - 2].fval  = sum_r / sum_norm2;
          v_stack[v_sp - 2].imval = -sum_i / sum_norm2;
          v_stack[v_sp - 2].tag   = tvCOMPLEX;
         }
        else
         v_stack[v_sp - 2].fval = 1 / (1 / v_stack[v_sp - 1].get () + 1 / v_stack[v_sp - 2].get ());
        v_stack[v_sp - 2].tag = tvFLOAT;
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPERCENT: // %
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                   MSK_ERR | MSK_STR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].get () == 0.0) || (v_stack[v_sp - 2].get () == 0.0))
         {
          error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
          return result_fval = qnan;
         }
        if (v_stack[v_sp - 1].tag == tvPERCENT)
         {
          float__t left = v_stack[v_sp - 2].get ();
          float__t right = v_stack[v_sp - 1].get ();
          right = left * right / ((float__t)100.0);
          v_stack[v_sp - 2].fval = ((float__t)100.0) * (left - right) / right;
          v_stack[v_sp - 2].tag = tvFLOAT;
         }
        else
         {
          float__t left = v_stack[v_sp - 2].get ();
          float__t right = v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].fval = ((float__t)100.0) * (left - right) / right;
         }
        v_stack[v_sp - 2].tag = tvFLOAT;
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toMOD:    // %
      case toSETMOD: // %=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, 
                                   MSK_ERR | MSK_STR};
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
         {
          FmodC (v_stack[v_sp - 2].get (), v_stack[v_sp - 2].imval, v_stack[v_sp - 1].get (),
                 v_stack[v_sp - 1].imval, v_stack[v_sp - 2].fval, v_stack[v_sp - 2].imval);
          v_stack[v_sp - 2].tag = tvCOMPLEX;
         }
        else if (v_stack[v_sp - 1].get () == 0.0)
         {
          error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
          return result_fval = qnan;
         }
        if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival %= v_stack[v_sp - 1].ival;
         }
        else
         {
          if (v_stack[v_sp - 1].tag == tvPERCENT)
           {
            float__t left = v_stack[v_sp - 2].get ();
            float__t right = v_stack[v_sp - 1].get ();
            v_stack[v_sp - 2].fval = Fmod (left, left * right / ((float__t)100.0));
            v_stack[v_sp - 2].tag = tvFLOAT;
           }
          else
           v_stack[v_sp - 2].fval = Fmod (v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
          v_stack[v_sp - 2].tag = tvFLOAT;
         }
        v_sp -= 1;
        if (cop == toSETMOD)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPOW:    // ** ^
      case toSETPOW: // **= ^=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, MSK_ERR | MSK_STR };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival
              = (int_t)Pow ((float__t)v_stack[v_sp - 2].ival, (float__t)v_stack[v_sp - 1].ival);
         }
        else
         {
          if (v_stack[v_sp - 1].tag == tvPERCENT)
           {
            float__t left = v_stack[v_sp - 2].get ();
            float__t right = v_stack[v_sp - 1].get ();
            v_stack[v_sp - 2].fval = Pow (left, left * right / ((float__t)100.0));
            v_stack[v_sp - 2].tag = tvFLOAT;
           }
          else if (is_complex2 (&v_stack[v_sp - 2], &v_stack[v_sp - 1], vf_pow))
           {
            PowC (v_stack[v_sp - 2].get (), v_stack[v_sp - 2].imval, v_stack[v_sp - 1].get (),
                  v_stack[v_sp - 1].imval, v_stack[v_sp - 2].fval, v_stack[v_sp - 2].imval);
            v_stack[v_sp - 2].tag = tvCOMPLEX;
           }
          else
           {
            v_stack[v_sp - 2].fval = Pow (v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
            v_stack[v_sp - 2].tag  = tvFLOAT;
           }
         }
        v_sp -= 1;
        if (cop == toSETPOW)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toAND:    // &
      case toSETAND: // &=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                   MSK_ERR | MSK_STR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (isMxIdx2())
         {
          error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
          return result_fval = qnan;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival &= v_stack[v_sp - 1].ival;
         }
        else
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () & v_stack[v_sp - 1].get_int ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        v_sp -= 1;
        if (cop == toSETAND)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toOR:    // |
      case toSETOR: // |=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                   MSK_ERR | MSK_STR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (isMxIdx2())
         {
          error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
          return result_fval = qnan;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival |= v_stack[v_sp - 1].ival;
         }
        else
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () | v_stack[v_sp - 1].get_int ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        v_sp -= 1;
        if (cop == toSETOR)
         {
          if (!set_op ()) return result_fval = qnan;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

       case toMX_ELEM: // M[1,2]
       {
        const uint32_t masks[] = { MSK_ERR, MSK_ERR};
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;

        if (((v_stack[v_sp - 1].tag == tvERR) || (v_stack[v_sp - 2].tag == tvERR)))
         {
          error (v_stack[v_sp - 2].pos, "Undefined operand");
          return result_fval = qnan;
         }
        if (((v_stack[v_sp - 1].tag == tvMX_ELEM) && (v_stack[v_sp - 2].tag == tvMATRIX)))
         {
          v_stack[v_sp - 2].tag   = tvFLOAT;
          v_stack[v_sp - 2].icols = v_stack[v_sp - 1].icols;
          v_stack[v_sp - 2].irows = v_stack[v_sp - 1].irows;
          v_stack[v_sp - 2].fval  = v_stack[v_sp - 1].fval;
          v_stack[v_sp - 2].imval = v_stack[v_sp - 1].imval;
          v_stack[v_sp - 2].ival  = v_stack[v_sp - 1].ival;
         }
        v_sp -= 1;
       }
       break;

      case toXOR:    // ^
       case toSETXOR: // ^=
        {
         const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                    MSK_ERR | MSK_STR | MSK_COMPLEX };
         if (!CheckOpArgs (2, masks)) return result_fval = qnan;

         mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
         if (mr == mrERROR)
          {
           errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
           return result_fval = qnan;
          }
         else if (mr == mrDONE)
          {
           v_sp -= 1;
           v_stack[v_sp - 1].var = nullptr;
           break;
          }
         else if (isMxIdx2())
          {
           error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
           return result_fval = qnan;
          }
         else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
          {
           v_stack[v_sp - 2].ival ^= v_stack[v_sp - 1].ival;
          }
         else
          {
           v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () ^ v_stack[v_sp - 1].get_int ();
           v_stack[v_sp - 2].tag  = tvINT;
          }
         v_sp -= 1;
         if (cop == toSETXOR)
          {
           if (!set_op ()) return result_fval = qnan;
          }
         v_stack[v_sp - 1].var = nullptr;
        }
       break;

      case toASL:    // <<
       case toSETASL: // <<=
        {
         const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                    MSK_ERR | MSK_STR | MSK_COMPLEX };
         if (!CheckOpArgs (2, masks)) return result_fval = qnan;

         mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
         if (mr == mrERROR)
          {
           errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
           return result_fval = qnan;
          }
         else if (mr == mrDONE)
          {
           v_sp -= 1;
           v_stack[v_sp - 1].var = nullptr;
           break;
          }
         else if (isMxIdx2 ())
          {
           error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
           return result_fval = qnan;
          }
         else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
          {
           v_stack[v_sp - 2].ival <<= v_stack[v_sp - 1].ival;
          }
         else
          {
           v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () << v_stack[v_sp - 1].get_int ();
           v_stack[v_sp - 2].tag  = tvINT;
          }
         v_sp -= 1;
         if (cop == toSETASL)
          {
           if (!set_op ()) return result_fval = qnan;
          }
         v_stack[v_sp - 1].var = nullptr;
        }
       break;

      case toASR:    // >>
       case toSETASR: // >>=
        {
         const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                    MSK_ERR | MSK_STR | MSK_COMPLEX };
         if (!CheckOpArgs (2, masks)) return result_fval = qnan;

         mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
         if (mr == mrERROR)
          {
           errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
           return result_fval = qnan;
          }
         else if (mr == mrDONE)
          {
           v_sp -= 1;
           v_stack[v_sp - 1].var = nullptr;
           break;
          }
         else if (isMxIdx2 ())
          {
           error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
           return result_fval = qnan;
          }
         else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
          {
           v_stack[v_sp - 2].ival >>= v_stack[v_sp - 1].ival;
          }
         else
          {
           v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () >> v_stack[v_sp - 1].get_int ();
           v_stack[v_sp - 2].tag  = tvINT;
          }
         v_sp -= 1;
         if (cop == toSETASR)
          {
           if (!set_op ()) return result_fval = qnan;
          }
         v_stack[v_sp - 1].var = nullptr;
        }
       break;

      case toLSR:    // >>> (logical shift right)
       case toSETLSR: // >>>=
        {
         const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 
                                    MSK_ERR | MSK_STR | MSK_COMPLEX };
         if (!CheckOpArgs (2, masks)) return result_fval = qnan;

         mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
         if (mr == mrERROR)
          {
           errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
           return result_fval = qnan;
          }
         else if (mr == mrDONE)
          {
           v_sp -= 1;
           v_stack[v_sp - 1].var = nullptr;
           break;
          }
         else if (isMxIdx2 ())
          {
           error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
           return result_fval = qnan;
          }
         else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
          {
           v_stack[v_sp - 2].ival = (unsigned_t)v_stack[v_sp - 2].ival >> v_stack[v_sp - 1].ival;
          }
         else
          {
           v_stack[v_sp - 2].ival = (unsigned_t)v_stack[v_sp - 2].get_int () >> v_stack[v_sp - 1].get_int ();
           v_stack[v_sp - 2].tag = tvINT;
          }
         v_sp -= 1;
         if (cop == toSETLSR)
          {
           if (!set_op ()) return result_fval = qnan;
          }
         v_stack[v_sp - 1].var = nullptr;
        }
       break;

      case toEQ: // ==
        {
         const uint32_t masks[] = { MSK_ERR, MSK_ERR };
         if (!CheckOpArgs (2, masks)) return result_fval = qnan;
         mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
         if (mr == mrERROR)
          {
           errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
           return result_fval = qnan;
          }
         else if (mr == mrDONE)
          {
           v_sp -= 1;
           v_stack[v_sp - 1].var = nullptr;
           break;
          }
         else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
          {
           v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival == v_stack[v_sp - 1].ival;
          }
         else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
          {
           v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) == 0);
           v_stack[v_sp - 2].tag  = tvINT;
          }
         else
          {
           v_stack[v_sp - 2].ival = (v_stack[v_sp - 2].get () == v_stack[v_sp - 1].get ())
                                    && (v_stack[v_sp - 2].imval == v_stack[v_sp - 1].imval);
           v_stack[v_sp - 2].tag = tvINT;
          }
         v_sp -= 1;
         v_stack[v_sp - 1].var = nullptr;
        }
       break;

      case toNE: // !=, <>
       {
        const uint32_t masks[] = { MSK_ERR, MSK_ERR };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival != v_stack[v_sp - 1].ival;
         }
        else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
         {
          v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) != 0);
          v_stack[v_sp - 2].tag  = tvINT;
         }
        else
         {
          v_stack[v_sp - 2].ival = (v_stack[v_sp - 2].get () != v_stack[v_sp - 1].get ())
                                   || (v_stack[v_sp - 2].imval != v_stack[v_sp - 1].imval);
          v_stack[v_sp - 2].tag = tvINT;
         }
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toGT: // >
       {
        const uint32_t masks[] = { MSK_ERR | MSK_COMPLEX, MSK_ERR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival > v_stack[v_sp - 1].ival;
         }
        else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
         {
          v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) > 0);
          v_stack[v_sp - 2].tag  = tvINT;
         }
        else
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () > v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toGE: // >=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_COMPLEX, MSK_ERR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival >= v_stack[v_sp - 1].ival;
         }
        else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
         {
          v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) >= 0);
          v_stack[v_sp - 2].tag  = tvINT;
         }
        else
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () >= v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toLT: // <
       {
        const uint32_t masks[] = { MSK_ERR | MSK_COMPLEX, MSK_ERR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival < v_stack[v_sp - 1].ival;
         }
        else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
         {
          v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) < 0);
          v_stack[v_sp - 2].tag  = tvINT;
         }
        else
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () < v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toLE: // <=
       {
        const uint32_t masks[] = { MSK_ERR | MSK_COMPLEX, MSK_ERR | MSK_COMPLEX };
        if (!CheckOpArgs (2, masks)) return result_fval = qnan;
        mr = matrixbin (v_stack[v_sp - 2], v_stack[v_sp - 2], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 2].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_sp -= 1;
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival <= v_stack[v_sp - 1].ival;
         }
        else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
         {
          v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) <= 0);
          v_stack[v_sp - 2].tag  = tvINT;
         }
        else
         {
          v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () <= v_stack[v_sp - 1].get ();
          v_stack[v_sp - 2].tag  = tvINT;
         }
        v_sp -= 1;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPREINC: //++v
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Operation is %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (isMxIdx1())
         {
          int row = v_stack[v_sp - 1].irows;
          int col = v_stack[v_sp - 1].icols;
          v_stack[v_sp - 1].mval[row * v_stack[v_sp - 1].mcols + col] += (float__t)1.0L;
         }
        else if (v_stack[v_sp - 1].tag == tvINT)
         {
          v_stack[v_sp - 1].ival += 1;
         }
        else
         {
          v_stack[v_sp - 1].fval += 1;
         }
        if (!set_op ()) return result_fval = qnan;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPREDEC: // --v
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Operation is %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (isMxIdx1())
         {
          int row = v_stack[v_sp - 1].irows;
          int col = v_stack[v_sp - 1].icols;
          v_stack[v_sp - 1].mval[row * v_stack[v_sp - 1].mcols + col] -= (float__t)1.0L;
         }
        else if (v_stack[v_sp - 1].tag == tvINT)
         {
          v_stack[v_sp - 1].ival -= 1;
         }
        else
         {
          v_stack[v_sp - 1].fval -= ((float__t)1);
         }
        if (!set_op ()) return result_fval = qnan;
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPOSTINC: // v++
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Operation is %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (isMxIdx1())
         {
          int row = v_stack[v_sp - 1].irows;
          int col = v_stack[v_sp - 1].icols;
          v_stack[v_sp - 1].mval[row * v_stack[v_sp - 1].mcols + col] += (float__t)1.0L;
         }
        else if (v_stack[v_sp - 1].var == nullptr)
         {
          error (v_stack[v_sp - 1].pos, "Varaibale expected");
          return result_fval = qnan;
         }
        if (v_stack[v_sp - 1].var->val.tag == tvINT)
         {
          v_stack[v_sp - 1].var->val.ival += 1;
         }
        else
         {
          v_stack[v_sp - 1].var->val.fval += ((float__t)1);
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPOSTDEC: // v--
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Operation is %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (isMxIdx1())
         {
          int row = v_stack[v_sp - 1].irows;
          int col = v_stack[v_sp - 1].icols;
          v_stack[v_sp - 1].mval[row * v_stack[v_sp - 1].mcols + col] -= (float__t)1.0L;
         }
        else if (v_stack[v_sp - 1].var == nullptr)
         {
          error (v_stack[v_sp - 1].pos, "Varaibale expected");
          return result_fval = qnan;
         }
        if (v_stack[v_sp - 1].var->val.tag == tvINT)
         {
          v_stack[v_sp - 1].var->val.ival -= 1;
         }
        else
         {
          v_stack[v_sp - 1].var->val.fval -= ((float__t)1.0L);
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toFACT: // n!
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Operation is %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvCOMPLEX)
         {
          FactorialC (v_stack[v_sp - 1].get (), v_stack[v_sp - 1].imval, v_stack[v_sp - 1].fval,
                      v_stack[v_sp - 1].imval);
          v_stack[v_sp - 1].tag = tvCOMPLEX;
         }
        else if (isMxIdx1())
         {
          error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
          return result_fval = qnan;
         }
        else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 1].ival >= 0)
         {
          v_stack[v_sp - 1].ival = (int_t)Factorial ((float__t)v_stack[v_sp - 1].ival);
         }
        else
         {
          v_stack[v_sp - 1].fval = (float__t)Factorial ((float__t)v_stack[v_sp - 1].fval);
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toSET: // =, :=
       {
        if ((v_sp < 2) || (v_stack[v_sp - 2].var == nullptr))
         {
          if (v_sp < 2)
           error ("Variabale expected");
          else
           error (v_stack[v_sp - 2].pos, "Variabale expected");
          return result_fval = qnan;
         }
        else
         {
          if (v_stack[v_sp - 2].var->tag == tsCONSTANT)
           {
            error (v_stack[v_sp - 2].pos, "assignment to constant");
            return result_fval = qnan;
           }
          if ((v_stack[v_sp - 2].tag == tvMATRIX) && (v_stack[v_sp - 2].mval))
           register_mem (v_stack[v_sp - 2].mval);
          if ((v_stack[v_sp - 1].tag == tvMATRIX) && (v_stack[v_sp - 1].mval))
           register_mem (v_stack[v_sp - 1].mval);
          // if ((v_stack[v_sp - 1].tag == tvSTR) && (v_stack[v_sp - 1].sval))
          register_mem (v_stack[v_sp - 2].sval);
          register_mem (v_stack[v_sp - 1].sval);

          // v_stack[v_sp - 2] := v_stack[v_sp - 1]
          if (v_stack[v_sp - 2].tag == tvFLOAT && // A[i,j] := v
              v_stack[v_sp - 2].mval && v_stack[v_sp - 2].mcols + v_stack[v_sp - 2].mrows)
           {
            int row = v_stack[v_sp - 2].irows;
            int col = v_stack[v_sp - 2].icols;
            v_stack[v_sp - 2].mval[row * v_stack[v_sp - 2].mcols + col] = v_stack[v_sp - 1].get ();
           }
          else if ((v_stack[v_sp - 1].tag == tvSTR) && (v_stack[v_sp - 1].sval))
           {
            v_stack[v_sp - 2] = v_stack[v_sp - 2].var->val = v_stack[v_sp - 1];
            v_stack[v_sp - 2].tag = tvSTR;
           }
          else
           v_stack[v_sp - 2] = v_stack[v_sp - 2].var->val = v_stack[v_sp - 1];
         }
        v_sp -= 1;
       }
       break;

      case toNOT: // !
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if (v_stack[v_sp - 1].tag == tvINT)
         {
          v_stack[v_sp - 1].ival = (v_stack[v_sp - 1].ival == 0) ? 1 : 0;
         }
        else
         {
          v_stack[v_sp - 1].ival = (v_stack[v_sp - 1].fval == ((float__t)0.0)) ? 1 : 0;
          v_stack[v_sp - 1].tag  = tvINT;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toMINUS: // -v
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else
         {
          v_stack[v_sp - 1].ival  = -v_stack[v_sp - 1].ival;
          v_stack[v_sp - 1].fval  = -v_stack[v_sp - 1].fval;
          v_stack[v_sp - 1].imval = -v_stack[v_sp - 1].imval;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toPLUS: //+v
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else
         v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toCOM: // ~
       {
        const uint32_t masks[] = { MSK_ERR | MSK_STR, 0 };
        if (!CheckOpArgs (1, masks)) return result_fval = qnan;
        mr = matrixuno (v_stack[v_sp - 1], v_stack[v_sp - 1], cop);
        if (mr == mrERROR)
         {
          errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
          return result_fval = qnan;
         }
        else if (mr == mrDONE)
         {
          v_stack[v_sp - 1].var = nullptr;
          break;
         }
        else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 1].imval != 0.0))
         {
          v_stack[v_sp - 1].imval = -v_stack[v_sp - 1].imval;
          v_stack[v_sp - 1].tag   = tvCOMPLEX;
         }
        else if (v_stack[v_sp - 1].tag == tvINT)
         {
          v_stack[v_sp - 1].ival = ~v_stack[v_sp - 1].ival;
         }
        else
         {
          v_stack[v_sp - 1].ival = ~(uint64_t)v_stack[v_sp - 1].fval;
          v_stack[v_sp - 1].tag  = tvINT;
         }
        v_stack[v_sp - 1].var = nullptr;
       }
       break;

      case toSOLVE: // solve function without '('
       error ("'(' expected");
       return result_fval = qnan;
       break;

      case toRPAR: // )
       error ("mismatched ')'");
       return result_fval = qnan;

      case toFUNC: // function without '('
       error ("'(' expected");
       return result_fval = qnan;

      case toLPAR: // (
       if (oper != toRPAR)
        {
         error ("')' expected");
         return result_fval = qnan;
        }

       if (o_stack[o_sp - 1] == toSOLVE)
        {
         symbol *sym = v_stack[v_sp - n_args - 1].var;
         if (sym)
          {
           switch (sym->tag)
            {
            case tsSOLVE: // float f(str equation)
            case tsCALC: 
             {
              if (n_args != 1)
               {
                error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
                return result_fval = qnan;
               }
              if (v_stack[v_sp - 1].tag == tvSOLVE)
               {
                const char *equation = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                Solve (equation, sym->tag, v_stack[v_sp - 2].fval, v_stack[v_sp - 2].imval);
                if (v_stack[v_sp - 2].imval == (float__t)0.0L) v_stack[v_sp - 2].tag = tvFLOAT;
                else v_stack[v_sp - 2].tag = tvCOMPLEX;
                v_sp -= 1;
               }
              else
               {
                error (v_stack[v_sp - 1].pos, "Expression expected");
                return result_fval = qnan;
               }
             }
             break;
            case tsINTEGR: // float f(str equation)
            case tsSUM:
             {
              if (n_args != 1)
               {
                error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
                return result_fval = qnan;
               }
              if (v_stack[v_sp - 1].tag == tvINTEGR)
               {
                const char *equation = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                float__t result = Integr (equation, sym->tag);
                v_stack[v_sp - 2].fval  = result;
                v_stack[v_sp - 2].imval = 0.0;
                v_stack[v_sp - 2].ival  = (int_t)result;
                v_stack[v_sp - 2].tag   = tvFLOAT;
                v_sp -= 1;
               }
              else
               {
                error (v_stack[v_sp - 1].pos, "Expression expected");
                return result_fval = qnan;
               }
             }
             break;

            case tsFOR:
             {
              if (n_args != 1)
               {
                error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
                return result_fval = qnan;
               }
              if (v_stack[v_sp - 1].tag == tvFOR)
               {
                const char *equation = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                if (For (equation, v_stack[v_sp - 2])) v_sp -= 1;
                else
                 {
                  //error (v_stack[v_sp - 1].pos, "Error in 'for'");
                  return result_fval = qnan;
                 }
               }
             }
             break;

             case tsPLOT: //plot("fname", expr, from, to, var)
             {
              if (n_args != 1)
               {
                error (v_stack[v_sp - n_args - 1].pos, "Function should take one arguments");
                return result_fval = qnan;
               }
              if (v_stack[v_sp - 1].tag == tvPLOT)
               {
                const char *fname_expr = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                if (!Plot (fname_expr, sym->fidx)) return result_fval = qnan;
               }
              v_stack[v_sp - 2].ival = 1;
              v_stack[v_sp - 2].tag  = tvINT;
              v_sp -= 1;
             }
             break;

            case tsDIFF: // float f(str equation)
             {
              const uint32_t masks[] = { 0, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (v_stack[v_sp - 1].tag == tvDIFF)
               {
                const char *equation = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                float__t result = Diff (equation);
                v_stack[v_sp - 2].fval = result;
                v_stack[v_sp - 2].imval = 0.0;
                v_stack[v_sp - 2].ival = (int_t)result;
                v_stack[v_sp - 2].tag = tvFLOAT;
                v_sp -= 1;
               }
              else
               {
                error (v_stack[v_sp - 1].pos, "Expression expected");
                return result_fval = qnan;
               }
             }
             break;
            }
           o_sp -= 1;
           n_args = 1;
          }
        }

       if (o_stack[o_sp - 1] == toFUNC)
        {
         symbol *sym = v_stack[v_sp - n_args - 1].var;
         if (n_args > 0)
         {
          uint32_t info = 0;
          for (int i = 0; i < n_args; i++)
           {
            info |= v_stack[v_sp - n_args + i].info;
           }
          v_stack[v_sp - n_args - 1].info = info;
         }
         if (sym)
          {
           switch (sym->tag)
            {
            case tsFFUNCM: // float f(matrix x)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;

              if (v_stack[v_sp - 1].tag != tvMATRIX)
               {
                error (v_stack[v_sp - 1].pos, "Matrix operand required");
                return result_fval = qnan;
               }
              v_stack[v_sp - 2].fval
                  = (*(float__t (*) (void *, value &))sym->func) ((void *)this, v_stack[v_sp - 1]);

              if (isnan (v_stack[v_sp - 2].fval) || mxerr[0])
               {
                if (mxerr[0])
                 errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
                else
                 error (v_stack[v_sp - 1].pos, "Matrix result is not a number");
                return result_fval = qnan;
               }
              v_stack[v_sp - 2].tag = tvFLOAT;
              v_sp -= 1;
             }
             break;

            case tsMFUNCM2: // matrix f(matrix x, matrix y)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX,
                                         MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              if (!CheckOperand (2, MSK_MATRIX)) return result_fval = qnan;
              if (!CheckOperand (1, MSK_MATRIX)) return result_fval = qnan;

              //if (v_stack[v_sp - 1].tag != tvMATRIX || v_stack[v_sp - 2].tag != tvMATRIX)
              // {
              //  error (v_stack[v_sp - 2].pos, "Matrix operand required");
              //  return result_fval = qnan;
              // }

              bool res = ((bool (*) (void *, value &, value &, value &))sym->func) (
                  (void *)this, v_stack[v_sp - 3], v_stack[v_sp - 2], v_stack[v_sp - 1]);
              if (!res || mxerr[0])
               {
                if (mxerr[0])
                 errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
                else
                 error (v_stack[v_sp - 1].pos, "Matrix result is not a number");
                return result_fval = qnan;
               }
              v_sp -= 2;
             }
             break;

            case tsMFUNCI2: // matrix f(int r, int c)
             {
              bool res = false;
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 
                                         MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0 };
              if (n_args == 1 && v_stack[v_sp - 1].tag == tvMATRIX)
              {
                res = ((bool (*) (void *, value &, int, int))sym->func) (
                    (void *)this, v_stack[v_sp - 2], v_stack[v_sp - 1].mrows,
                    v_stack[v_sp - 1].mcols);
              }
              else
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              else
               {
                res = ((bool (*) (void *, value &, int, int))sym->func) (
                    (void *)this, v_stack[v_sp - 3], v_stack[v_sp - 2].ival,
                    v_stack[v_sp - 1].ival);
               }

              if (!res || mxerr[0])
               {
                if (mxerr[0])
                 errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
                else
                 error (v_stack[v_sp - 1].pos, "Matrix result is not a number");
                return result_fval = qnan;
               }

              v_sp -= n_args;
             }
             break;

            case tsVFUNC1: // float or complex f(x|z)
             {
              uint32_t masks[] = { MSK_ERR | MSK_STR , 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;

              if (v_stack[v_sp - 1].tag == tvMATRIX)
               {
                bool res = mxElemFn (sym->fidx, v_stack[v_sp - 2], v_stack[v_sp - 1]);
                if (!res || mxerr[0])
                 {
                  if (mxerr[0])
                   errorf (v_stack[v_sp - 1].pos, "Matrix %s", mxerr);
                  else
                   error (v_stack[v_sp - 1].pos, "Illegal matrix operation");
                  return result_fval = qnan;
                 }
                v_stack[v_sp - 2].tag = tvMATRIX;
                v_sp -= 1;
                break;
               }
              errtype = teMath;
              ((void (*) (value *, value *, int))sym->func) (&v_stack[v_sp - 2], &v_stack[v_sp - 1],
                                                             sym->fidx);
              v_sp -= 1;
             }
             break;

            case tsVFUNC2: // float or complex f(x|z,y|z)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX,
                                         MSK_ERR | MSK_STR | MSK_MATRIX, 0 };
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              errtype = teMath;
              ((void (*) (value *, value *, value *, int))sym->func) (
                  &v_stack[v_sp - 3], &v_stack[v_sp - 2], &v_stack[v_sp - 1], sym->fidx);
              v_sp -= 2;
             }
             break;

             case tsFFUNCI1: // float f(int x) (float() function)
              {
               const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
               if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
               errtype = teMath;
               v_stack[v_sp - 2].fval = (*(float__t (*) (int_t))sym->func) (v_stack[v_sp - 1].ival);
               v_stack[v_sp - 2].tag  = tvFLOAT;
               v_sp -= 1;
              }
              break;

            case tsIFUNCF1: // int f(float x) (wrgb() function)
              {
               const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
               if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
               v_stack[v_sp - 2].ival = (*(int_t (*) (float__t))sym->func) (v_stack[v_sp - 1].get ());
               v_stack[v_sp - 2].tag = tvINT;
               v_sp -= 1;
              }
             break;

            case tsSFUNCF1: // str f(float x) (winf())
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              const char *resStr = (*(const char *(*)(float__t))sym->func) (v_stack[v_sp - 1].get ());
              strncpy (sres, resStr ? resStr : "", STRBUF - 1);
              sres[STRBUF - 1] = '\0';
              if (sres[0]) fflags |= STR;
              v_stack[v_sp - 2].sval = dupString (sres);
              v_stack[v_sp - 2].fval = v_stack[v_sp - 1].get ();
              v_stack[v_sp - 2].ival = v_stack[v_sp - 1].ival;
              v_stack[v_sp - 2].tag  = tvSTR;
              v_sp -= 1;
             }
             break;

            case tsIFUNC1: // int f(int x) (int() function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              v_stack[v_sp - 2].ival = (*(int_t (*) (int_t))sym->func) (v_stack[v_sp - 1].get_int ());
              v_stack[v_sp - 2].tag = tvINT;
              v_sp -= 1;
             }
             break;

            case tsIFUNC2: // int f(int x, int y) (invmod() function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX,
                                         MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0 };
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;

              v_stack[v_sp - 3].ival = (*(int_t (*) (int_t, int_t))sym->func) (
                  v_stack[v_sp - 2].get_int (), v_stack[v_sp - 1].get_int ());
              v_stack[v_sp - 3].tag = tvINT;
              v_sp -= 2;
             }
             break;

            case tsFFUNC1: // float f(float x) (sing(x) function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              errtype = teMath;
              v_stack[v_sp - 2].fval = (*(float__t (*) (float__t))sym->func) (v_stack[v_sp - 1].get ());
              v_stack[v_sp - 2].tag = tvFLOAT;
              v_sp -= 1;
             }
             break;

            case tsFFUNC2: // float f(float x, float y) (min(), max(), ee() functions)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX,
                                         MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0 };
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              errtype = teMath;
              v_stack[v_sp - 3].fval = (*(float__t (*) (float__t, float__t))sym->func) (
                  v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
              v_stack[v_sp - 3].tag = tvFLOAT;
              v_sp -= 2;
             }
             break;

            case tsFFUNC3: // float f(float x, float y, float z) (vout() function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX,
                                         MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX,  
                                         MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX };
              if (!CheckFnArgs (n_args, 3, masks)) return result_fval = qnan;

              if (v_stack[v_sp - 1].tag == tvPERCENT) v_stack[v_sp - 1].fval /= ((float__t)100.0);
              errtype = teMath;
              v_stack[v_sp - 4].fval = (*(float__t (*) (float__t, float__t, float__t))sym->func) (
                  v_stack[v_sp - 3].get (), v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
              v_stack[v_sp - 4].tag = tvFLOAT;
              v_sp -= 3;
             }
             break;

            case tsPFUNCn: // f(str, ...) ( prn(...) function)
             if (n_args < 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one or more arguments");
               return result_fval = qnan;
              }

             (*(int_t (*) (char *, char *, int, char, value *))sym->func) // call prn(...)
                 (sres, // put result string in sres first
                  v_stack[v_sp - n_args].get_str (), n_args - 1,
                  c_imaginary, & v_stack[v_sp - n_args + 1]);

             v_stack[v_sp - n_args - 1].sval = dupString(sres);
             v_stack[v_sp - n_args - 1].ival = 0;
             v_stack[v_sp - n_args - 1].tag  = tvSTR;

             if (n_args > 1) // if there are arguments other than the first string argument, return
                             // the value of the first argument as well
              {
               v_stack[v_sp - n_args - 1].ival = v_stack[v_sp - n_args + 1].ival;
               v_stack[v_sp - n_args - 1].fval = v_stack[v_sp - n_args + 1].fval;
              }
             v_sp -= n_args;
             break;

            case tsFPFUNCn: // prnf("filename", "fmt", ...) function
             {
              if (n_args < 2)
               {
                error (v_stack[v_sp - n_args - 1].pos, "Function should take two or more arguments");
                return result_fval = qnan;
               }

              if (!CheckOperand (n_args, MSK_STR)) return result_fval = qnan; // filename (1st argument)
              if (!CheckOperand (n_args - 1, MSK_STR)) return result_fval = qnan; // format string (2nd argument)
              char fnamebuf[STRBUF];
              NormalizePath (v_stack[v_sp - n_args].get_str (), fnamebuf, STRBUF);

              int res = (*(int_t (*) (char *, char *, int, char, value *))sym->func) // call prnf(...)
                  (fnamebuf,                            // filename
                   v_stack[v_sp - n_args + 1].get_str (), n_args - 1, c_imaginary,
                   &v_stack[v_sp - n_args + 2]);

              v_stack[v_sp - n_args - 1].ival = res;
              v_stack[v_sp - n_args - 1].tag  = tvINT;

              v_sp -= n_args;
             }
             break;

            case tsSFUNCS1: // str f(str) (filedlg("*.*) function)
             {
              char filename[STRBUF];
              const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX | MSK_SCALAR, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (!CheckOperand (n_args, MSK_STR)) return result_fval = qnan; // file mask (1st argument should be string)
              char *filemask = v_stack[v_sp - n_args].get_str ();
              filename[0] = '\0';
              if (FileDlgFn)
               {
                if (filemask) strncpy (filename, filemask, STRBUF - 1);
                filename[STRBUF - 1] = '\0';
                if (!FileDlgFn (filename, STRBUF)) filename[0] = '\0';
               }
              v_stack[v_sp - n_args - 1].sval = dupString (filename);
              v_stack[v_sp - n_args - 1].fval = 0;
              v_stack[v_sp - n_args - 1].ival = 0;
              v_stack[v_sp - n_args - 1].tag  = tvSTR;
              v_sp -= n_args;  
             }
            break;

            case tsCIFUNC1: // int f(this, int x) (method of calculator class with int argument,
                            // prec() function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;

              v_stack[v_sp - 2].ival = (*(int_t (*) (void *, int_t))sym->func) (
                  (void *)this, v_stack[v_sp - 1].get_int ());
              v_stack[v_sp - 2].tag = tvINT;
              v_sp -= 1;
             }
             break;

            case tsSFUNCF2: //  f(str, val) (const())
             {
              const uint32_t masks[] = { MSK_ERR, MSK_ERR, 0, 0 };
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
              errtype = teMath;  
              bool res = (*((bool (*) (void *, char *, value &))sym->func)) // call const("name", value)
                  ((void *)this, v_stack[v_sp - 2].get_str (), v_stack[v_sp - 1]);
              if (!res) return result_fval = qnan;

              v_stack[v_sp - 3] = v_stack[v_sp - 1];
              v_sp -= 2;
             }
             break;

            case tsSIFUNC1: // int f(char *str) (datatime() function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
              v_stack[v_sp - 2].ival = (*(int_t (*) (char *))sym->func) (v_stack[v_sp - 1].get_str ());
              v_stack[v_sp - 2].tag = tvINT;
              v_sp -= 1;
             }
             break;

            case tsIF: //  if(x, y, z) 
             {
              const uint32_t masks[] = { MSK_ERR, MSK_ERR, MSK_ERR};
              if (!CheckFnArgs (n_args, 3, masks)) return result_fval = qnan;

              if (v_stack[v_sp - 3].ival)
                v_stack[v_sp - 4] = v_stack[v_sp - 2];
              else
                v_stack[v_sp - 4] = v_stack[v_sp - 1];
              v_sp -= 3;
             }
             break;

            case tsSFUNCI1: // char* f(int n) (factorize)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              (*(void (*) (char *, bool, int_t))sym->func) (sres, (scfg & PAS), v_stack[v_sp - 1].get_int ());
              sres[STRBUF - 1] = '\0';
              fflags |= FCTR;
              if (sres[0]) fflags |= STR;
              v_stack[v_sp - 2].sval = dupString (sres);
              v_stack[v_sp - 2].fval = (float__t)v_stack[v_sp - 1].get_int ();
              v_stack[v_sp - 2].ival = v_stack[v_sp - 1].get_int ();
              v_stack[v_sp - 2].tag  = tvSTR;
              v_sp -= 1;
             }
             break;

            case tsFITFN: // matrix fitpoly("data",["msk"], int n)|fit*("data", ["msk"]) etc.
            {
              //bool mxRegrFn (const char *fname, const char *msk, int n, rtype rt, value &res)
              char *filename = nullptr;
              char *msk = nullptr;
              int n = 0;
              bool res = false;
              rtype rfn = (rtype)sym->fidx;
              if (rfn == rtPoly) // fitlin can take either 1, 2 or 3 arguments, other fit functions take
               {               // only 1 or 2 argument
                if (n_args == 3) // fitlin("data", "msk", n)
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, //n
                                             MSK_ERR | MSK_MATRIX | MSK_COMPLEX,           // msk
                                             MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0 };      // filename 
                  if (!CheckFnArgs (n_args, 3, masks)) return result_fval = qnan;
                  if (!CheckOperand (3, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 3].get_str ();
                  if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                  msk = v_stack[v_sp - 2].get_str ();
                  if (!CheckOperand (1, MSK_SCALAR)) return result_fval = qnan;
                  n = (int)v_stack[v_sp - 1].get_int ();
                 }
                else
                if (n_args == 2) // fitlin("data", n) 
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, // n
                                             MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };   // filename
                  if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
                  if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 2].get_str ();
                  if (!CheckOperand (1, MSK_SCALAR)) return result_fval = qnan;
                  n = (int)v_stack[v_sp - 1].get_int ();
                 }
                else
                 {
                  error (v_stack[v_sp - n_args - 1].pos, "Wrong arguments number");
                  return result_fval = qnan;
                 }
               }
              else // fit*("data", ["msk"])
               {
                if (n_args == 2) // fitlin("data", "msk") 
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX, // msk
                                             MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0 }; // filename
                  if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
                  if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 2].get_str ();
                  if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
                  msk = v_stack[v_sp - 1].get_str ();  
                 }
                else if (n_args == 1) // fitlin("data") 
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0, 0 }; // filename
                  if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
                  if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 1].get_str ();
                 }
                else
                 {
                  error (v_stack[v_sp - n_args - 1].pos, "Wrong arguments number");
                  return result_fval = qnan;
                 }
               }
              char fnamebuf[STRBUF];
              NormalizePath (filename, fnamebuf, STRBUF);
              res = mxRegrFn (fnamebuf, msk, n, rfn, v_stack[v_sp - n_args - 1]); 

              if (!res || mxerr[0])
               {
                if (mxerr[0])
                 errorf (v_stack[v_sp - n_args].pos, "Matrix %s", mxerr);
                else
                 error (v_stack[v_sp - n_args].pos, "Error in fit function");
                return result_fval = qnan;
               }
              v_sp -= n_args;
            }
            break;

            case tsCLCFN: // float clc*(matrix, float)
            {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_COMPLEX | MSK_MATRIX, // float
                                         MSK_ERR | MSK_STR | MSK_COMPLEX, 0 };          // matrix
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              if (!CheckOperand (2, MSK_MATRIX)) return result_fval = qnan;
              errtype = teMath;
              float__t res = mxCalcFn (v_stack[v_sp - 2], (rtype)sym->fidx, v_stack[v_sp - 1].get());
              if (isnan (res) || mxerr[0])
               {
                if (mxerr[0])
                 errorf (v_stack[v_sp - 1].pos, "Stat: %s", mxerr);
                else
                 error (v_stack[v_sp - 1].pos, "Error in clc* function");
                return result_fval = qnan;
               }
              v_stack[v_sp - 3].fval = res;
              v_stack[v_sp - 3].tag = tvFLOAT;
              v_sp -= 2;
             }
            break;

            case tsSTFUN: // float mean("data") (stat functions)
             {
              char *filename = nullptr;
              char *msk      = nullptr;
              sfntype sfn    = (sfntype)sym->fidx;
              float__t x     = qnan;
              if (sfn < sfNormP) // sfNum..sfMax
               {
                if (n_args == 2) // mean("data", "msk")
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX,
                                             MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
                  if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
                  if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 2].get_str ();
                  if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
                  msk = v_stack[v_sp - 1].get_str ();
                 }
                else 
                if (n_args == 1) // mean("data")
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
                  if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
                  if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 1].get_str ();
                 }
                else
                 {
                  error (v_stack[v_sp - 1].pos, "Wrong arguments number");
                  return result_fval = qnan;
                 }
               }
              else // sfNormP..sfNormPD
               {
                if (n_args == 3) // normp("fname", "msk", x)
                 {
                  const uint32_t masks[]
                      = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX, MSK_ERR | MSK_MATRIX | MSK_COMPLEX,
                          MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0 };
                  if (!CheckFnArgs (n_args, 3, masks)) return result_fval = qnan;
                  if (!CheckOperand (3, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 3].get_str ();
                  if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                  msk = v_stack[v_sp - 2].get_str ();
                  if (!CheckOperand (1, MSK_SCALAR)) return result_fval = qnan;
                  x = v_stack[v_sp - 1].get ();
                 }
                else 
                if (n_args == 2) // normp("fname", x)
                 {
                  const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX,
                                             MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0 };
                  if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
                  if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                  filename = v_stack[v_sp - 2].get_str ();
                  if (!CheckOperand (1, MSK_SCALAR)) return result_fval = qnan;
                  x = v_stack[v_sp - 1].get ();
                 }
                else
                 {
                  error (v_stack[v_sp - 1].pos, "Wrong arguments number");
                  return result_fval = qnan;
                 }
               }
              char fnamebuf[STRBUF];
              NormalizePath (filename, fnamebuf, STRBUF);
              errtype = teMath;
              float__t res = StatFn (fnamebuf, msk, sfn, x);
              if (isnan (res) || mxerr[0])
               {
                if (mxerr[0])
                 errorf (v_stack[v_sp - 1].pos, "Stat: %s", mxerr);
                else
                 error (v_stack[v_sp - 1].pos, "Error in statistical function");
                return result_fval = qnan;
               }
              v_stack[v_sp - n_args - 1].ival = res;
              v_stack[v_sp - n_args - 1].tag  = tvFLOAT;
              v_sp -= n_args;
            }
            break;

            case tsUFUNCT: // user defined function
              { 
                //   sym->name -> frq
                //   (char *)sym->func -> (L, C)1/(2 pi sqrt(L C))
                //1. When tsUFUNC is found, create a new instance of the calculator class.
		        //2. Sequentially parse the previously stored string in the name, selecting
		        //   the names separated by ',' and calling addvar(name, value) for the new 
                //   calculator, popping value from the stack. And checking that the number 
                //   of declared parameters matches the number of passed ones.
		        //3. Call the evaluate method on the new calculator with the remainder after 
                //   the string parameters equal to 1/(2 pi sqrt(L C))
		        //4. Push the result onto the stack.
		        //5. Delete the previously created calculator.

                calculator *child = new calculator (scfg, hash_table, (MASK_DEFAULT), deep);
                if (!child)
                 {
                  errorf (pos, "Out of memory");
                  return result_fval = qnan;
                 }

                const char *funcdef = (const char *)sym->func;
                const char *p = strchr (funcdef, '(');
                if (!p)
                 {
                  errorf (pos, "No '(' in function definition");
                  delete child;
                  return result_fval = qnan;
                 }
                p++; // after '('

                int param_count = 0;
                int arg_idx     = 0;
                char vbuf[MAXOP]; // buffer for variable name, max 15 characters + null terminator
                while (*p && *p != ')')
                 {
                  // Skip spaces
                  while (*p && isspace (*p)) p++;
                  // Read variable name
                  int vi = 0;
                  while (*p && (isalnum (*p & 0x7f) || *p == '_'))
                   {
                    if (vi < (int)sizeof (vbuf) - 1) vbuf[vi++] = *p;
                    p++;
                   }
                  vbuf[vi] = 0;
                  if (vi == 0)
                   {
                    errorf (v_stack[v_sp - n_args + arg_idx].pos, "Empty parameter name");
                    delete child;
                    return result_fval = qnan;
                   }
                  // Add variable
                  if (arg_idx >= n_args)
                   {
                    errorf (v_stack[v_sp - n_args + arg_idx].pos, 
                        "Too many parameters in function definition");
                    delete child;
                    return result_fval = qnan;
                   }
                  if (((v_stack[v_sp - n_args + arg_idx].tag == tvERR)))
                   {
                    errorf (v_stack[v_sp - n_args + arg_idx].pos, "Undefined operand");
                    delete child;
                    return result_fval = qnan;
                   }
                  child->addvar (vbuf, v_stack[v_sp - n_args + arg_idx]);
                  arg_idx++;

                  // Skip spaces
                  while (*p && isspace (*p)) p++;
                  if (*p == ',')
                   {
                    p++;
                    continue;
                   }
                  else if (*p == ')')
                   {
                    break;
                   }
                  else
                   {
                    errorf (v_stack[v_sp - n_args + arg_idx].pos, 
                            "Invalid character in parameter list");
                    delete child;
                    return result_fval = qnan;
                   }
                 }
                if (*p != ')')
                 {
                  errorf (v_stack[v_sp - n_args + arg_idx].pos, "No closing ')' in function definition");
                  delete child;
                  return result_fval = qnan;
                 }
                p++; // after ')'

                if (arg_idx != n_args)
                 {
                  errorf (v_stack[v_sp - n_args - 1].pos, "Function should take %d argument(s)", arg_idx);
                  delete child;
                  return result_fval = qnan;
                 }

                // Rest of the string — expression
                while (*p && isspace (*p)) p++;

                child->setEscFn (EscFn);
                //child->setFileDlgFn (FileDlgFn);

                float__t res = child->evaluate_f ((char *)p);

               if (isnan (res) || child->error ()[0])
                 {
                  errorf (v_stack[v_sp - n_args - 1].pos, "%s", child->error());
                  delete child;
                  return result_fval = qnan;
                 }
                errtype = child->errt(); 
                v_stack[v_sp - n_args - 1].tag = tvFLOAT;
                v_stack[v_sp - n_args - 1].fval  = child->get_re_res ();
                v_stack[v_sp - n_args - 1].imval = child->get_im_res ();
                v_stack[v_sp - n_args - 1].ival  = child->get_int_res ();
                v_stack[v_sp - n_args - 1].sval  = dupString (child->get_str_res ());

                if (v_stack[v_sp - n_args - 1].imval != 0.0)
                 v_stack[v_sp - n_args - 1].tag = tvCOMPLEX;
                else 
                if ((float__t)v_stack[v_sp - n_args - 1].ival == v_stack[v_sp - n_args - 1].fval)
                 v_stack[v_sp - n_args - 1].tag = tvINT;
               if (child->get_res_tag () == tvMATRIX)
                 {
                  mxresult_t mxr = child->get_mx_res ();
                  v_stack[v_sp - n_args - 1].tag = tvMATRIX;
                  v_stack[v_sp - n_args - 1].mcols = mxr.cols;
                  v_stack[v_sp - n_args - 1].mrows = mxr.rows;
                  int msize = mxr.rows * mxr.cols * sizeof (float__t);
                  if (msize)
                   {
                    float__t *new_mval = (float__t *)sf_alloc (msize); 
                    if (new_mval)
                     {
                      memcpy (new_mval, mxr.mval, msize);
                      v_stack[v_sp - n_args - 1].mval = new_mval;
                     }
                    else
                     {
                      errorf (v_stack[v_sp - n_args - 1].pos, "Out of memory");
                      delete child;
                      return result_fval = qnan;
                     }
                   }
                 }
               else
                if (child->get_res_tag() == tvSTR)
                 {
                  v_stack[v_sp - n_args - 1].tag  = tvSTR;
                  fflags |= STR;
                 }

               fflags |= child->isfflags ();
               delete child;
               v_sp -= n_args; // pop arguments
             }
             break;
            default:
             error ("Invalid expression");
            }
          }
         o_sp -= 1;
         n_args = 1;
        }
       else if (n_args != 1)
        {
         error ("Function call expected");
         return result_fval = qnan;
        }
       goto next_token;
      default:
       error ("syntax error");
      }
    }
   if (o_sp == max_stack_size)
    {
     error ("operator stack overflow");
     return result_fval = qnan;
    }
   o_stack[o_sp++] = oper;
  }
}
#pragma endregion
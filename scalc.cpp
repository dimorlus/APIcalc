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
calculator::calculator (int_t cfg, symbol **symtab, int_t copyMask, int deep)
{
 this->deep  = deep + 1; // Set the current stack depth (incremented by 1 to account for the new instance)
 v_sp         = 0;    // Clear the value stack pointer
 o_sp         = 0;    // Clear the operator stack pointer

 blockflag     = false;   // Clear the block flag
 EscFn         = nullptr; // Clear the escape function pointer
 FileDlgFn     = nullptr; // Clear the file dialog function pointer
 ShowImageFn   = nullptr; // Clear the show image function pointer
 debugFn       = nullptr; // Clear the debug callback function pointer

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
//---------------------------------------------------------------------------

#pragma region Predefined Symbols
void calculator::AddPredefined (void)
{
 // This function can be used to add predefined constants and functions
 // to the symbol table. It is called from the constructor after initializing
 // the hash table and copying symbols from the provided symbol table (if any).

 add (tsIFUNC1, "console", (void *)console);
 
 add (tsSOLVE, "solve", nullptr);
 add (tsCALC, "calc", nullptr);
 add (tsINTEGR, "integr", nullptr);
 add (tsINTEGR, "integral", nullptr);
 add (tsSUM, "sum", nullptr);
 add (tsFOR, "for", nullptr);
 add (tsDIFF, "diff", nullptr);
 add (tsDIFF, "derivative", nullptr);
 add (tsEXTR, "extremum", nullptr);
 add (tsEXTR, "extr", nullptr);
 add (tsINVERSE, "inverse", nullptr);
 add (tsINVERSE, "inv", nullptr);

 add (tsCOLOR, "color", nullptr);

 add (tsLDSV, vrLoad, "load", nullptr);
 add (tsLDSV, vrSave, "save", nullptr);


 add (tsIF, "if", nullptr);
 add (tsRUN, scRun, "run", nullptr, true);
 add (tsRUN, scEval, "eval", nullptr, true);
 add (tsDATAF, dfDataf, "dataf", nullptr);
 add (tsDATAF, dfDatas, "datas", nullptr);
 add (tsERROR, "error", nullptr);

 // Cartesian plots
 add (tsPLOT, pl_plot, "plot", nullptr, false);

 // Polar plots
 add (tsPLOT, pl_plotpol, "plotpol", nullptr, false);

 // Parametric plots
 add (tsPLOT, pl_xyplot, "plotxy", nullptr, false);

 // Logarithmic plots
 add (tsPLOT, pl_plotlgx, "plotlgx", nullptr, false);

 add (tsPLOT, pl_plotlgy, "plotlgy", nullptr, false);

 add (tsPLOT, pl_plotlgxy, "plotlgxy", nullptr, false);

 // Smith chart
 add (tsPLOT, pl_plotsmith, "plotsmith", nullptr, false);

 add (tsPLOT, pl_plotsmithz, "plotsmithz", nullptr, false);

 // Data plots (points)
 add (tsPLOT, pl_plotdata, "plotdata", nullptr, false);

 // Data plots (lines)
 add (tsPLOT, pl_plotdatal, "plotdatal", nullptr, false);

 add (tsPLOTREG, fn_plotreg, "plotreg", nullptr, false);
 add (tsPLOTREG, fn_plotreg, "plotregion", nullptr, false);

 add (tsDIFF, "diff", nullptr);
 add (tsDIFF, "derivative", nullptr);

 add (tsFFUNCM, mx_Det, "det", nullptr);
 add (tsFFUNCM, mx_Trace, "trace", nullptr);
 add (tsFFUNCM, mx_Trace, "tr", nullptr);
 add (tsFFUNCM, mx_Norm, "norm", nullptr);
 add (tsFFUNCM, mx_Rows, "rows", nullptr);
 add (tsFFUNCM, mx_Cols, "cols", nullptr);
 add (tsFFUNCM, mx_Size, "size", nullptr);
 add (tsMFUNCM2, mx_Dot, "dot", nullptr);
 add (tsMFUNCM2, mx_Cross, "cross", nullptr);
 add (tsMFUNCI2, mx_Zeros, "zeros", nullptr);
 add (tsMFUNCI2, mx_Diag, "diag", nullptr);
 add (tsMFUNCI2, mx_Diag, "eye", nullptr);

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

 add (tsSFUNCF2, vrConst, "const", nullptr);
 add (tsSFUNCF2, vrVar,   "var", nullptr);

 add (tsCIFUNC1, ccPrec,   "prec", nullptr);
 add (tsCIFUNC1, ccOpt,    "opt", nullptr);
 add (tsCIFUNC1, ccOptOn,  "opton", nullptr);
 add (tsCIFUNC1, ccOptOff, "optoff", nullptr);

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
 add (tsVFUNC1, vf_isnan, "isnan", (void *)vfunc);
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
 addivar ("plot_top", 0); // Default plot top position in pixels)
 addivar ("plot_left", 0); // Default plot left position in pixels)
 addivar ("plot_bgc", 0x00FFFFFF); // Default plot background color (white)
 addivar ("plot_fgc", 0x00000000); // Default plot foreground color (black)
 //addfvar ("plot_ymin", 0.0L);       // Default plot y-axis minimum value)
 //addfvar ("plot_ymax", 0.0L);      // Default plot y-axis maximum value)
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
 errtype = teSyntax;
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
 err[0] = '\0'; // Clear error message
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
 if (!cp) return 0;
 int num        = 0;
 while (*cp)
  {
   if (*cp >= '0' && *cp <= '9') num++;
   cp++;
  }
 return num;
}

int calculator::strscan (char *str, const char *msk, int n, double *v, ...)
{
 char *cp = str;
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
      double val;
      char *fpos;
      //const char *endp;
      val = scandatatime (cp, &fpos);
      if (cp == fpos)
       {
        val = strtod (cp, &fpos);
        scientific (fpos, val);
       }
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
bool calculator::CheckFnArgs (int n_args, int expected_args, const uint32_t mask[])
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
bool calculator::CheckOpArgs (int n_args, const uint32_t mask[])
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
 errtype = teMath;  // Reset error type to math error
 mxerr[0] = '\0';    // Clear the error buffer
 //blockflag = false; // Clear the block flag

 if (deep > MAXSTK)
  {
   errorf (pos, "Too deep (%d) recursion.", deep);
   return qnan;
  }

 result_fval  = qnan;           // Clear the floating-point result
 result_imval = 0.0;            // Clear the imaginary result
 result_ival  = 0;              // Clear the integer result
 result_tag   = tvERR;          // Reset the result tag to error
 res_cols     = 0;              // Clear the result columns count
 res_rows = 0;                  // Clear the result rows count
 if (res_mval) free (res_mval); // Free any previously allocated matrix result
 res_mval = nullptr;            // Clear the matrix result pointer

 PlotReset ();
 clear_v_stack (); // Clear the value stack before evaluation
 
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
         if (v_stack[0].tag == tvBMP)
          register_mem (v_stack[0].sval, ptBMP); 
         else
          register_mem (v_stack[0].sval, ptMALLOC);

         register_mem (v_stack[0].mval);

         result_fval = v_stack[0].get ();
         result_imval = v_stack[0].imval;
         result_ival  = v_stack[0].ival;
         result_tag   = v_stack[0].tag;
         result_info  = v_stack[0].info;
         if (piVal) *piVal = v_stack[0].ival;
         if (pimval) *pimval = v_stack[0].imval;

         if (v_stack[0].tag == tvBMP)
         {
           if (v_stack[0].sval && ShowImageFn) ShowImageFn ((void *)v_stack[0].sval);
           else 
           if (!v_stack[0].sval) error ("Invalid bitmap data");
           v_stack[0].sval = nullptr; // Prevent freeing the bitmap data in clear_v_stack
         }

         if (v_stack[0].tag == tvCOLOR)
          {
           symbol *sp = find ("showcolor");
           if (sp && sp->tag == tsIFUNC1)
           {
             (*(int_t (*) (int_t))sp->func) (v_stack[0].get_int ());  
           }
          }

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
       if (v_stack[v_sp - 1].tag == tvBMP) register_mem (v_stack[v_sp - 1].sval, ptBMP);
       else register_mem (v_stack[v_sp - 1].sval, ptMALLOC);
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
        else if ((v_stack[v_sp - 1].tag == tvBMP) && (v_stack[v_sp - 2].tag == tvBMP))
         {
          uint32_t fgc = (uint32_t)getivar ("plot_fgc");
          bmpdraw *bmp1 = (bmpdraw *)v_stack[v_sp - 2].sval; 
          bmpdraw *bmp2 = (bmpdraw *)v_stack[v_sp - 1].sval;
          AddBmp (bmp1, bmp2, fgc);
         }
        else if (ColorOp (v_stack[v_sp - 2], v_stack[v_sp - 1], cop))
         {
          v_stack[v_sp - 2].tag = tvCOLOR;
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
           char *new_s = (char *)sf_alloc (new_len, ptMALLOC);
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
        else if (ColorOp (v_stack[v_sp - 2], v_stack[v_sp - 1], cop))
         {
          v_stack[v_sp - 2].tag = tvCOLOR;
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
        else if (ColorOp (v_stack[v_sp - 2], v_stack[v_sp - 1], cop))
         {
          v_stack[v_sp - 2].tag = tvCOLOR;
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
        else if (ColorOp (v_stack[v_sp - 2], v_stack[v_sp - 1], cop))
         {
          v_stack[v_sp - 2].tag = tvCOLOR;
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
            if ((scfg & SNAN) == 0)
             {
              error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
              return result_fval = qnan;
             }
           }
          v_stack[v_sp - 2].fval  = (a * c + b * d) / denom;
          v_stack[v_sp - 2].imval = (b * c - a * d) / denom;
          v_stack[v_sp - 2].tag   = tvCOMPLEX;
         }
        else if (v_stack[v_sp - 1].get () == (float__t)0.0L)
         {
          if ((scfg & SNAN) == 0)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
            return result_fval = qnan;
           }
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
          if ((scfg & SNAN) == 0)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
            return result_fval = qnan;
           }
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
            if ((scfg & SNAN) == 0)
             {
              error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
              return result_fval = qnan;
             }
           }
          float__t inv_a_r = ar / a_norm2;
          float__t inv_a_i = -ai / a_norm2;

          // 1/b
          float__t b_norm2 = br * br + bi * bi;
          if (b_norm2 == (float__t)0.0L)
           {
            if ((scfg & SNAN) == 0)
             {
              error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
              return result_fval = qnan;
             }
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
            if ((scfg & SNAN) == 0)
             {
              error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
              return result_fval = qnan;
             }
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
          if ((scfg & SNAN) == 0)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
            return result_fval = qnan;
           }
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
          if ((scfg & SNAN) == 0)
           {
            error (v_stack[v_sp - 2].pos, "Division by zero", teMath);
            return result_fval = qnan;
           }
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
        else if ((v_stack[v_sp - 1].tag == tvBMP) && (v_stack[v_sp - 2].tag == tvBMP))
         {
          uint32_t fgc  = (uint32_t)getivar ("plot_fgc");
          bmpdraw *bmp1 = (bmpdraw *)v_stack[v_sp - 2].sval;
          bmpdraw *bmp2 = (bmpdraw *)v_stack[v_sp - 1].sval;
          AddBmp (bmp1, bmp2, fgc);
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

          if (v_stack[v_sp - 2].tag == tvBMP)
           register_mem (v_stack[v_sp - 2].sval, ptBMP);
          else
           register_mem (v_stack[v_sp - 2].sval, ptMALLOC);

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
              if ((v_stack[v_sp - 1].tag == tvSOLVE) || (v_stack[v_sp - 1].tag == tvSOLVEA))
               {
                const char *equation = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                Solve (equation, v_stack[v_sp - 1].tag, sym->tag, v_stack[v_sp - 2].fval, v_stack[v_sp - 2].imval);
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
                if (!Plot (fname_expr, sym->fidx, v_stack[v_sp - 2])) return result_fval = qnan;
               }
              v_stack[v_sp - 2].ival = 1;
              v_stack[v_sp - 2].fval = (float__t)1.0L;
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

            case tsEXTR: // float f(str equation)
             {
              const uint32_t masks[] = { 0, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (v_stack[v_sp - 1].tag == tvEXTR)
               {
                const char *equation = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                float__t result = Extremum (equation);
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

            case tsINVERSE: // float f(str equation)
             {
              const uint32_t masks[] = { 0, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (v_stack[v_sp - 1].tag == tvINVERSE)
               {
                const char *equation    = v_stack[v_sp - 1].sval ? v_stack[v_sp - 1].sval : "";
                float__t result         = Inverse (equation);
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
              switch (sym->fidx)
               {
               case mx_Det:
                v_stack[v_sp - 2].fval = mxDet (v_stack[v_sp - 1]);
               break;
               case mx_Trace:
                v_stack[v_sp - 2].fval = mxTrace (v_stack[v_sp - 1]);
               break;
               case mx_Norm:
                v_stack[v_sp - 2].fval = mxNorm (v_stack[v_sp - 1]);
               break;
               case mx_Rows:
                v_stack[v_sp - 2].fval = (float__t)v_stack[v_sp - 1].mrows;
               break;
               case mx_Cols:
                v_stack[v_sp - 2].fval = (float__t)v_stack[v_sp - 1].mcols;
               break;
               case mx_Size:
                v_stack[v_sp - 2].fval = (float__t)(v_stack[v_sp - 1].mrows * v_stack[v_sp - 1].mcols);
               break;
               default:
                error (v_stack[v_sp - 1].pos, "Unknown matrix function");
                return result_fval = qnan;
               break;
               }

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
              bool res;
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              if (!CheckOperand (2, MSK_MATRIX)) return result_fval = qnan;
              if (!CheckOperand (1, MSK_MATRIX)) return result_fval = qnan;
              switch (sym->fidx)
               {
               case mx_Dot:  
                res = mxDot (v_stack[v_sp - 3], v_stack[v_sp - 2], v_stack[v_sp - 1]);
               break;
               case mx_Cross:
                res = mxCross (v_stack[v_sp - 3], v_stack[v_sp - 2], v_stack[v_sp - 1]);
               break;
               default:
                error (v_stack[v_sp - 1].pos, "Unknown matrix function");
                return result_fval = qnan;
               break;
               }

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
              int rows = 0, cols = 0;
              value *resval = nullptr;
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 
                                         MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0 };
              if (n_args == 1 && v_stack[v_sp - 1].tag == tvMATRIX)
              {
               rows = v_stack[v_sp - 1].mrows;
               cols = v_stack[v_sp - 1].mcols;
               resval = &v_stack[v_sp - 2];
              }
              else 
              if (n_args == 1 && ((v_stack[v_sp - 1].tag == tvINT)
                       || (v_stack[v_sp - 1].tag == tvFLOAT)))
              {
                const uint8_t rng[] = 
                {
                 //   0     1     2     3     4     5     6     7     8     9
                   0x00, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x24, 0x33,
                 //  10    11    12    13    14    15    16    17    18    19
                   0x25, 0x34, 0x34, 0x27, 0x27, 0x35, 0x44, 0x36, 0x36, 0x45,
                 //  20    21    22    23    24    25    26    27    28    29
                   0x45, 0x37, 0x46, 0x46, 0x46, 0x55, 0x47, 0x54, 0x47, 0x48,
                 //  30    31    32    33    34    35    36    37    38    39
                   0x56, 0x48, 0x56, 0x67, 0x67, 0x57, 0x66, 0x67, 0x67, 0x68,
                 //  40    41    42    43    44    45    46    47    48    49
                   0x58, 0x68, 0x67, 0x68, 0x68, 0x59, 0x68, 0x77, 0x68, 0x77
                };
                int idx = v_stack[v_sp - 1].get_int ();
                cols    = rng[idx] & 0x0f;
                rows    = rng[idx] / 16;
                resval = &v_stack[v_sp - 2];
              }
              else
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              else
               {
                rows   = v_stack[v_sp - 2].ival;
                cols   = v_stack[v_sp - 1].ival;
                resval = &v_stack[v_sp - 3];
               }
              switch (sym->fidx)
               {
               case mx_Diag:
                res = mxDiag (*resval, rows, cols);
                break;
               case mx_Zeros:
                res = mxZeros (*resval, rows, cols);
                break;
               default:
                error (v_stack[v_sp - 1].pos, "Unknown matrix function");
                return result_fval = qnan;
                break;
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
               v_stack[v_sp - 2].fval = (float__t)v_stack[v_sp - 2].ival;
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

            case tsCOLOR: // color(int x)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              v_stack[v_sp - 2].ival = v_stack[v_sp - 1].get_int ();
              v_stack[v_sp - 2].fval = (float__t)v_stack[v_sp - 2].ival;
              v_stack[v_sp - 2].imval = (float__t)0.0L;
              v_stack[v_sp - 2].tag = tvCOLOR;
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

            case tsPLOTREG: // plotreg(xmin, xmax, ymin, ymax)
             {
              const uint32_t masks[] = { (uint32_t)~MSK_SCALAR, (uint32_t)~MSK_SCALAR,
                                         (uint32_t)~MSK_SCALAR, (uint32_t)~MSK_SCALAR };
              if (!CheckFnArgs (n_args, 4, masks)) return result_fval = qnan;
              PlotRegion (v_stack[v_sp - 4].get (), v_stack[v_sp - 3].get (),
                          v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
              v_stack[v_sp - n_args - 1].ival = 0;
              v_stack[v_sp - n_args - 1].fval = 0;
              v_stack[v_sp - n_args - 1].tag = tvINT;
              v_sp -= n_args;
             }
            break;

            case tsPFUNCn: // f(str, ...) ( prn(...) function)
             {
              char strres[2048];
              strres[0] = '\0';
              if (n_args < 1)
               {
                error (v_stack[v_sp - n_args - 1].pos,
                       "Function should take one or more arguments");
                return result_fval = qnan;
               }

              (*(int_t (*) (char *, char *, int, char, value *))sym->func) // call prn(...)
                  (strres, // put result string in sres first
                   v_stack[v_sp - n_args].get_str (), n_args - 1, c_imaginary,
                   &v_stack[v_sp - n_args + 1]);

              strncpy (sres, strres, STRBUF - 1);
              sres[STRBUF - 1] = '\0';

              v_stack[v_sp - n_args - 1].sval = dupString (strres);
              v_stack[v_sp - n_args - 1].ival = 0;
              v_stack[v_sp - n_args - 1].tag  = tvSTR;

              if (n_args > 1) // if there are arguments other than the first string argument, return
                              // the value of the first argument as well
               {
                v_stack[v_sp - n_args - 1].ival = v_stack[v_sp - n_args + 1].ival;
                v_stack[v_sp - n_args - 1].fval = v_stack[v_sp - n_args + 1].fval;
               }
              v_sp -= n_args;
             }
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
              char fnamebuf[STRBUF] = { 0 };
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

            case tsRUN:
            {
              const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX | MSK_SCALAR, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (!CheckOperand (n_args, MSK_STR))
               return result_fval = qnan; // file mask (1st argument should be string)
              char *filename = v_stack[v_sp - n_args].get_str ();
              if (!Run (filename, sym->fidx, v_stack[v_sp - n_args - 1]))
               {
                if (!err[0]) error (v_stack[v_sp - 1].pos, "Error run script");
                return result_fval = qnan;
               }
              v_sp -= n_args; 
            }
            break;

            case tsCIFUNC1: // int f(this, int fn, int x) (method of calculator class with int argument,
                            // prec() function)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_STR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              int_t prec = v_stack[v_sp - 1].get_int ();
              switch (sym->fidx)
               {
               case ccOpt:
                {
                 // Handle option setting
                 int_t opt = issyntax ();
                 syntax (prec);
                 prec = opt;
                }
               break;
               case ccOptOn:
                {
                 int_t opt = issyntax ();
                 syntax (opt | prec);
                 prec = issyntax ();
                }
               break;
               case ccOptOff:
                {
                 int_t opt = issyntax ();
                 syntax (opt & ~prec);
                 prec = issyntax ();
                }
               break;
               case ccPrec:
                if (prec < 0) prec = 0;
                if (prec > MAX_PREC) prec = MAX_PREC;
                set_fprec (prec);
               break;
               default:
                error (v_stack[v_sp - 1].pos, "Unknown calculator method");
                return result_fval = qnan;
               }
              v_stack[v_sp - 2].ival = prec;
              v_stack[v_sp - 2].tag = tvINT;
              v_sp -= 1;
             }
             break;

            case tsSFUNCF2: //  f(str, val) (const())
             {
              const uint32_t masks[] = { MSK_ERR, MSK_ERR, 0, 0 };
              bool res;
              if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
              if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
              errtype = teMath;  
              switch (sym->fidx)
              {
               case vrConst:
                res = addconst (v_stack[v_sp - 2].get_str (), v_stack[v_sp - 1]);
               break;
               case vrVar:
                res = addvar (v_stack[v_sp - 2].get_str (), v_stack[v_sp - 1]);
                break;
               default:
                error (v_stack[v_sp - 1].pos, "Unknown function");
                return result_fval = qnan;
               break;
              }
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
              char fnamebuf[STRBUF] = { 0 };
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
              char fnamebuf[STRBUF] = { 0 };
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
              v_stack[v_sp - n_args - 1].ival = (int)res;
              v_stack[v_sp - n_args - 1].fval = res;
              v_stack[v_sp - n_args - 1].tag  = tvFLOAT;
              v_sp -= n_args;
            }
            break;

            case tsLDSV:
             {
               if (sym->fidx == vrLoad)
                {
                 const uint32_t masks[] = { (uint32_t)~MSK_STR, 0, 0 }; // filename
                 if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
                 if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
                 char *filename = v_stack[v_sp - 1].get_str ();
                 if (!Load (filename, v_stack[v_sp - n_args - 1]))
                  {
                   error (v_stack[v_sp - n_args - 1].pos, "Error loading variable");
                   return result_fval = qnan;
                  }
                }
               else 
               if (sym->fidx == vrSave)
                {
                 const uint32_t masks[] = { MSK_ERR,                    // variable to save
                                            (uint32_t)~MSK_STR, 0, 0 }; // filename
                 if (!CheckFnArgs (n_args, 2, masks)) return result_fval = qnan;
                 if (!CheckOperand (2, MSK_STR)) return result_fval = qnan;
                 char *filename = v_stack[v_sp - 2].get_str ();
                 if (!Save (filename, v_stack[v_sp - 1]))
                  {
                   error (v_stack[v_sp - 1].pos, "Error saving variable");
                   return result_fval = qnan;
                  }
                 v_stack[v_sp - n_args - 1] = v_stack[v_sp - 1];
                }
               v_sp -= n_args;
             }
            break;

            case tsDATAF: // dataf("filename", "mask", x1, x2, ...) function
             {
              int res = 0;
              if (n_args < 2)
               {
                error (v_stack[v_sp - n_args - 1].pos,
                       "Function should take two or more arguments");
                return result_fval = qnan;
               }

              if (!CheckOperand (n_args, MSK_STR))
               return result_fval = qnan; // filename (1st argument)
              if (!CheckOperand (n_args - 1, MSK_STR))
               return result_fval = qnan; // format string (2nd argument)
              if (sym->fidx == dfDataf)
               {
                char fnamebuf[STRBUF] = { 0 };
                NormalizePath (v_stack[v_sp - n_args].get_str (), fnamebuf, STRBUF);

                res = dataf (fnamebuf, v_stack[v_sp - n_args + 1].get_str (), n_args - 2,
                                 &v_stack[v_sp - n_args + 2]);
               }
              else 
              if (sym->fidx == dfDatas)
               {
                res = datas (v_stack[v_sp - n_args].get_str (), v_stack[v_sp - n_args + 1].get_str (), n_args - 2,
                             &v_stack[v_sp - n_args + 2]);
               }
              else
               {
                error (v_stack[v_sp - n_args - 1].pos, "Unknown function");
                return result_fval = qnan;
               }

              if (res < 0)
               {
                if (res == da_FileErr)
                 error (v_stack[v_sp - n_args - 1].pos, "Error in data file");
                else if (res == da_ArgNum)
                 error (v_stack[v_sp - n_args - 1].pos,
                        "Format does not match number of arguments");
                else if (res < da_ArgNum)
                 error (-(res - da_ArgNum), "Argument type mismatch");
                return result_fval = qnan;
               }

              v_stack[v_sp - n_args - 1].ival = res;
              v_stack[v_sp - n_args - 1].tag  = tvINT;

              v_sp -= n_args;
             }
             break;

            case tsERROR: // error(char *str)
             {
              const uint32_t masks[] = { MSK_ERR | MSK_MATRIX | MSK_COMPLEX, 0, 0 };
              if (!CheckFnArgs (n_args, 1, masks)) return result_fval = qnan;
              if (!CheckOperand (1, MSK_STR)) return result_fval = qnan;
              char *errStr = v_stack[v_sp - 1].get_str ();

              strncpy (err, errStr ? errStr : "", 80 - 1);
              err[80 - 1] = '\0';
              errpos  = 1;
              errtype = teSyntax;
              return result_fval = qnan;
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

                calculator *child = new calculator (scfg|SNAN, hash_table, (MASK_DEFAULT), deep);
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

               GetChildRes (child, v_stack[v_sp - n_args - 1]);
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
//---------------------------------------------------------------------------

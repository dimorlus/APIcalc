//---------------------------------------------------------------------------
//-< CCALC.H >-------------------------------------------------------*--------*
// Ccalc                      Version 1.01       (c) 1998  GARRET    *     ?  *
// (C expression command line calculator)                            *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Oct-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Oct-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
#ifndef scalcH
#define scalcH

#pragma region Includes and definitions
#ifndef _WIN32
#include <minwindef.h>
#endif

#include <cstdlib> // for free() function
#include <limits>
#include "bmp.h"
#include "files.h"

#pragma endregion

// RW - set both by calc engine and application
// WO - set only from application
// RO - set only by calc engine
// UI - set and used only from application
#define PAS    (1ULL << 0) // (RW) Pascal assingment and comparison style (:= =) <-> (= ==)
#define SCI    (1ULL << 1) // (WO) Scientific numbers format (2k == 2000)
#define UPCASE (1ULL << 2) // (WO) Case insensetive variables
#define UTMP   (1ULL << 3) // (WO) Using $n for temp
#define FFLOAT (1ULL << 4) // (WO) Forced float

#define DEG  (1ULL << 5)  // (RO) Degrees format found
#define CPX  (1ULL << 6)  // (RO) Complex format found
#define ENG  (1ULL << 7)  // (RO) Engineering (6k8) format found
#define STR  (1ULL << 8)  // (RO) String format found
#define HEX  (1ULL << 9)  // (RO) Hex format found
#define OCT  (1ULL << 10) // (RO) Octal format found
#define fBIN (1ULL << 11) // (RO) Binary format found
#define FBIN (1ULL << 11) // (RO) Binary format found
#define DAT  (1ULL << 12) // (RO) Date time format found
#define CHR  (1ULL << 13) // (RO) Char format found
#define WCH  (1ULL << 14) // (RO) WChar format found
#define ESC  (1ULL << 15) // (RO) Escape format found
#define CMP  (1ULL << 16) // (RO) Computing format found

#define NRM  (1ULL << 17) // (UI) Normalized output
#define IGR  (1ULL << 18) // (UI) Integer output
#define UNS  (1ULL << 19) // (UI) Unsigned output
#define ALL  (1ULL << 20) // (UI) All outputs
#define MIN  (1ULL << 21) // (UI) Esc minimized feature (GUI only)
#define OPT  (1ULL << 21) // (UI) Print options (CLI only)
#define MNU  (1ULL << 22) // (UI) Show/hide menu feature (GUI only)
#define SRC  (1ULL << 22) // (UI) Print source expression (CLI only)
#define UTM  (1ULL << 23) // (UI) Unix time
#define FRC  (1ULL << 24) // (UI) Fraction output
#define FRI  (1ULL << 25) // (UI) Fraction inch output
#define FRH  (1ULL << 26) // (UI) Farenheit input/output
#define TOP  (1ULL << 27) // (UI) Always on top (GUI only)
#define FLT  (1ULL << 27) // (UI) Floating point output (CLI only)
#define IMUL (1ULL << 28) // (WO) Implicit multiplication
#define AUTO (1ULL << 29) // (UI) Auto output format
#define FCTR (1ULL << 30) // (UI) Factorization output
#define SNAN (1ULL << 31) // (RO) Silent NaN
#define DBG  (1ULL << 32) // (WO) Debug mode (prints internal details) (CLI only)
#define NBLK (1ULL << 33) // (WO) No block operators (GUI only)

#define STRBUF 256 // bufer size for string operations
#define MAXOP  64  // maximum length of operator or function name
#define MAXSTK 10  // maximum stack depth
#define MAXNAME 16  // maximum length of variable or function name

#ifdef __BORLANDC__

#pragma warn -8027
#define nullptr NULL
#define _long_double_
typedef unsigned char uint8_t;
typedef char int8_t;
typedef __int64 int_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef unsigned __int64 unsigned_t;

typedef unsigned int uint32_t;
typedef int int32_t;
#else //__BORLANDC__

#include <cstdint>
#include <cstring> 

//#define __GNUC__

#ifdef __GNUC__
#define _float128_
#else
#define _long_double_
#endif
typedef int64_t int_t;
typedef uint64_t unsigned_t;
#endif 


#ifdef __BORLANDC__
#include <alloc.h>
#include <process.h>
#endif

#define _WIN_
#define _WCHAR_ // L'c' and 'c'W input format allow
#define _ENABLE_PREIMAGINARY_

#define _ENABLE_BMP_RES_ 

#pragma region Floating-point 128-bit support

#ifdef _float128_
#include <quadmath.h>
#endif


#ifdef _float128_
inline bool isnan(__float128 x) { return isnanq(x); }
inline bool isinf(__float128 x) { return isinfq(x); }
inline bool isfinite(__float128 x) { return finiteq(x); }
#endif

#ifdef _float128_
#define MAX_PREC 34
namespace std
{
inline __float128 pow (__float128 x, __float128 y)
{
 return powq (x, y);
}
inline __float128 fmod (__float128 x, __float128 y)
{
 return fmodq (x, y);
}
inline __float128 hypot (__float128 x, __float128 y)
{
 return hypotq (x, y);
}
inline __float128 atan2 (__float128 y, __float128 x)
{
 return atan2q (y, x);
}
inline __float128 sin (__float128 x)
{
 return sinq (x);
}
inline __float128 cos (__float128 x)
{
 return cosq (x);
}
inline __float128 tan (__float128 x)
{
 return tanq (x);
}
inline __float128 asin (__float128 x)
{
 return asinq (x);
}
inline __float128 acos (__float128 x)
{
 return acosq (x);
}
inline __float128 atan (__float128 x)
{
 return atanq (x);
}
inline __float128 sinh (__float128 x)
{
 return sinhq (x);
}
inline __float128 cosh (__float128 x)
{
 return coshq (x);
}
inline __float128 tanh (__float128 x)
{
 return tanhq (x);
}
inline __float128 sqrt (__float128 x)
{
 return sqrtq (x);
}
inline __float128 cbrt (__float128 x)
{
 return cbrtq (x);
}
inline __float128 exp (__float128 x)
{
 return expq (x);
}
inline __float128 log (__float128 x)
{
 return logq (x);
}
inline __float128 log10 (__float128 x)
{
 return log10q (x);
}
inline __float128 fabs (__float128 x)
{
 return fabsq (x);
}
inline __float128 floor (__float128 x)
{
 return floorq (x);
}
inline __float128 ceil (__float128 x)
{
 return ceilq (x);
}
inline __float128 round (__float128 x)
{
 return roundq (x);
}
inline bool isnan (__float128 x)
{
 return isnanq (x);
}
inline bool isinf (__float128 x)
{
 return isinfq (x);
}
inline bool isfinite (__float128 x)
{
 return finiteq (x);
}
}
// and in the global namespace too — for calls without std::
using std::acos;
using std::asin;
using std::atan;
using std::atan2;
using std::ceil;
using std::cos;
using std::cosh;
using std::exp;
using std::fabs;
using std::floor;
using std::fmod;
using std::hypot;
using std::log;
using std::log10;
using std::pow;
using std::round;
using std::sin;
using std::sinh;
using std::sqrt;
using std::cbrt;
using std::tan;
using std::tanh;
#endif
#ifdef _long_double_
typedef long double float__t;
#define MAX_PREC 20
#endif

#ifdef _float128_
typedef __float128 float__t;
#define fabsl  fabsq
#endif


//const double maxdbl = DBL_MAX; // 1.7976931348623157e+308
#ifdef __BORLANDC__
#define GetTickCount64 GetTickCount
const float__t qnan = 0.0 / 0.0;
const float__t inf = 1.0 / 0.0;
#else
constexpr float__t qnan = std::numeric_limits<float__t>::quiet_NaN ();
constexpr float__t inf = std::numeric_limits<float__t>::infinity ();
#endif
#pragma endregion

#pragma region Value and symbol definitions
class value;
class symbol;

enum t_value // t_value represents the type of a value in the calculator
{
 tvERR,
 tvINT,
 tvFLOAT,
 tvPERCENT,
 tvCOMPLEX,
 tvSTR,
 tvMATRIX,
 tvMX_ELEM,
 tvUFUNCT,
 tvSOLVE,
 tvSOLVEA,
 tvINTEGR,
 tvDIFF,
 tvEXTR,
 tvINVERSE,
 tvFOR,
 tvPLOT,
 tvBMP,
 tvCOLOR,
 tvTBLFN,
};

#define MSK_ERR     (1 << tvERR)     //  0 Mask for error values
#define MSK_INT     (1 << tvINT)     //  1 Mask for integer values
#define MSK_FLOAT   (1 << tvFLOAT)   //  2 Mask for float values   
#define MSK_PERCENT (1 << tvPERCENT) //  3 Mask for percentage values
#define MSK_COMPLEX (1 << tvCOMPLEX) //  4 Mask for complex values
#define MSK_STR     (1 << tvSTR)     //  5 Mask for string values
#define MSK_MATRIX  (1 << tvMATRIX)  //  6 Mask for matrix values
#define MSK_MX_ELEM (1 << tvMX_ELEM) //  7 Mask for matrix element values
#define MSK_UFUNCT  (1 << tvUFUNCT)  //  8 Mask for user-defined function values
#define MSK_SOLVE   (1 << tvSOLVE)   //  9 Mask for solve operator values
#define MSK_SOLVEA  (1 << tvSOLVEA)  // 10 Mask for alternative solve operator values
#define MSK_INTEGR  (1 << tvINTEGR)  // 11 Mask for integration operator values
#define MSK_DIFF    (1 << tvDIFF)    // 12 Mask for differentiation operator values
#define MSK_EXTR    (1 << tvEXTR)    // 13 Mask for extremum operator values
#define MSK_FOR     (1 << tvFOR)     // 14 Mask for for operator values
#define MSK_BMP     (1 << tvBMP)     // 15 Mask for bitmap operator values
#define MSK_INVERCE (1 << tvINVERSE) // 16 Mask for inverse operator values
#define MSK_COLOR   (1 << tvCOLOR)   // 17 Mask for color operator values
#define MSK_TBLFN   (1 << tvTBLFN)   // 18 Mask for table function operator values

#define MSK_SCALAR (MSK_INT | MSK_FLOAT | MSK_PERCENT) // Mask for scalar values
#define MSK_ALLVAR (MSK_SCALAR | MSK_COMPLEX | MSK_STR | MSK_MATRIX | MSK_BMP | MSK_COLOR | MSK_UFUNCT)                                                                                    \
 
#define MAX_R 7
#define MAX_C 7

#define TIMEOUT 10000 // 10 sec Timeout in milliseconds for long calculations
#define MAX_TIMEOUT 60000 // 60 sec Maximum timeout in milliseconds for long calculations

enum t_operator // t_operator represents the type of an operator in the calculator
{
 toBEGIN,    // 0  toBEGIN must be the first operator in the list
 toOPERAND,  // 1  toOPERAND represents an operand in the expression
 toERROR,    // 2  toERROR represents an error in the expression
 toEND,      // 3  toEND represents the end of the expression
 toLPAR,     // 4  toLPAR represents a left parenthesis '('
 toRPAR,     // 5  toRPAR represents a right parenthesis ')'
 toFUNC,     // 6  toFUNC represents a function
 toSOLVE,    // 7  toSOLVE represents a solve, integr and diff function
 toPOSTINC,  // 8  toPOSTINC represents a post-increment (v++) operator
 toPOSTDEC,  // 9  toPOSTDEC represents a post-decrement (v--) operator
 toFACT,     // 10 toFACT represents a factorial 'n!' operator
 toPREINC,   // 11 toPREINC represents a pre-increment (++v) operator
 toPREDEC,   // 12 toPREDEC represents a pre-decrement (--v) operator
 toPLUS,     // 13 toPLUS represents an '+v' operator
 toMINUS,    // 14 toMINUS represents a '-v' operator
 toNOT,      // 15 toNOT represents a logical NOT operator
 toCOM,      // 16 toCOM represents a bitwise complement '~' operator
 toPOW,      // 17 toPOW represents a power '^' operator
 toPERCENT,  // 18 toPERCENT represents a percentage '%' operator
 toMUL,      // 19 toMUL represents a multiplication operator
 toDIV,      // 20 toDIV represents a division operator
 toMOD,      // 21 toMOD represents a modulo operator
 toPAR,      // 22 toPAR represents a parenthesis operator
 toADD,      // 23 toADD represents an addition operator
 toSUB,      // 24 toSUB represents a subtraction operator
 toASL,      // 25 toASL represents a arithmetic shift left operator
 toASR,      // 26 toASR represents a arithmetic shift right operator
 toLSR,      // 27 toLSR represents a logical shift right operator
 toGT,       // 28 toGT represents a greater than operator
 toGE,       // 29 toGE represents a greater than or equal operator
 toLT,       // 30 toLT represents a less than operator
 toLE,       // 31 toLE represents a less than or equal operator
 toEQ,       // 32 toEQ represents an equality operator
 toNE,       // 33 toNE represents a not equal operator
 toAND,      // 34 toAND represents a logical AND operator
 toXOR,      // 35 toXOR represents a logical XOR operator
 toOR,       // 36 toOR represents a logical OR operator
 toMX_ELEM,  // 37 toMX_ELEM represents a matrix element operator
 toSET,      // 38 toSET represents a set operator
 toSETADD,   // 39 toSETADD represents a set addition operator
 toSETSUB,   // 40 toSETSUB represents a set subtraction operator
 toSETMUL,   // 41 toSETMUL represents a set multiplication operator
 toSETDIV,   // 42 toSETDIV represents a set division operator
 toSETMOD,   // 43 toSETMOD represents a set modulo operator
 toSETASL,   // 44 toSETASL represents a set arithmetic shift left operator
 toSETASR,   // 45 toSETASR represents a set arithmetic shift right operator
 toSETLSR,   // 46 toSETLSR represents a set logical shift right operator
 toSETAND,   // 47 toSETAND represents a set logical AND operator
 toSETXOR,   // 48 toSETXOR represents a set logical XOR operator
 toSETOR,    // 49 toSETOR represents a set logical OR operator
 toSETPOW,   // 50 toSETPOW represents a set power operator
 toSEMI,     // 51 toSEMI represents a semicolon operator
 toCOMMA,    // 52 toCOMMA represents a comma operator
 toCONTINUE, // 53 toCONTINUE represents a continue operator for continue scanning
 toTERMINALS // 54 toTERMINALS must be the last operator in the list and represents the total number of
             // operators
};

enum t_symbol // t_symbol represents the type of a symbol in the calculator
{
 tsVARIABLE, // 0  tsVARIABLE represents a variable symbol
 tsCONSTANT, // 1  tsCONSTANT represents a constant symbol
 tsFFUNCI1,  // 2  float f(int x) (floatf)
 tsIFUNCF1,  // 3  int f(float x) (wrgb, trgb)
 tsSFUNCF1,  // 4  char* f(float x) (winf)
 tsCIFUNC1,  // 5  int f(this, int x) (prec)
 tsIFUNC1,   // 6  int f(int x) (now, not)
 tsIFUNC2,   // 7  int f(int x, int y) (gcd)
 tsFFUNC1,   // 8  float f(float x) (swg, awg)
 tsFFUNC2,   // 9  float f(float x, float y) (min, max)
 tsFFUNC3,   // 10  float f(float x, float y, float z) (vout)
 tsPFUNCn,   // 11  char * printf(char *format, ...) (prn)
 tsFPFUNCn,  // 12  int printf(char *fname, char *format, ...) (prnf)
 tsSFUNCF2,  // 13  float const(char *name, float value) (const, var)
 tsSIFUNC1,  // 14  int f(char *s) (datatime)
 tsFFUNCM,   // 15  float f(matrix M) (det, trace)
 tsMFUNCM2,  // 16  matrix f(matrix A, matrix B) (dot, cross)
 tsMFUNCI2,  // 17  matrix f(int r, int c) (zeros, eye
 tsVFUNC1,   // 18  void vfunc(value* res, value* arg, int idx) (abs, sin)
 tsVFUNC2,   // 19  void vfunc(value* res, value* arg1, value* arg2, int idx) (pow, rootn
 tsUFUNCT,   // 20  User-defined function
 tsSOLVE,    // 21  Solve operator for solving equations (solve)
 tsCALC,     // 22  Calculate operator for evaluating expressions (calc)
 tsINTEGR,   // 23  Integration operator for numerical integration (integr)
 tsSUM,      // 24  Summation operator for numerical summation (sum)
 tsDIFF,     // 25  Differentiation operator for numerical differentiation (diff)
 tsFOR,      // 26  For operator (for)
 tsIF,       // 27  if operator for conditional expressions (if)
 tsSFUNCI1,  // 28  char* f(int n) (factorize)
 tsFITFN,    // 29  matrix fitpoly("data", int n) Fit function for curve fitting 
 tsSTFUN,    // 30  float mean("data") statistics function for calculating mean, median, etc
 tsCLCFN,    // 31  pair to fit* clcpoly([(1,2,3)], 10.5) float clcpoly(Matrix data, float x)
 tsPLOT,     // 32  plot operator for plotting data (plot)
 tsFDLG,     // 33  char* f(char *s) (fdlg("*.bmp"))
 tsRUN,      // 34  value f(char *s) load("user.txt") load calculator's script. Return any type.
 tsDATAF,    // 35  dataf operator for loading data from file (dataf("data.txt","mask", x1, x2, ...))
 tsERROR,    // 36  error("message") operator for reporting errors
 tsEXTR,     // 37  extremum operator for finding local minima and maxima (extr)
 tsLDSV,     // 38  load/save operator for loading and saving variables.
 tsPLOTREG,  // 39  plotreg(xmin, xmax, ymin, ymax) - define plot region
 tsINVERSE,  // 40  inverse operator for calculating the inverse of a function (inverse)
 tsCOLOR,    // 41  color(r,g,b) operator for creating color values 
 tsSCRIPT,   // 42  script service finction
 tsGUI,      // 43  GUI service functions
 tsTBLFN,    // 44  table function for creating tables of values (tblfn)
 tsNUM       // 45  Total number of symbol types, must be the last in the list
};

enum t_mresult
{
 mrDONE,    // represents a successful result
 mrSKIP,    // represents a result that indicates to skip the current operation or iteration
 mrERROR,   // represents an error result
};

enum t_br_result
{
 brNONE,    // Nothing to break, continue normal execution
 brTIMEOUT, // Indicates that a timeout has occurred during a long calculation
 brESC,     // Indicates that an ESC key was pressed
};

#define MASK_ALL        (uint64_t)~0ULL         // 64bit mask with all symbol types included
#define MASK_NONE       0ULL                    // represents an empty mask with no symbol types included
#define MASK_VARIABLE   (1ULL << tsVARIABLE)    // represents a variable symbol
#define MASK_CONSTANT   (1ULL << tsCONSTANT)    // represents a constant symbol
#define MASK_FFUNCI1    (1ULL << tsFFUNCI1)     // float f(int x)
#define MASK_IFUNCF1    (1ULL << tsIFUNCF1)     // int f(float x)
#define MASK_SFUNCF1    (1ULL << tsSFUNCF1)     // char* f(float x)
#define MASK_IFUNC1     (1ULL << tsIFUNC1)      // int f(int x)
#define MASK_CIFUNC1    (1ULL << tsCIFUNC1)     // int f(this, int x)
#define MASK_IFUNC2     (1ULL << tsIFUNC2)      // int f(int x, int y)
#define MASK_FFUNC1     (1ULL << tsFFUNC1)      // float f(float x)
#define MASK_FFUNC2     (1ULL << tsFFUNC2)      // float f(float x, float y)
#define MASK_FFUNC3     (1ULL << tsFFUNC3)      // float f(float x, float y, float z)
#define MASK_PFUNCn     (1ULL << tsPFUNCn)      // int printf(char *format, ...)
#define MASK_FPFUNCn    (1ULL << tsFPFUNCn)     // int printf(char *fname, char *format, ...)
#define MASK_SFUNCF2    (1ULL << tsSFUNCF2)     // float const(char *name, float value)
#define MASK_SIFUNC1    (1ULL << tsSIFUNC1)     // int f(char *name)
#define MASK_FFUNCM     (1ULL << tsFFUNCM)      // float f(matrix M)
#define MASK_MFUNCM2    (1ULL << tsMFUNCM2)     // matrix f(matrix A, matrix B)
#define MASK_MFUNCI2    (1ULL << tsMFUNCI2)     // matrix f(int r, int c)
#define MASK_VFUNC1     (1ULL << tsVFUNC1)      // void vfunc(value* res, value* arg, int idx)
#define MASK_VFUNC2     (1ULL << tsVFUNC2)      // void vfunc(value* res, value* arg1, value* arg2, int idx)
#define MASK_UFUNCT     (1ULL << tsUFUNCT)      // user-defined function
#define MASK_SOLVE      (1ULL << tsSOLVE)       // solve operator for solving equations
#define MASK_SUM        (1ULL << tsSUM)         // summation operator for numerical summation
#define MASK_CALC       (1ULL << tsCALC)        // calculate operator for evaluating expressions
#define MASK_INTEGR     (1ULL << tsINTEGR)      // integration operator for numerical integration
#define MASK_DIFF       (1ULL << tsDIFF)        // differentiation operator for numerical differentiation
#define MASK_FOR        (1ULL << tsFOR)         // for operator for numerical summation
#define MASK_IF         (1ULL << tsIF)          // if operator for conditional expressions
#define MASK_FITFN      (1ULL << tsFITFN)       // Fit function for curve fitting
#define MASK_STFUN      (1ULL << tsSTFUN)       // Statistics function for calculating mean, median, etc
#define MASK_CLCFN      (1ULL << tsCLCFN)       // clcpoly function for curve fitting with confidence intervals 
#define MASK_PLOT       (1ULL << tsPLOT)        // plot operator for plotting data
#define MASK_FDLG       (1ULL << tsFDLG)        // char* f(char *s) (fdlg("*.bmp"))
#define MASK_RUN        (1ULL << tsRUN)         // value f(char *s) run("script.txt") run calculator's script. Return any type.
#define MASK_DATAF      (1ULL << tsDATAF)       // dataf operator for loading data from file (dataf("data.txt","mask", x1, x2, ...))
#define MASK_EXTR       (1ULL << tsEXTR)        // extremum operator for finding local minima and maxima (extr)
#define MASK_LDSV       (1ULL << tsLDSV)        // load/save operator for loading and saving variables
#define MASK_PLOTREG    (1ULL << tsPLOTREG)     // plotreg(xmin, xmax, ymin, ymax) - define plot region
#define MASK_INVERSE    (1ULL << tsINVERSE)     // inverse operator for calculating the inverse of a function (inverse)
#define MASK_SCRIPT     (1ULL << tsSCRIPT)      // Script service
#define MASK_GUI        (1ULL << tsGUI)         // GUI service
#define MASK_TBLFN      (1ULL << tsTBLFN)       // table function for creating tables of values (tblfn)

// default mask for user defined functions, excludes variables
#define MASK_DEFAULT ((uint64_t)(MASK_ALL & ~(MASK_VARIABLE|MASK_PLOT|MASK_FDLG|MASK_GUI))) 

enum v_func // v_func represents the index of a built-in function in the calculator
{
 vf_abs, // Absolute value function
 vf_pol, // Polar coordinates function

 vf_sin, // Sine function
 vf_cos, // Cosine function
 vf_tan, // Tangent function
 vf_cot, // Cotangent function

 vf_sinh, // Hyperbolic sine function
 vf_cosh, // Hyperbolic cosine function
 vf_tanh, // Hyperbolic tangent function
 vf_ctnh, // Hyperbolic cotangent function

 vf_asin, // Arcsine function
 vf_acos, // Arccosine function
 vf_atan, // Arctangent function
 vf_acot, // Arccotangent function

 vf_asinh, // Arcsine hyperbolic sine function
 vf_acosh, // Arccosine hyperbolic cosine function
 vf_atanh, // Arctangent hyperbolic tangent function
 vf_acoth, // Arccotangent hyperbolic cotangent function

 vf_exp,   // Exponential function
 vf_log,   // Natural logarithm function
 vf_log10, // Logarithm base 10 function
 vf_log2,  // Logarithm base 2 function
 vf_exp10, // Exponential function with base 10
 vf_sqrt,  // Square root function

 vf_pow,   // Power function
 vf_root3, // Cube root function
 vf_rootn, // N-th root function
 vf_logn,  // Logarithm with specified base function

 vf_re,    // Real part of a complex number function
 vf_im,    // Imaginary part of a complex number function
 vf_isnan, // Check if value is NaN function
 vf_conj,  // Complex conjugate function
 vf_cplx,  // Complex number construction function
 vf_polar, // Polar coordinates construction function
 vf_hypot, // Hypotenuse function
 vf_atan2, // Arctangent of two variables function
 vf_fmod,  // Modulo function
 vf_rand,  // Random function

 vf_floor, // Floor function
 vf_ceil,  // Ceiling function
 vf_round, // Round function
 vf_frac,  // Fractional part function
 vf_int,   // Integer part function
 vf_float, // Float function

 vf_db,    // Decibel function (10*log10(x) for power quantities, 20*log10(x) for field quantities)
 vf_np,    // Neper function (20*log10(x) for power quantities)
 vf_adb,   // Inverse decibel function (10^(x/10) for power quantities, 10^(x/20) for field quantities)
 vf_anp,   // Inverse neper function (10^(x/20) for power quantities)

 vf_pol_rt, //Polynom root

 vf_factorial, // Factorial function

 // Cartesian coordinates
 pl_plot,       // plot plot Cartesian to screen

 // Polar coordinates
 pl_plotpol,    // plotpol polar plot to screen

 // Parametric (X-Y) plot
 pl_xyplot,     // xyplot xy plot to screen

 // Logarithmic plots
 pl_plotlgx, // log X axis, linear Y
 pl_plotlgy, // linear X, log Y axis
 pl_plotlgxy, // log both X and Y axes

 // Smith chart
 pl_plotsmith,  // smith(expr, from, to, var) - Z0=50Ω
 pl_plotsmithz,  // smithz(expr, from, to, var, Z0)

 // Data plot functions
 pl_plotdata,  // plotdata(datafile, mask)
 pl_plotdatal,  // plotdatal(datafile, mask) - with lines

 pl_any,        // special value for plotreg  
 fn_plotreg, // plotreg(xmin, xmax, ymin, ymax) - define plot region

 rtPoly, // Linear regression (polynomial fit of degree up to 6)
 rtExp,  // Exponential regression (y = a * exp(b * x))
 rtLg,   // Logarithmic regression (y = a + b * ln(x))
 rtPow,  // Power regression (y = a * x^b)
 rtInv,  // Inverse regression (y = a + b / x)

 sfNum,     // Number of elements in the dataset
 sfMean,    // Mean (average) of the dataset
 sfMedian,  // Median of the dataset
 sfRMS,     // Root mean square of the dataset
 sfSumX,    // Sum of the elements in the dataset
 sfStdDevP, // Population standard deviation of the dataset
 sfStdDevS, // Sample standard deviation of the dataset
 sfMin,     // Minimum value in the dataset
 sfMax,     // Maximum value in the dataset
 sfNormP,   // Probability that a value from the dataset is <= x.
 sfNormQ,   // Probability of falling within |x - mean| range.
 sfNormR,   // Probability that a value is > x (upper tail).
 sfInvNorm, // Inverse of the normal distribution function (returns the z-score for a given
            // cumulative probability)
 sfNormPD,  // Probability density function of the normal distribution (returns the height
            // of the normal distribution curve at a given x value)

 mx_Rows,  // Get the number of rows in a matrix
 mx_Cols,  // Get the number of columns in a matrix
 mx_Size,  // Get the total number of elements in a matrix 
 mx_Trace, // Get the trace of a matrix (sum of diagonal elements)
 mx_Det,   // Get the determinant of a matrix
 mx_Norm,  // Get the norm of a matrix
 mx_Dim,   // Get the dimensions of a matrix
 mx_Dot,   // Get the dot product of two matrices
 mx_Cross, // Get the cross product of two matrices
 mx_Zeros, // Get a matrix filled with zeros
 mx_Diag,  // Get a diagonal matrix 

 ssFdlg,    // File dialog function for selecting files (e.g., for plotting)

 dfDataf, 
 dfDatas,

 ccPrec,    // Set or get the calculator's precision for floating-point calculations
 ccOpt,     // Set the calculator's options (e.g., output format, angle mode, etc.)
 ccOptOn,   // Set specific options (e.g., enable/disable features) without affecting other options
 ccOptOff,  // Clear specific options without affecting other options
 ccTimeout,

 scRun,    // Run a script file containing calculator commands
 scEval,   // Evaluate an expression from a string
 scVars,   // Script vars function

 vrConst, // Create a constant variable
 vrVar,   // Create a variable
 vrLoad,   // Load variable from a file
 vrSave,   // Save variable to a file

 vf_num
};

typedef v_func rtype; // rtype represents the type of regression (fit) function in the calculator
typedef v_func sfntype; // sfntype represents the type of statistical function in the

enum terr
{
 teSyntax,
 teMath,
};

#pragma endregion

#pragma region Sybols and values classes
class value // value represents a value in the calculator, which can be an integer, float, complex
            // number, string, or user-defined function
{
 public:
 t_value tag; // Type of value
 uint32_t info; // Additional info for the value (e.g., flags, precision, etc.)
 symbol *var; // Uses for variables and functions
 int pos;     // Position in expression for error reporting
 
 int_t ival;     // Integer value
 float__t fval;  // Float value or real part of complex value
 float__t imval; // Imaginary part of complex value
 char *sval;     // String value
 uint8_t mrows;  // Number of rows in matrix
 uint8_t mcols;  // Number of columns in matrix
 uint8_t irows;  // Number of rows in matrix
 uint8_t icols;  // Number of columns in matrix
 float__t *mval; // Matrix value (pointer to array of floats)

 inline value ()
 {
  tag   = tvERR; //tvINT;
  info  = 0;
  var   = nullptr; // Uses for variables and functions
  ival  = 0;
  fval  = 0.0;
  imval = 0.0;
  pos   = 0;
  sval  = nullptr;
  mrows = 0;
  mcols = 0;
  irows = 0;
  icols = 0;
  mval  = nullptr;
 }

 inline float__t get () { return tag == tvINT ? (float__t)ival : fval; }
 inline int_t get_int () { return tag == tvINT ? ival : (int_t)fval; }
 inline char *get_str () { return tag == tvSTR ? sval : nullptr; }
 inline bool is_scalar () { return tag == tvINT || tag == tvFLOAT; }
};

class symbol // symbol represents a symbol in the calculator, which can be a variable, constant, or
             // function
{
 public:
 t_symbol tag; // Type of symbol
 v_func fidx;  // Function index
 void *func;   // Function pointer
 bool block;   // true if the symbol is a block (e.g., for gui, longtime, etc.)
 value val;    // Value associated with the symbol
 char name[MAXNAME]; // Name of the symbol (fixed-size array to avoid dynamic memory allocation)
 symbol *next; // Next symbol in the hash table chain

 inline symbol ()
 {
  tag  = tsVARIABLE;
  fidx = vf_num;
  func = nullptr;
  block   = false;
  name[0] = '\0'; // Initialize name to an empty string
  next = nullptr;
 }
};
#pragma endregion


#pragma region Memory management class

enum ptr_type
{
 ptMALLOC = 0, // Default: C-style malloc/free
 ptBMP    = 1  // C++ bmpdraw* objects (new/delete)
};

struct mem_entry
{
 void *ptr;
 ptr_type type;
};

class MemList
{
 mem_entry *list;
 int capacity;
 int count;

 public:
 MemList (int initial = 256) : capacity (initial), count (0)
 {
  list = (mem_entry *)malloc (capacity * sizeof (mem_entry));
  if (list) memset (list, 0, capacity * sizeof (mem_entry));
 }

 ~MemList () { free (list); }

 void init_mem_list ()
 {
  if (list) memset (list, 0, capacity * sizeof (mem_entry));
  count = 0;
 }

 int search_mem (void *mem)
 {
  for (int i = 0; i < count; i++)
   if (list[i].ptr == mem) return i;
  return -1;
 }

 void *register_mem (void *mem, ptr_type type = ptMALLOC)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1) return mem; // already registered

  // fill holes first
  for (int i = 0; i < count; i++)
   if (!list[i].ptr)
    {
     list[i].ptr  = mem;
     list[i].type = type;
#ifdef _DEBUG_MEM_   
     if (type == ptBMP)
      {
       printf ("Registering bmpdraw* object at %p\n", mem);
      }
#endif
     return mem;
    }

  // no holes - append
  if (count < capacity)
   {
    list[count].ptr  = mem;
    list[count].type = type;
    count++;
#ifdef _DEBUG_MEM_   
    if (type == ptBMP)
     {
      printf ("Registering bmpdraw* object at %p\n", mem);
     }
#endif
    return mem;
   }

  // grow
  int newcap         = capacity * 2;
  mem_entry *newlist = (mem_entry *)realloc (list, newcap * sizeof (mem_entry));
  if (!newlist) return nullptr;
  list = newlist;
  memset (list + capacity, 0, (newcap - capacity) * sizeof (mem_entry));
  capacity         = newcap;
  list[count].ptr  = mem;
  list[count].type = type;
#ifdef _DEBUG_MEM_   
  if (type == ptBMP)
   {
    printf ("Registering bmpdraw* object at %p\n", mem);
   }
#endif
  count++;
  return mem;
 }

 void *unregister_mem (void *mem)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1)
   {
    list[idx].ptr  = nullptr;
    list[idx].type = ptMALLOC;
   }
  return mem;
 }

 void *sf_alloc (int size, ptr_type type = ptMALLOC)
 {
  if (type != ptMALLOC) return nullptr; // Invalid type
  if (!size) return nullptr;
  void *mem = malloc (size);
  if (mem) register_mem (mem, type);
  return mem;
 }

 void sf_free (void *dat, ptr_type type = ptMALLOC)
 {
  if (dat)
   {
    int idx = search_mem (dat);
    if (idx != -1)
     {
      switch (list[idx].type)
       {
       case ptBMP:
#ifdef _DEBUG_MEM_   
        printf ("Freeing bmpdraw* object at %p\n", dat); 
#endif
        delete (bmpdraw *)dat;
        break;
       case ptMALLOC:
       default:
        free (dat);
        break;
       }
      list[idx].ptr  = nullptr;
      list[idx].type = ptMALLOC;
     }
    else
     { // Not found in list, free anyway
      switch (type)
       {
       case ptBMP:
#ifdef _DEBUG_MEM_   
        printf ("Freeing bmpdraw* object at %p (not found in list)\n", dat); 
#endif
        delete (bmpdraw *)dat;
        break;
       case ptMALLOC:
       default:
        free (dat);
        break;
       }
     }
   }
 }

 void free_all ()
 {
  for (int i = 0; i < count; i++)
   {
    if (list[i].ptr)
     {
      switch (list[i].type)
       {
       case ptBMP:
#ifdef _DEBUG_MEM_   
        printf ("Freeing bmpdraw* object at %p\n", list[i].ptr);
#endif
        delete (bmpdraw *)list[i].ptr;
        break;

       case ptMALLOC:
       default:
        free (list[i].ptr);
        break;
       }
      list[i].ptr = nullptr;
     }
   }
  count = 0;
 }

 int size () const { return count; }
};
#pragma endregion
const int max_stack_size        = 256;  // Maximum size of value and operator stacks
const int max_expression_length = 1024; // Maximum length of expression
const int hash_table_size = 1013; // Size of hash table for variables and functions

struct GKResult
{
 float__t value;
 float__t error;
 bool ok;
};

struct mxresult_t
{
 uint8_t rows;  // Number of rows in the matrix
 uint8_t cols;  // Number of columns in the matrix
 float__t *mval; // Matrix values (pointer to array of floats)
};

#define SUPPORT_TABLEFN // Enable support for table functions (interpolation)

#ifdef SUPPORT_TABLEFN
struct datapoint
{
 double x;
 double y;
};

enum t_appr
{
 ap_linear,
 ap_spline
};

struct tablefn_data
{
 int num;         // Number of data points
 t_appr appr;     // Approximation type (linear/spline)
 datapoint *data; // Array of data points (sorted by x)

 // === Optimization: cached search state ===
 int last_idx;  // Last used interval index (hint for next search)
 double last_x; // Last queried x value

 // === Precomputed bounds (for fast range check) ===
 double x_min; // Minimum x value (data[0].x)
 double x_max; // Maximum x value (data[num-1].x)

 // === Statistics (optional, for debugging) ===
 int eval_count; // Number of evaluations performed
 int cache_hits; // Number of times last_idx was useful
};
#endif
typedef bool (*fnShowImage) (void *bmpObject); // Pointer to function for showing an image
typedef int (*debug_callback_t) (void *context, const char *fmt, ...);// Debug callback function type

int_t scan_opt (char *str, int_t &opts);
int Mxprint (t_value tag, int8_t res_rows, int8_t res_cols, 
             float__t *res_mval, char *str, bool nl, int *size);

class calculator // calculator represents the main class for the expression calculator, which
                 // manages the state of the calculator, including variables, functions, stacks, and
                 // parsing logic
{
 private:

// Structure for storing plot parameters
 struct PlotParams
   {
    char *sexpr;         // Expression of the function
    char *sexpr_y;       // Expression of the function for Y
    char *svar;          // Variable
    float__t vfrom;      // Start of the range
    float__t vto;        // End of the range
    float__t ymin;       // Minimum Y
    float__t ymax;       // Maximum Y
    float__t xmin;       // for polar plots
    float__t xmax;       // for polar plots
    float__t scale;      // for polar plots
    float__t z0;         // Reference impedance for Smith chart
    bool log_x;          // logarithmic X axis
    bool log_y;          // logarithmic Y axis
    int width;           // Image width
    int height;          // Image height
    int top;
    int left;
    int padding;         // Padding
    bool dot;            // dot or line plotting.   
    uint32_t pxsize;     // pixel size
    uint32_t bgc;        // Background color
    uint32_t fgc;        // Plot line color
    uint32_t grid_color; // Grid color
    uint32_t axis_color; // Axis color
    uint32_t text_color; // Text color
    calculator *child;   // Calculator for computations
   };
         
 int_t scfg;      // Syntax configuration flags
 int_t fflags;   // Founded format flags
 int deep; // Current stack depth
 bool blockflag; // Flag to indicate if we are currently parsing a block (e.g., for GUI functions, loops, etc.)
 value v_stack[max_stack_size]; // Value stack for operands
 symbol *hash_table[hash_table_size]; // Hash table for variables and functions
 t_operator o_stack[max_stack_size]; // Operator stack
 MemList mem_list; // Memory list for temporary strings used during expression parsing and evaluation
 filesystem fs; // Filesystem object for file reading

 int v_sp; // Value stack pointer
 int o_sp; // Operator stack pointer
 char *buf; // Buffer for expression parsing
 int pos;   // Current position in the expression during parsing
 int tmp_var_count;
 char err[80]; // Error message buffer
 char mxerr[80]; // Error message buffer for matrix operations
 int errpos;   // Error position
 terr errtype;
 int fprec;      // Floating point precision for output formatting
 char c_imaginary; // Imaginary unit character
 uint64_t timeout; // Timeout for long calculations in milliseconds
 float__t Plot_Ymax; // Maximum Y value for plotting, used for autoscaling
 float__t Plot_Ymin; // Minimum Y value for plotting, used for autoscaling
 float__t Plot_Xmax; // Maximum X value for plotting, used for autoscaling
 float__t Plot_Xmin; // Minimum X value for plotting, used for autoscaling
 float__t Plot_Tmax; // Maximum T value for plotting, used for autoscaling
 float__t Plot_Tmin; // Minimum T value for plotting, used for autoscaling
 float__t Plot_Rmax; // Maximum R value for plotting, used for autoscaling
 v_func PlotFunc; // Function index for plotting, used to identify which function is being plotted
                  // for autoscaling

 bool expr;    // Expression flag
 char sres[STRBUF]; // String result buffer
 char lastvar[MAXOP];  // Last variable name used in the expression, if it is a string

 int (*EscFn) (void);
 bool (*FileDlgFn) (char*, int);
 fnShowImage ShowImageFn;
 debug_callback_t debugFn;

 uint8_t res_cols; // Number of columns in the matrix result
 uint8_t res_rows;   // Number of rows in the matrix result
 float__t *res_mval; // Matrix result (pointer to array of floats)

 int64_t result_ival; // Integer result
 float__t result_fval; // Float result
 float__t result_imval; // Imaginary part of complex result
 t_value result_tag; // Type of result
 uint32_t result_info;  // Additional info for the result (e.g., flags, precision, etc.)

 void AddPredefined (void);

 // copy symbols from parent calculator with specified mask
 void copy_symbols (symbol **symtab = nullptr, uint64_t mask = (MASK_NONE));

 //memory management
 void init_mem_list () { mem_list.init_mem_list (); }
 int search_mem (void *mem) { return mem_list.search_mem (mem); }
 void *register_mem (void *mem, ptr_type type = ptMALLOC) { return mem_list.register_mem (mem, type); }
 void *unregister_mem (void *mem) { return mem_list.unregister_mem (mem); }
 void *sf_alloc (int size, ptr_type type = ptMALLOC) { return mem_list.sf_alloc (size, type); }
 void sf_free (void *dat, ptr_type type = ptMALLOC) { mem_list.sf_free (dat, type); }
 void clear_mem_list (void) { mem_list.free_all (); }

 // sybol table management
 void save_vars_mem (void); // Save the current variables in the hash table to the mem array for
                            // memory management
 char *dupString (const char *src); // Duplicate a string and register it in the string list
 bmpdraw *dupBMP (bmpdraw *src); // Duplicate a bitmap and register it in the memory list

 bool dupvar (value &dst, value &src); // Duplicate a variable value and register any dynamically
                                       // allocated memory in the memory list 
 bool freevar (value &src); // Free the memory associated with a variable value and unregister it
                            // from the memory list

 void destroyvars (void); // Destroy all variables in the hash table
 inline unsigned string_hash_function (const char *p); // Hash function for strings
 symbol *add (t_symbol tag, const char *name, 
              void *func = nullptr, bool block = false); // Add a symbol to the hash table
 symbol *add (t_symbol tag, v_func fidx, const char *name,
              void *func = nullptr, bool block = false); // Add a symbol with function index to the hash table
 symbol *find (const char *name);    // Find a symbol in the hash table by name
 symbol *addUF (const char *name, const char *expr); // Add a user-defined function to the calculator
                                                    // with the given name and expression
 inline char *get_last_var (void) { return lastvar;}; // Get the last variable name assigned in the expression
 float__t AddConst (const char *name, float__t val); // Add a constant to the calculator and return its value
 float__t AddVar (const char *name, float__t val); // Add a variable to the calculator and return its value
 bool addvar (const char *name, value &val); // Add a variable with a specified value to the calculator
 bool addconst (const char *name, value &val); // Add a constant with a specified value to the calculator
 void addfconst (const char *name, float__t val); // Add a floating-point constant to the calculator
 void addfvar (const char *name, float__t fval, float__t imval = (float__t)0.0L);
 void addivar (const char *name, int_t ival);
 void addsvar (const char *name, const char *svar);
 int_t getivar (const char *name);
 float__t getfvar (const char *name);
 bool getvar (const char *name, value &val); // Get the value of a variable by name 
 char *getsvar (const char *name);
 void addiconst (const char *name, int_t val); // Add an integer variable to the calculator
 void addlconst (const char *name, float__t fval, int_t ival); // Add a long constant to the calculator
 void import_child (calculator *child, uint32_t mask); // import symbols from child calculator with specified mask


 // Expression parsing
 void isNRM (char *start, char *end); // Check if the current position in the expression is a normalized number format
 t_operator sscan (symbol *sym); // Scan body of the solve, integr and diff 

 t_operator sqbraces (void); // Scan [..] matrix/vector constructor 
 
 t_operator braces (void);    // Scan the expression for parentheses and return the operator type of
                              // the next token after the parentheses
 t_operator dqscan (char qc); // Scan the double quote string in the expression and return its operator
                              // type, used for main scan
 t_operator dscan (bool operand, bool percent); // Scan the digits in the expression and return its 
                                                //operator type, used for main scan
 t_operator scan (bool operand, bool percent); // Scan the next token in the expression and return 
                                               // its operator type
 int mx_idx (int &row, int &col); // Scan the matrix index in the expression and return true if it is valid,
                                   // with row and col being the parsed row and column indices

 // Error handling
 void error (int pos, const char *msg, terr errt = teSyntax); // Report an error at the given position with the specified message
 void errorf (int pos, const char *fmt, ...); // Report an error at the given position with a formatted message
 inline void error (const char *msg) {error (pos - 1, msg, teSyntax);} // Report an error at the current position with 
                                                                       //the specified message

 //Functions and operators arguments check helpers.
 bool CheckOperand (int sp, uint32_t mask);
 bool CheckFnArgs (int n_args, int expected_args, const uint32_t mask[]); 
 bool CheckOpArgs (int n_args, const uint32_t mask[]);
 bool isMxIdx1 ();
 bool isMxIdx2 ();

 // Format checking and conversion
 bool isCMP (char *&fpos); // Check if the current position is a computing format
 int hscanf (char *str, int_t &ival,  int &nn); // Scan a hexadecimal number from the string and store it in ival,
                                                // with nn being the number of characters processed
 int bscanf (char *str, int_t &ival, int &nn); // Scan a binary number from the string and store it in ival, 
                                               //with nn being the number of characters processed
 int oscanf (char *str, int_t &ival, int &nn); // Scan an octal number from the string and store it in ival, 
                                               //with nn being the number of characters processed
 int xscanf (char *str, int len, int_t &ival, int &nn); // Scan a hexadecimal number from the string 
                                               //with a specified length and store it in ival, with nn being 
                                               // the number of characters processed
 double dstrtod (char *s, char **endptr); // Convert a string to a double-precision floating-point number
 double tstrtod (char *s, char **endptr); // Convert a string to a double-precision floating-point number
 void engineering (double mul, char *&fpos, double &fval); // Perform engineering notation conversion
 void scientific (char *&fpos, double &fval); // Perform scientific notation conversion
 
 bool set_op (); // Assign a value to a variable
 void clear_v_stack (); // Clear the value stack
 void addim (void); // Add imaginary unit
 
 //scan data from string with mask
 int scanmasknum (const char *str);
 int strscan (char *str, const char *msk = nullptr, int n =0, double *v = nullptr, ...); 

 // Math for solving, integrating and differentiating
 
 // Solvers
 // Solve an equation given by the expression and return the solution as a complex value
 bool Solve (const char *expr, t_value ttag, t_symbol tag, float__t &re_res, float__t &im_res);
 float__t Diff (const char *expr); // Differentiate an equation given by the expression and return the
 float__t Extremum (const char *expr);
 bool For (const char *expr, value &res); // Operator 'for' (loop).

 float__t Integr (const char *expr, // Integrate an equation given by the expression and return the
                  t_symbol tag);    // result as a floating-point value

 float__t gkEval (calculator *pCalc, char *sexpr, // Evaluate a function for a given expression, variable name, and
                  const char *svar, float__t x); // variable value, and return the result as a floating-point value
                               
 GKResult gkPanel (calculator *pCalc, char *sexpr, const char *svar, float__t a, float__t b);
 GKResult gkAdaptive (calculator *pCalc, 
                     char *sexpr, 
                     const char *svar, 
                     float__t a,
                     float__t b, 
                     float__t tol, 
                     int depth, 
                     int maxDepth, 
                     int &callCount, 
                     int maxCalls);
 float__t Inverse (const char *expr);

 //Helpers
 bool Split (const char *expr, ...);
 t_br_result check_break (uint64_t init_ms, uint64_t last_gui_check); // Check for a break condition 
                                                                      //during long calculations
 void NormalizePath (const char *input, char *output, int outSize);

 bool isChildResReal (calculator *child);
 bool CheckChildRes (calculator *child);
 void GetChildRes (calculator *child, value &res);

 // Plotting functions
 bool PlotPrepare (const char *expr, v_func fidx, PlotParams &params);

 bool PlotCartesian (bmpdraw *bmp, PlotParams &params);
 void PlotDrawAxesCartesian (bmpdraw *bmp, PlotParams &params);

 bool PlotPolar (bmpdraw *bmp, PlotParams &params);
 void PlotDrawAxesPolar (bmpdraw *bmp, PlotParams &params);
 
 bool PlotParametric (bmpdraw *bmp, PlotParams &params);
 void PlotDrawAxesParametric (bmpdraw *bmp, PlotParams &params);
 
 bool PlotLogarithmic (bmpdraw *bmp, PlotParams &params);
 void PlotDrawAxesLog (bmpdraw *bmp, PlotParams &params);

 bool PlotSmith (bmpdraw *bmp, PlotParams &params);
 void PlotDrawAxesSmith (bmpdraw *bmp, PlotParams &params);

 bool PlotData (bmpdraw *bmp, PlotParams &params);

 bool Plot (const char *expr, v_func fidx, value &res); // Operator 'plot' for plotting data points or functions
 void PlotRegion (float__t x_min, float__t x_max, 
                  float__t y_min, float__t y_max); // Set the plotting region for the plot operator
 void PlotReset ();    // Reset plot settings to defaults
 bool AddBmp (bmpdraw *bmp1, bmpdraw *bmp2, uint32_t fg_color);


 // Matrix operations
 float__t *mxAlloc (int rows, int cols);
 float__t *dupMatrix (value &val);
 bool mxElemOp (value &res, value &left, value &right, int op);
 bool mxScalarOp (value &res, value &mx, float__t scalar, int op, bool scalar_left);
 bool mxMatMul (value &res, value &left, value &right);
 bool mxGaussJordan (float__t *aug, int n, float__t &det);
 float__t *mxMakeAug (value &M);

 bool mxElemFn (v_func fidx, value &res, value &M); //element-wise matrix function

 bool mxInv (value &res, value &M);
 bool mxNeg (value &res, value &M);
 bool mxTranspose (value &res, value &M);

 bool mxPolyRoots (value &res, value &coeffs);
 bool mxRegrFn (const char *fname, const char *msk, int n, rtype rt, value &res);
 float__t mxCalcFn (value M, rtype rt, float__t x);

 t_mresult matrixbin (value &res, value &left, value &right, t_operator cop);
 t_mresult matrixuno (value &res, value &left, t_operator cop);

 float__t mxTrace (value &M);
 float__t mxDet (value &M);
 float__t mxNorm (value &M);
 float__t mxDim (value &M, v_func dim);
 bool mxDot (value &res, value &A, value &B);
 bool mxCross (value &res, value &A, value &B);
 bool mxZeros (value &res, int rows, int cols);
 bool mxDiag (value &res, int rows, int cols);
 void mxerror (const char *msg);

// Statistical functions 
 float__t StatFn (const char *fname, const char *msk, sfntype sfn, float__t x=qnan);
 double Median (const char *fname, const char *msk, double totalN, double minV, double maxV);

 #ifdef SUPPORT_TABLEFN
 tablefn_data *tablefn (const char *fname, const char *msk, t_appr appr);
 float__t tablefn_eval (tablefn_data *tbl, float__t x);
 int find_tbl_fn (const char *name, const char **endp);
 int scan_tbl_fn (const char *expr, int fn_idx, tablefn_data **tbl);
 symbol *addTF (const char *name, const char *expr);
 symbol *addTFptr (const char *name, tablefn_data *tbl);
 symbol *cpyTFptr (const char *name, tablefn_data *tbl);
 bool dup_tbl_fn (tablefn_data **dst, tablefn_data *src);
 #endif

 // Script execution
 friend class script; // Declare script as a friend class to allow it to access private members of
                      // calculator

 bool Run (const char *expr, v_func fidx, value &res); // Run a script or expression and store the result in res
                                          // return the result in res
 int dataf (char *fname, char *sfmt, int args, value *v_stack);
 int datas (char *str, char *sfmt, int args, value *v_stack);

 // Variable and constant management
 bool Load (char *fname, value &res);
 bool Save (char *fname, value &val);

 inline char Ichar (void) { return c_imaginary; }; // Get the character used for the imaginary unit

 int mxprint (char *str, bool nl, // Print matrix result in a formatted way, with an option for a new line
              int *size = nullptr) // and an optional pointer to store the size of the output
 {
  return Mxprint (result_tag, res_rows, res_cols, res_mval, str, nl, size);
 }

 float__t evaluate_f (char *expr, // Evaluate an expression
                      __int64 *piVal = nullptr, float__t *pimval = nullptr); 

 inline int64_t get_int_res () { return result_ival; };
 inline float__t get_re_res () { return result_fval; };
 inline float__t get_im_res () { return result_imval; };
 inline t_value get_res_tag () { return result_tag; };
 inline uint32_t get_res_info () { return result_info; };
 inline char *get_str_res () { return result_tag == tvSTR ? sres : nullptr; };
 inline mxresult_t get_mx_res ()
 {
  mxresult_t res;
  res.rows = res_rows;
  res.cols = res_cols;
  res.mval = res_mval;
  return res;
 };

 void strval (char *str, value &val); // Convert a value to its string representation and store it
                                      // in the provided buffer
 public:
 calculator (int_t cfg = PAS + SCI + UPCASE, symbol **symtab = nullptr, int_t mask = (MASK_NONE),
             int deep = 0); // Constructor with optional syntax configuration

 void setEscFn (int (__cdecl *fn) (void)) { EscFn = fn; } // Set the escape function for long calculations
 void setFileDlgFn (bool (*fn) (char *, int)) { FileDlgFn = fn; } // Set the file dialog callback
 void setShowImageFn (fnShowImage fn) { ShowImageFn = fn; }
 void setDebugFn (debug_callback_t fn) { debugFn = fn; }

 void addfn (const char *name, void *func) { add (tsGUI, name, func);} // Add a function to the calculator

 double evaluate (char *expr) { return (double)evaluate_f (expr); } // Evaluate an expression

 inline void syntax (int_t cfg = PAS + SCI + UPCASE + FFLOAT) {scfg = cfg;}  // Set syntax configuration
 inline int_t issyntax (void) { return scfg; } // Get current syntax configuration

 inline int_t isfflags (void) { return fflags; } // Get current flags configuration
 inline void clrfflags (void) { fflags = 0; } // Clear flags configuration

 inline char *error (void) { return err; }   // Get error message
 inline int errps (void) { return errpos; }; // Get error position
 inline terr errt (void) { return errtype; }; // Get the error type

 inline void set_fprec (int prec) { fprec = prec; } // Set floating-point precision for output formatting
 inline int get_fprec () { return fprec; } // Get current floating-point precision for output formatting

 int print (char *str, int_t Options,
            int binwide,          // Print a string representation of the result with specified
            int *size = nullptr); // options and binary width,
 int printres (char *str, int_t options = FFLOAT, int binwide = 64);

 int varlist (char *buf, int bsize, // Get a list of variables in the calculator and store it in the provided
              int *maxlen = nullptr); // buffer, with an optional maximum length for variable names 

 bool Eval (char *expr, char *sres);
 int_t ScriptService (int_t x, v_func fidx);


 inline bool block () { return (scfg & NBLK) ? false : blockflag; };  // Block GUI
 ~calculator (void); // Destructor to clean up resources
};

#ifdef __BORLANDC__
 extern bool IsNaN (const double fVal); // Function to check if a double-precision floating-point
                                        // value is NaN (Not a Number)
 extern bool IsNaNL (const long double ldVal); // Function to check if a long double-precision
                                              // floating-point
//  value is NaN (Not a Number)
// #define isnan(a) (a != a) // Macro to check if a value is NaN (Not a Number) by comparing it to
                             // itself
#define isnan(a) IsNaNL (a)
#define isinf(a) IsInfL (a)
#endif

#endif // scalcH

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

#ifndef _WIN32
#include <minwindef.h>
#endif

#include <cstdlib> // for free() function

// RW - set both by calc engine and application
// WO - set only from application
// RO - set only by calc engine
// UI - set and used only from application
#define PAS    (1 << 0) // (RW) Pascal assingment and comparison style (:= =) <-> (= ==)
#define SCI    (1 << 1) // (WO) Scientific numbers format (2k == 2000)
#define UPCASE (1 << 2) // (WO) Case insensetive variables
#define UTMP   (1 << 3) // (WO) Using $n for temp
#define FFLOAT (1 << 4) // (WO) Forced float

#define DEG  (1 << 5)  // (RO) Degrees format found
#define CPX  (1 << 6)  // (RO) Complex format found
#define ENG  (1 << 7)  // (RO) Engineering (6k8) format found
#define STR  (1 << 8)  // (RO) String format found
#define HEX  (1 << 9)  // (RO) Hex format found
#define OCT  (1 << 10) // (RO) Octal format found
#define fBIN (1 << 11) // (RO) Binary format found
#define FBIN (1 << 11) // (RO) Binary format found
#define DAT  (1 << 12) // (RO) Date time format found
#define CHR  (1 << 13) // (RO) Char format found
#define WCH  (1 << 14) // (RO) WChar format found
#define ESC  (1 << 15) // (RO) Escape format found
#define CMP  (1 << 16) // (RO) Computing format found

#define NRM  (1 << 17) // (UI) Normalized output
#define IGR  (1 << 18) // (UI) Integer output
#define UNS  (1 << 19) // (UI) Unsigned output
#define ALL  (1 << 20) // (UI) All outputs
#define MIN  (1 << 21) // (UI) Esc minimized feature
#define MNU  (1 << 22) // (UI) Show/hide menu feature
#define UTM  (1 << 23) // (UI) Unix time
#define FRC  (1 << 24) // (UI) Fraction output
#define FRI  (1 << 25) // (UI) Fraction inch output
#define FRH  (1 << 26) // (UI) Farenheit input/output
#define TOP  (1 << 27) // (UI) Always on top
#define IMUL (1 << 28) // (WO) Implicit multiplication
#define OPT  (1 << 29) // (UI) Print options


#define STRBUF 256 // bufer size for string operations

#define _WCHAR_ // L'c' and 'c'W input format allow

#ifdef __BORLANDC__

#define nullptr NULL
#define _long_double_
typedef __int64 int_t;
typedef __int64 int64_t;
typedef unsigned __int64 unsigned_t;
#else //__BORLANDC__
#define _long_double_
#include <cstdint>
typedef int64_t int_t;
typedef uint64_t unsigned_t;
#endif //__BORLANDC__

#ifdef _long_double_
typedef long double float__t;
#else
typedef double float__t;
#endif
class value;
class symbol;

#pragma pack(push, 1)
typedef union
{
 int options;
 struct
 {
  int pas : 1;
  int sci : 1;
  int upcase : 1;
  int utmp : 1;
  int ffloat : 1;
  int deg : 1;
  int cpx : 1;
  int eng : 1;
  int str : 1;
  int hex : 1;
  int oct : 1;
  int fbin : 1;
  int dat : 1;
  int chr : 1;
  int wch : 1;
  int esc : 1;
  int cmp : 1;
  int nrm : 1;
  int igr : 1;
  int uns : 1;
  int all : 1;
  int min : 1;
  int mnu : 1;
  int utm : 1;
  int frc : 1;
  int fri : 1;
  int frh : 1;
  int top : 1;
  int imul : 1;
 };
} toptions;
#pragma pack(pop)

enum t_value
{
 tvINT,
 tvFLOAT,
 tvPERCENT,
 tvCOMPLEX,
 tvSTR
};

enum t_operator
{
 toBEGIN,
 toOPERAND,
 toERROR,
 toEND,
 toLPAR,
 toRPAR,
 toFUNC,
 toPOSTINC,
 toPOSTDEC,
 toFACT,
 toPREINC,
 toPREDEC,
 toPLUS,
 toMINUS,
 toNOT,
 toCOM,
 toPOW,
 toPERCENT,
 toMUL,
 toDIV,
 toMOD,
 toPAR,
 toADD,
 toSUB,
 toASL,
 toASR,
 toLSR,
 toGT,
 toGE,
 toLT,
 toLE,
 toEQ,
 toNE,
 toAND,
 toXOR,
 toOR,
 toSET,
 toSETADD,
 toSETSUB,
 toSETMUL,
 toSETDIV,
 toSETMOD,
 toSETASL,
 toSETASR,
 toSETLSR,
 toSETAND,
 toSETXOR,
 toSETOR,
 toSETPOW,
 toSEMI,
 toCOMMA,
 toTERMINALS
};

#define BINARY(opd) (opd >= toPOW)
#define UNARY(opd)  ((opd >= toPOSTINC) && (opd <= toCOM))

enum t_symbol
{
 tsVARIABLE,
 tsCONSTANT,
 tsIFUNCF1, // int f(float x)
 tsSFUNCF1, // char* f(float x)
 tsIFUNC1,  // int f(int x)
 tsIFUNC2,  // int f(int x, int y)
 tsFFUNC1,  // float f(float x)
 tsCFUNCC1, // complex f(complex x)
 tsFFUNCC1, // float f(complex x)
 tsFFUNC2,  // float f(float x, float y)
 tsFFUNC3,  // float f(float x, float y, float z)
 tsIFFUNC3, // int f(float x, float y, int z)
 tsPFUNCn,  // int printf(char *format, ...)
 tsSFUNCF2,  // float const(char *name, float value)
 tsSIFUNC1, // int f(char *s)
 tsVFUNC1,  // void vfunc(value* res, value* arg, int idx)
 tsVFUNC2,  // void vfunc(value* res, value* arg1, value* arg2, int idx)
 tsNUM
};

enum v_func
{
 vf_abs,
 vf_pol,

 vf_sin,
 vf_cos,
 vf_tan,
 vf_cot,

 vf_sinh,
 vf_cosh,
 vf_tanh,
 vf_ctnh,

 vf_asin,
 vf_acos,
 vf_atan,
 vf_acot,

 vf_asinh,
 vf_acosh,
 vf_atanh,
 vf_acoth,

 vf_exp,
 vf_log,
 vf_sqrt,

 vf_pow,
 vf_rootn,
 vf_logn,

 vf_re,
 vf_im,
 vf_cplx,

 vf_num
};

class value
{
 public:
 t_value tag;
 symbol *var;
 int pos;
 struct
 {
  int_t ival;
  float__t fval;
  float__t imval;
 };
 char *sval;

 inline value ()
 {
  tag   = tvINT;
  var   = nullptr;
  ival  = 0;
  fval  = 0.0;
  imval = 0.0;
  pos   = 0;
  sval  = nullptr;
 }

 inline ~value ()
 {
  if (sval) free (sval);
  sval = nullptr;
 }

 inline float__t get () { return tag == tvINT ? (float__t)ival : fval; }

 inline float__t get_dbl () { return tag == tvINT ? (double)ival : (double)fval; }

 inline int_t get_int () { return tag == tvINT ? ival : (int_t)fval; }

 inline char *get_str () { return tag == tvSTR ? sval : nullptr; }
};

class symbol
{
 public:
 t_symbol tag;
 v_func fidx;
 void *func;
 value val;
 char *name;
 symbol *next;

 inline symbol ()
 {
  tag  = tsVARIABLE;
  fidx = vf_num;
  func = nullptr;
  name = nullptr;
  next = nullptr;
 }
};

const int max_stack_size        = 256;
const int max_expression_length = 1024;

const int hash_table_size = 1013;

typedef void (*complex_func_t) (long double re, long double im, long double &out_re,
                                long double &out_im);

class calculator
{
 private:
 int scfg;
 value v_stack[max_stack_size];
 symbol *hash_table[hash_table_size];
 t_operator o_stack[max_stack_size];
 int v_sp;
 int o_sp;
 char *buf;
 int pos;
 int tmp_var_count;
 char err[80];
 int errpos;
 char c_imaginary;
 bool expr;
 char sres[STRBUF];
 int64_t result_ival;
 float__t result_fval;
 float__t result_imval;
 t_value result_tag;

 inline unsigned string_hash_function (const char *p);
 symbol *add (t_symbol tag, const char *name, void *func = nullptr);
 symbol *add (t_symbol tag, v_func fidx, const char *name, void *func = nullptr);
 symbol *find (const char *name);
 t_operator scan (bool operand, bool percent);
 void error (int pos, const char *msg);
 inline void error (const char *msg) { error (pos - 1, msg); }
 bool assign ();

 bool isCMP (char *&fpos);
 int hscanf (char *str, int_t &ival, int &nn);
 int bscanf (char *str, int_t &ival, int &nn);
 int oscanf (char *str, int_t &ival, int &nn);
 int xscanf (char *str, int len, int_t &ival, int &nn);
 float__t dstrtod (char *s, char **endptr);
 float__t tstrtod (char *s, char **endptr);
 void engineering (float__t mul, char *&fpos, float__t &fval);
 void scientific (char *&fpos, float__t &fval);
 void clear_v_stack ();
 void addim (void);
 
 public:
 calculator (int cfg = PAS + SCI + UPCASE);
 inline void syntax (int cfg = PAS + SCI + UPCASE + FFLOAT) { scfg = cfg; }
 inline int issyntax (void) { return scfg; }
 inline char *error (void) { return err; }
 inline int errps (void) { return errpos; };
 inline char *Sres (void) { return sres; };
 inline char Ichar (void) { return c_imaginary; };

 float__t AddConst (const char *name, float__t val);
 float__t AddVar (const char *name, float__t val);
 void addfconst (const char *name, float__t val);
 void addfvar (const char *name, float__t val);
 void addivar (const char *name, int_t val);
 void addlvar (const char *name, float__t fval, int_t ival);
 void addfn (const char *name, void *func) { add (tsFFUNC1, name, func); }
 void addfn2 (const char *name, void *func) { add (tsFFUNC2, name, func); }
 int varlist (char *buf, int bsize, int *maxlen = nullptr);
 float__t evaluate (char *expr, __int64 *piVal = nullptr, float__t *pimval = nullptr);

 int print (char *str, int Options, int binwide, int *size = nullptr);

 ~calculator (void);
};

extern bool IsNaN (const double fVal);
extern bool IsNaNL (const long double ldVal);
#define isnan(a) (a != a)

#endif

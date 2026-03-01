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
#define OPT  (1 << 29) // (UI) Print options (CLI only)

#define STRBUF 256 // bufer size for string operations
#define MAXOP  64  // maximum length of operator or function name
#define MAXSTK 10  // maximum stack depth

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

#ifdef __BORLANDC__
const float__t qnan = 0.0 / 0.0;
#else
constexpr float__t qnan = std::numeric_limits<float__t>::quiet_NaN ();
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
  int opt : 1;
 };
} toptions;
#pragma pack(pop)

enum t_value // t_value represents the type of a value in the calculator
{
 tvERR,
 tvINT,
 tvFLOAT,
 tvPERCENT,
 tvCOMPLEX,
 tvSTR,
 tvUFUNCT,
 tvSOLVE,
 tvINTEGR,
 tvDIFF
};

enum t_operator // t_operator represents the type of an operator in the calculator
{
 toBEGIN,   // 0  toBEGIN must be the first operator in the list
 toOPERAND, // 1  toOPERAND represents an operand in the expression
 toERROR,   // 2  toERROR represents an error in the expression
 toEND,     // 3  toEND represents the end of the expression
 toLPAR,    // 4  toLPAR represents a left parenthesis '('
 toRPAR,    // 5  toRPAR represents a right parenthesis ')'
 toFUNC,    // 6  toFUNC represents a function
 toSOLVE,   // 7  toSOLVE represents a solve, integr and diff function
 toPOSTINC, // 8  toPOSTINC represents a post-increment (v++) operator
 toPOSTDEC, // 9  toPOSTDEC represents a post-decrement (v--) operator
 toFACT,    // 10 toFACT represents a factorial 'n!' operator
 toPREINC,  // 11 toPREINC represents a pre-increment (++v) operator
 toPREDEC,  // 12 toPREDEC represents a pre-decrement (--v) operator
 toPLUS,    // 13 toPLUS represents an '+v' operator
 toMINUS,   // 14 toMINUS represents a '-v' operator
 toNOT,     // 15 toNOT represents a logical NOT operator
 toCOM,     // 16 toCOM represents a bitwise complement '~' operator
 toPOW,     // 17 toPOW represents a power '^' operator
 toPERCENT, // 18 toPERCENT represents a percentage '%' operator
 toMUL,     // 19 toMUL represents a multiplication operator
 toDIV,     // 20 toDIV represents a division operator
 toMOD,     // 21 toMOD represents a modulo operator
 toPAR,     // 22 toPAR represents a parenthesis operator
 toADD,     // 23 toADD represents an addition operator
 toSUB,     // 24 toSUB represents a subtraction operator
 toASL,     // 25 toASL represents a arithmetic shift left operator
 toASR,     // 26 toASR represents a arithmetic shift right operator
 toLSR,     // 27 toLSR represents a logical shift right operator
 toGT,      // 28 toGT represents a greater than operator
 toGE,      // 29 toGE represents a greater than or equal operator
 toLT,      // 30 toLT represents a less than operator
 toLE,      // 31 toLE represents a less than or equal operator
 toEQ,      // 32 toEQ represents an equality operator
 toNE,      // 33 toNE represents a not equal operator
 toAND,     // 34 toAND represents a logical AND operator
 toXOR,     // 35 toXOR represents a logical XOR operator
 toOR,      // 36 toOR represents a logical OR operator
 toSET,     // 37 toSET represents a set operator
 toSETADD,  // 38 toSETADD represents a set addition operator
 toSETSUB,  // 39 toSETSUB represents a set subtraction operator
 toSETMUL,  // 40 toSETMUL represents a set multiplication operator
 toSETDIV,  // 41 toSETDIV represents a set division operator
 toSETMOD,  // 42 toSETMOD represents a set modulo operator
 toSETASL,  // 43 toSETASL represents a set arithmetic shift left operator
 toSETASR,  // 44 toSETASR represents a set arithmetic shift right operator
 toSETLSR,  // 45 toSETLSR represents a set logical shift right operator
 toSETAND,  // 46 toSETAND represents a set logical AND operator
 toSETXOR,  // 47 toSETXOR represents a set logical XOR operator
 toSETOR,   // 48 toSETOR represents a set logical OR operator
 toSETPOW,  // 49 toSETPOW represents a set power operator
 toSEMI,    // 50 toSEMI represents a semicolon operator
 toCOMMA,   // 51 toCOMMA represents a comma operator
 toCONTINUE, // 52 toCONTINUE represents a continue operator for continue scanning
 toTERMINALS // 53 toTERMINALS must be the last operator in the list and represents the total number of
             // operators
};

enum t_symbol // t_symbol represents the type of a symbol in the calculator
{
 tsVARIABLE, // tsVARIABLE represents a variable symbol
 tsCONSTANT, // tsCONSTANT represents a constant symbol
 tsIFUNCF1,  // int f(float x)
 tsSFUNCF1,  // char* f(float x)
 tsIFUNC1,   // int f(int x)
 tsIFUNC2,   // int f(int x, int y)
 tsFFUNC1,   // float f(float x)
 tsFFUNC2,   // float f(float x, float y)
 tsFFUNC3,   // float f(float x, float y, float z)
 tsPFUNCn,   // int printf(char *format, ...)
 tsSFUNCF2,  // float const(char *name, float value)
 tsSIFUNC1,  // int f(char *s)
 tsVFUNC1,   // void vfunc(value* res, value* arg, int idx)
 tsVFUNC2,   // void vfunc(value* res, value* arg1, value* arg2, int idx)
 tsUFUNCT,   // User-defined function
 tsSOLVE,    // Solve operator for solving equations
 tsINTEGR,   // Integration operator for numerical integration
 tsDIFF,     // Differentiation operator for numerical differentiation
 tsNUM
};

#define MASK_ALL 0xffff
#define MASK_NONE 0x0000
#define MASK_VARIABLE (1<< tsVARIABLE) // tsVARIABLE represents a variable symbol
#define MASK_CONSTANT (1<< tsCONSTANT) // tsCONSTANT represents a constant symbol
#define MASK_IFUNCF1 (1<< tsIFUNCF1) // tsIFUNCF1 represents an int function with one float argument
#define MASK_SFUNCF1 (1<< tsSFUNCF1) // tsSFUNCF1 represents a char* function with one float argument
#define MASK_IFUNC1 (1<< tsIFUNC1) // tsIFUNC1 represents an int function with one int argument
#define MASK_IFUNC2 (1<< tsIFUNC2) // tsIFUNC2 represents an int function with two int arguments
#define MASK_FFUNC1 (1<< tsFFUNC1) // tsFFUNC1 represents a float function with one float argument
#define MASK_FFUNC2 (1<< tsFFUNC2) // tsFFUNC2 represents a float function with two float arguments
#define MASK_FFUNC3 (1<< tsFFUNC3) // tsFFUNC3 represents a float function with three float arguments
#define MASK_PFUNCn (1<< tsPFUNCn) // tsPFUNCn represents a printf function with a variable number of arguments
#define MASK_SFUNCF2 (1<< tsSFUNCF2) // tsSFUNCF2 represents a float function with two float arguments
#define MASK_SIFUNC1 (1<< tsSIFUNC1) // tsSIFUNC1 represents an int function with one char* argument
#define MASK_VFUNC1 (1<< tsVFUNC1) // tsVFUNC1 represents a void function with one value argument and one int argument
#define MASK_VFUNC2 (1<< tsVFUNC2) // tsVFUNC2 represents a void function with two value arguments and one int argument
#define MASK_UFUNCT (1<< tsUFUNCT) // tsUFUNCT represents a user-defined function
#define MASK_SOLVE  (1 << tsSOLVE)  // tsSOLVE represents a solve operator for solving equations
#define MASK_INTEGR (1 << tsINTEGR) // tsINTEGR represents an integration operator for numerical integration
#define MASK_DIFF   (1 << tsDIFF) // tsDIFF represents a differentiation operator for numerical differentiation
#define MASK_DEFAULT (MASK_CONSTANT | MASK_IFUNCF1 | MASK_SFUNCF1 | MASK_IFUNC1 \
                    | MASK_IFUNC2 | MASK_FFUNC1  | MASK_FFUNC2 | MASK_FFUNC3  \
                    | MASK_PFUNCn | MASK_SFUNCF2 | MASK_SIFUNC1 | MASK_VFUNC1 \
                    | MASK_VFUNC2 | MASK_UFUNCT)

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

 vf_exp, // Exponential function
 vf_log, // Natural logarithm function
 vf_sqrt, // Square root function

 vf_pow, // Power function
 vf_rootn, // N-th root function
 vf_logn,  // Logarithm with specified base function

 vf_re, // Real part of a complex number function
 vf_im, // Imaginary part of a complex number function
 vf_cplx, // Complex number construction function
 vf_polar, // Polar coordinates construction function

 vf_num
};

class value // value represents a value in the calculator, which can be an integer, float, complex
            // number, string, or user-defined function
{
 public:
 t_value tag; // Type of value
 symbol *var; // Uses for variables and functions
 int pos;     // Position in expression for error reporting
 
 int_t ival; // Integer value
 float__t fval;  // Float value or real part of complex value
 float__t imval; // Imaginary part of complex value
 char *sval;     // String value

 inline value ()
 {
  tag   = tvINT;
  var   = nullptr; // Uses for variables and functions
  ival  = 0;
  fval  = 0.0;
  imval = 0.0;
  pos   = 0;
  sval  = nullptr;
 }

 inline float__t get () { return tag == tvINT ? (float__t)ival : fval; }
 inline float__t get_dbl () { return tag == tvINT ? (double)ival : (double)fval; }
 inline int_t get_int () { return tag == tvINT ? ival : (int_t)fval; }
 inline char *get_str () { return tag == tvSTR ? sval : nullptr; }
};

class symbol // symbol represents a symbol in the calculator, which can be a variable, constant, or
             // function
{
 public:
 t_symbol tag; // Type of symbol
 v_func fidx;  // Function index
 void *func;   // Function pointer
 value val;    // Value associated with the symbol
 char *name;   // Name of the symbol
 symbol *next; // Next symbol in the hash table chain

 inline symbol ()
 {
  tag  = tsVARIABLE;
  fidx = vf_num;
  func = nullptr;
  name = nullptr;
  next = nullptr;
 }
 
};

const int max_stack_size        = 256;  // Maximum size of value and operator stacks
const int max_expression_length = 1024; // Maximum length of expression
const int hash_table_size = 1013; // Size of hash table for variables and functions

struct StringNode // StringNode represents a node in a linked list of strings used for memory
                  // management of allocated strings
{
 char *str;        // String pointer to the allocated string
 StringNode *next; // Next node in the list
};

struct GKResult
{
 float__t value;
 float__t error;
 bool ok;
};


class calculator // calculator represents the main class for the expression calculator, which
                 // manages the state of the calculator, including variables, functions, stacks, and
                 // parsing logic
{
 private:
 int scfg; // Syntax configuration flags
 int deep; // Current stack depth
 value v_stack[max_stack_size]; // Value stack for operands
 symbol *hash_table[hash_table_size]; // Hash table for variables and functions
 t_operator o_stack[max_stack_size]; // Operator stack
 StringNode *string_list_head; // Head of the string list
 int v_sp; // Value stack pointer
 int o_sp; // Operator stack pointer
 char *buf; // Buffer for expression parsing
 int pos;   // Current position in the expression during parsing
 int tmp_var_count;
 char err[80]; // Error message buffer
 int errpos;   // Error position
 char c_imaginary; // Imaginary unit character
 bool expr;    // Expression flag
 char sres[STRBUF]; // String result buffer
 char lastvar[MAXOP];  // Last variable name used in the expression, if it is a string

 int64_t result_ival; // Integer result
 float__t result_fval; // Float result
 float__t result_imval; // Imaginary part of complex result
 t_value result_tag; // Type of result

 
 char *registerString (char *str); // Register a string in the string list and return the registered string pointer
 void clearAllStrings (); // Clear all strings in the string list
 char *dupString (const char *src); // Duplicate a string and register it in the string list
 void destroyvars (void); // Destroy all variables in the hash table
 void dupstrvars (void); // Duplicate string variables in the hash table (used when copying the calculator state)

 inline unsigned string_hash_function (const char *p); // Hash function for strings
 symbol *add (t_symbol tag, const char *name, void *func = nullptr); // Add a symbol to the hash table
 symbol *add (t_symbol tag, v_func fidx, const char *name,
              void *func = nullptr); // Add a symbol with function index to the hash table
 symbol *find (const char *name);    // Find a symbol in the hash table by name
 symbol *addUF (const char *name, const char *expr); // Add a user-defined function to the calculator
                                                    // with the given name and expression
 t_operator sscan (symbol *sym); // Scan body of the solve, integr and diff 

 t_operator scan (bool operand,
                  bool percent); // Scan the next token in the expression and return its operator type
 void error (int pos, const char *msg); // Report an error at the given position with the specified message
 void errorf (int pos, const char *fmt, ...); // Report an error at the given position with a formatted message
 inline void error (const char *msg) { error (pos - 1, msg); } // Report an error at the current position with 
                                                               //the specified message
 bool set_op (); // Assign a value to a variable

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
 float__t dstrtod (char *s, char **endptr); // Convert a string to a double-precision floating-point number
 float__t tstrtod (char *s, char **endptr); // Convert a string to a long double-precision floating-point number
 void engineering (float__t mul, char *&fpos, float__t &fval); // Perform engineering notation conversion
 void scientific (char *&fpos, float__t &fval); // Perform scientific notation conversion
 void clear_v_stack (); // Clear the value stack
 void addim (void); // Add imaginary unit
 
 float__t Solve (const char *expr); // Solve an equation given by the expression and return the
                                    // solution as a floating-point value

 float__t gkEval (calculator *pCalc, char *sexpr, const char *svar,
                  float__t x); // Evaluate a function for a given expression, variable name, and
                               // variable value, and return the result as a floating-point value
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

 float__t Integr (const char *expr); // Integrate an equation given by the expression and return the
                                    // result as a floating-point value
 float__t Diff (const char *expr); // Differentiate an equation given by the expression and return the
 public:
 calculator (int cfg = PAS + SCI + UPCASE,
             symbol **symtab = nullptr,
             int mask=MASK_DEFAULT,
             int deep = 0); // Constructor with optional syntax configuration
 inline void syntax (int cfg = PAS + SCI + UPCASE + FFLOAT)  { scfg = cfg; } // Set syntax configuration
 inline int issyntax (void) { return scfg; } // Get current syntax configuration
 inline char *error (void) { return err; }   // Get error message
 inline int errps (void) { return errpos; }; // Get error position
 inline char *Sres (void) { return sres; };  // Get string result
 inline char Ichar (void) { return c_imaginary;}; // Get the character used for the imaginary unit

 float__t AddConst (const char *name, float__t val); // Add a constant to the calculator and return its value
 float__t AddVar (const char *name, float__t val); // Add a variable to the calculator and return its value

 void addvar (const char *name, value &val); // Add a variable with a specified value to the calculator
 void addfconst (const char *name, float__t val); // Add a floating-point constant to the calculator
 void addfvar (const char *name, float__t val); // Add a floating-point variable to the calculator
 void addivar (const char *name, int_t val); // Add an integer variable to the calculator
 void addlvar (const char *name, float__t fval, int_t ival); // Add a long variable to the calculator
 void addfn (const char *name, void *func) { add (tsFFUNC1, name, func); } // Add a function to the calculator
 void addfn2 (const char *name, void *func) { add (tsFFUNC2, name, func); } // Add a function with two arguments to the calculator
 int varlist (char *buf, int bsize,
              int *maxlen = nullptr); // Get a list of variables in the calculator and store it in the provided
                          // buffer, with an optional maximum length for variable names
 float__t  evaluate (char *expr, __int64 *piVal = nullptr,
           float__t *pimval = nullptr); // Evaluate an expression and return the result as a floating-point value,
                       // with optional pointers to store integer and imaginary results
 inline char *get_last_var (void) { return lastvar; }; // Get the last variable name assigned in the 
                                                       //expression  
 int64_t get_int_res () { return result_ival; };
 float__t get_re_res () { return result_fval; };
 float__t get_im_res () { return result_imval; };
 t_value get_res_tag () { return result_tag; };
 char *get_str_res () { return result_tag == tvSTR ? sres : nullptr; };

 int print (char *str, int Options, int binwide,
            int *size = nullptr); // Print a string representation of the result with specified
                                  // options and binary width,

 ~calculator (void); // Destructor to clean up resources
};

extern bool IsNaN (const double fVal); // Function to check if a double-precision floating-point
                                       // value is NaN (Not a Number)
extern bool IsNaNL (const long double ldVal); // Function to check if a long double-precision floating-point
                                              // value is NaN (Not a Number)
#define isnan(a) (a != a) // Macro to check if a value is NaN (Not a Number) by comparing it to itself

#endif // scalcH

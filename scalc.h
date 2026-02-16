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
 toBEGIN,   // toBEGIN must be the first operator in the list
 toOPERAND, // toOPERAND represents an operand in the expression
 toERROR,   // toERROR represents an error in the expression
 toEND,     // toEND represents the end of the expression
 toLPAR,    // toLPAR represents a left parenthesis '('
 toRPAR,    // toRPAR represents a right parenthesis ')'
 toFUNC,    // toFUNC represents a function
 toPOSTINC, // toPOSTINC represents a post-increment operator
 toPOSTDEC, // toPOSTDEC represents a post-decrement operator
 toFACT,    // toFACT represents a factorial operator
 toPREINC,  // toPREINC represents a pre-increment operator
 toPREDEC,  // toPREDEC represents a pre-decrement operator
 toPLUS,    // toPLUS represents an addition operator
 toMINUS,   // toMINUS represents a subtraction operator
 toNOT,     // toNOT represents a logical NOT operator
 toCOM,     // toCOM represents a bitwise complement operator
 toPOW,     // toPOW represents a power operator
 toPERCENT, // toPERCENT represents a percentage operator
 toMUL,     // toMUL represents a multiplication operator
 toDIV,     // toDIV represents a division operator
 toMOD,     // toMOD represents a modulo operator
 toPAR,     // toPAR represents a parenthesis operator
 toADD,     // toADD represents an addition operator
 toSUB,     // toSUB represents a subtraction operator
 toASL,     // toASL represents a arithmetic shift left operator
 toASR,     // toASR represents a arithmetic shift right operator
 toLSR,     // toLSR represents a logical shift right operator
 toGT,      // toGT represents a greater than operator
 toGE,      // toGE represents a greater than or equal operator
 toLT,      // toLT represents a less than operator
 toLE,      // toLE represents a less than or equal operator
 toEQ,      // toEQ represents an equality operator
 toNE,      // toNE represents a not equal operator
 toAND,     // toAND represents a logical AND operator
 toXOR,     // toXOR represents a logical XOR operator
 toOR,      // toOR represents a logical OR operator
 toSET,     // toSET represents a set operator
 toSETADD,  // toSETADD represents a set addition operator
 toSETSUB,  // toSETSUB represents a set subtraction operator
 toSETMUL,  // toSETMUL represents a set multiplication operator
 toSETDIV,  // toSETDIV represents a set division operator
 toSETMOD,  // toSETMOD represents a set modulo operator
 toSETASL,  // toSETASL represents a set arithmetic shift left operator
 toSETASR,  // toSETASR represents a set arithmetic shift right operator
 toSETLSR,  // toSETLSR represents a set logical shift right operator
 toSETAND,  // toSETAND represents a set logical AND operator
 toSETXOR,  // toSETXOR represents a set logical XOR operator
 toSETOR,   // toSETOR represents a set logical OR operator
 toSETPOW,  // toSETPOW represents a set power operator
 toSEMI,    // toSEMI represents a semicolon operator
 toCOMMA,   // toCOMMA represents a comma operator
 toTERMINALS // toTERMINALS must be the last operator in the list and represents the total number of
             // operators
};

// Macros to determine if an operator is binary or unary based on its position in the enumeration
#define BINARY(opd) (opd >= toPOW)
#define UNARY(opd)  ((opd >= toPOSTINC) && (opd <= toCOM))

enum t_symbol
{
 tsVARIABLE, // tsVARIABLE represents a variable symbol
 tsCONSTANT, // tsCONSTANT represents a constant symbol
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

class symbol
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

// Function pointer type for complex functions (taking real and imaginary parts as input and output)
typedef void (*complex_func_t) (long double re, long double im, long double &out_re,
                                long double &out_im);

struct StringNode
{
 char *str;        // String pointer to the allocated string
 StringNode *next; // Next node in the list
};

class calculator
{
 private:
 int scfg; // Syntax configuration flags
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
 int64_t result_ival; // Integer result
 float__t result_fval; // Float result
 float__t result_imval; // Imaginary part of complex result
 t_value result_tag; // Type of result

 char *registerString (char *str); // Register a string in the string list and return the registered string pointer
 void clearAllStrings (); // Clear all strings in the string list
 char *dupString (const char *src); // Duplicate a string and register it in the string list

 inline unsigned string_hash_function (const char *p); // Hash function for strings
 symbol *add (t_symbol tag, const char *name, void *func = nullptr); // Add a symbol to the hash table
 symbol *add (t_symbol tag, v_func fidx, const char *name,
              void *func = nullptr); // Add a symbol with function index to the hash table
 symbol *find (const char *name);    // Find a symbol in the hash table by name
 t_operator scan (bool operand,
       bool percent); // Scan the next token in the expression and return its operator type
 void error (int pos, const char *msg); // Report an error at the given position with the specified message
 inline void error (const char *msg)
 {
  error (pos - 1, msg);
 } // Report an error at the current position with the specified message
 bool assign (); // Assign a value to a variable

 bool isCMP (char *&fpos); // Check if the current position is a computing format
 int hscanf (char *str, int_t &ival,
             int &nn); // Scan a hexadecimal number from the string and store it in ival, with nn
                       // being the number of characters processed
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
 
 public:
 calculator (int cfg = PAS + SCI + UPCASE); // Constructor with optional syntax configuration
   inline void syntax (int cfg = PAS + SCI + UPCASE + FFLOAT)
   {
    scfg = cfg;
   } // Set syntax configuration
   inline int issyntax (void) { return scfg; } // Get current syntax configuration
   inline char *error (void) { return err; }   // Get error message
   inline int errps (void) { return errpos; }; // Get error position
   inline char *Sres (void) { return sres; };  // Get string result
   inline char Ichar (void)
   {
    return c_imaginary;
   }; // Get the character used for the imaginary unit

 float__t AddConst (const char *name, float__t val); // Add a constant to the calculator and return its value
 float__t AddVar (const char *name, float__t val); // Add a variable to the calculator and return its value
 void addfconst (const char *name, float__t val); // Add a floating-point constant to the calculator
 void addfvar (const char *name, float__t val); // Add a floating-point variable to the calculator
 void addivar (const char *name, int_t val); // Add an integer variable to the calculator
 void addlvar (const char *name, float__t fval, int_t ival); // Add a long variable to the calculator
 void addfn (const char *name, void *func) { add (tsFFUNC1, name, func); } // Add a function to the calculator
 void addfn2 (const char *name, void *func) { add (tsFFUNC2, name, func); } // Add a function with two arguments to the calculator
 int varlist (char *buf, int bsize,
              int *maxlen = nullptr); // Get a list of variables in the calculator and store it in the provided
                          // buffer, with an optional maximum length for variable names
 float__t
 evaluate (char *expr, __int64 *piVal = nullptr,
           float__t *pimval = nullptr); // Evaluate an expression and return the result as a floating-point value,
                       // with optional pointers to store integer and imaginary results

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

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
#define MAXNAME 16  // maximum length of variable or function name

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

//#define _STATIC_MM_

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
 tvMATRIX,
 tvUFUNCT,
 tvSOLVE,
 tvINTEGR,
 tvDIFF
};

#define MAX_R 7
#define MAX_C 7

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
 tsFFUNCM,   // float f(matrix M)
 tsFFUNCM2,  // float f(matrix A, matrix B)
 tsMFUNCM,   // matrix f(matrix M)
 tsMFUNCM2,  // matrix f(matrix A, matrix B)
 tsVFUNC1,   // void vfunc(value* res, value* arg, int idx)
 tsVFUNC2,   // void vfunc(value* res, value* arg1, value* arg2, int idx)
 tsUFUNCT,   // User-defined function
 tsSOLVE,    // Solve operator for solving equations
 tsCALC,     // Calculate operator for evaluating expressions
 tsINTEGR,   // Integration operator for numerical integration
 tsSUM,      // Summation operator for numerical summation
 tsDIFF,     // Differentiation operator for numerical differentiation
 tsNUM
};

enum t_mresult
{
 mrDONE,    // represents a successful result
 mrSKIP,    // represents a result that indicates to skip the current operation or iteration
 mrERROR,   // represents an error result
};

enum t_mxDim
{
 mxRows,   
 mxCols,   
 mxSize,  
};


#define MASK_ALL 0xffffffff
#define MASK_NONE 0x00000000
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
#define MASK_FFUNCM (1<< tsFFUNCM) // float f(matrix M)
#define MASK_FFUNCM2 (1<< tsFFUNCM2)  // float f(matrix A, matrix B)
#define MASK_MFUNCM (1<< tsMFUNCM)     // matrix f(matrix M)
#define MASK_MFUNCM2 (1<< tsMFUNCM2) // matrix f(matrix A, matrix B)
#define MASK_VFUNC1 (1<< tsVFUNC1) // tsVFUNC1 represents a void function with one value argument and one int argument
#define MASK_VFUNC2 (1<< tsVFUNC2) // tsVFUNC2 represents a void function with two value arguments and one int argument
#define MASK_UFUNCT (1<< tsUFUNCT) // tsUFUNCT represents a user-defined function
#define MASK_SOLVE  (1 << tsSOLVE)  // tsSOLVE represents a solve operator for solving equations
#define MASK_INTEGR (1 << tsINTEGR) // tsINTEGR represents an integration operator for numerical integration
#define MASK_DIFF   (1 << tsDIFF) // tsDIFF represents a differentiation operator for numerical differentiation
#define MASK_DEFAULT (MASK_CONSTANT | MASK_IFUNCF1 | MASK_SFUNCF1 | MASK_IFUNC1 \
                    | MASK_IFUNC2 | MASK_FFUNC1  | MASK_FFUNC2 | MASK_FFUNC3  \
                    | MASK_PFUNCn | MASK_SFUNCF2 | MASK_SIFUNC1 | MASK_VFUNC1 \
                    | MASK_FFUNCM |MASK_FFUNCM2 | MASK_MFUNCM | MASK_MFUNCM2 \
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
 uint8_t mrows;  // Number of rows in matrix
 uint8_t mcols;  // Number of columns in matrix
 float__t *mval; // Matrix value (pointer to array of floats)

 inline value ()
 {
  tag   = tvERR; //tvINT;
  var   = nullptr; // Uses for variables and functions
  ival  = 0;
  fval  = 0.0;
  imval = 0.0;
  pos   = 0;
  sval  = nullptr;
  mrows = 0;
  mcols = 0;
  mval  = nullptr;
 }

 inline float__t get () { return tag == tvINT ? (float__t)ival : fval; }
 inline float__t get_dbl () { return tag == tvINT ? (double)ival : (double)fval; }
 inline int_t get_int () { return tag == tvINT ? ival : (int_t)fval; }
 inline char *get_str () { return tag == tvSTR ? sval : nullptr; }
 inline bool is_scalar () { return tag == tvINT || tag == tvFLOAT; }

//inline ~value ()
// {
//  if (sval) free (sval);
//  if (mval) free (mval);
//  sval = nullptr;
//  mval = nullptr;
// }
};

class symbol // symbol represents a symbol in the calculator, which can be a variable, constant, or
             // function
{
 public:
 t_symbol tag; // Type of symbol
 v_func fidx;  // Function index
 void *func;   // Function pointer
 value val;    // Value associated with the symbol
 char name[MAXNAME]; // Name of the symbol (fixed-size array to avoid dynamic memory allocation)
 symbol *next; // Next symbol in the hash table chain

 inline symbol ()
 {
  tag  = tsVARIABLE;
  fidx = vf_num;
  func = nullptr;
  name[0] = '\0'; // Initialize name to an empty string
  next = nullptr;
 }
 
};


class MemList
{
 void **list;
 int capacity;
 int count; // mem_idx аналог

 public:
 MemList (int initial = 256) : capacity (initial), count (0)
 {
  list = (void **)malloc (capacity * sizeof (void *));
  if (list) memset (list, 0, capacity * sizeof (void *));
 }

 ~MemList () { free (list); }

 void init_mem_list ()
 {
  if (list) memset (list, 0, capacity * sizeof (void *));
  count = 0;
 }

 int search_mem (void *mem)
 {
  for (int i = 0; i < count; i++)
   if (list[i] == mem) return i;
  return -1;
 }

void *register_mem (void *mem)
 {
  if (!mem) return nullptr;
  if (search_mem (mem) != -1) return mem; // already registered
  // fill holes first
  for (int i = 0; i < count; i++)
   if (!list[i])
    {
     list[i] = mem;
     return mem;
    }
  // no holes Ч append
  if (count < capacity)
   {
    list[count++] = mem;
    return mem;
   }
  // grow
  int newcap     = capacity * 2;
  void **newlist = (void **)realloc (list, newcap * sizeof (void *));
  if (!newlist) return nullptr;
  list = newlist;
  memset (list + capacity, 0, (newcap - capacity) * sizeof (void *));
  capacity      = newcap;
  list[count++] = mem;
  return mem;
 }

 void *unregister_mem (void *mem)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1) list[idx] = nullptr; // mark as unregistered, not freed
  return mem;
 }

 void *sf_alloc (int size)
 {
  if (!size) return nullptr;
  void *mem = malloc (size);
  if (mem) register_mem (mem);
  return mem;
 }

 void sf_free (void *dat)
 {
  if (dat)
   {
    unregister_mem (dat);
    free (dat);
   }
 }

 void free_all ()
 {
  for (int i = 0; i < count; i++)
   if (list[i])
    {
     free (list[i]);
     list[i] = nullptr;
    }
  count = 0;
 }

 int size () const { return count; }
};


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

 #ifdef _STATIC_MM_
 void *mem_list[max_stack_size]; // Memory for temporary strings used during expression parsing and
                            // evaluation
 int mem_idx;               // Index for the mem array  to manage temporary string memory
#else //_STATIC_MM_
 MemList mem_list; // Memory list for temporary strings used during expression parsing and evaluation
#endif //_STATIC_MM_

 int v_sp; // Value stack pointer
 int o_sp; // Operator stack pointer
 char *buf; // Buffer for expression parsing
 int pos;   // Current position in the expression during parsing
 int tmp_var_count;
 char err[80]; // Error message buffer
 char mxerr[80]; // Error message buffer for matrix operations
 int errpos;   // Error position
 char c_imaginary; // Imaginary unit character
 bool expr;    // Expression flag
 char sres[STRBUF]; // String result buffer
 char lastvar[MAXOP];  // Last variable name used in the expression, if it is a string

 uint8_t res_cols; // Number of columns in the matrix result
 uint8_t res_rows;   // Number of rows in the matrix result
 float__t *res_mval; // Matrix result (pointer to array of floats)

 int64_t result_ival; // Integer result
 float__t result_fval; // Float result
 float__t result_imval; // Imaginary part of complex result
 t_value result_tag; // Type of result

 void AddPredefined (void);

 void copy_symbols (symbol **symtab = nullptr, int mask = MASK_DEFAULT);

 //memory management
#ifdef _STATIC_MM_
 void init_mem_list (); // Initialize the mem array and mem_idx for memory management of temporary
                        // strings and matrix values
 int search_mem (void *mem); // Search for a pointer in the mem array and return its index, or -1 if not found
  void *register_mem (void *mem); // Register a pointer in the mem array and return the registered pointer
 void *unregister_mem (void *mem); // Unregister a pointer from the mem array by setting its entry to nullptr
  void clear_mem_list (void); // Clear all strings in the string list
 void *sf_alloc (int size); // Allocate memory for a temporary string and register it in the mem
                            // array for memory management
 void sf_free (void *dat);  // Free memory for a temporary string and unregister it from the mem
                            // array for memory management
#else //_STATIC_MM_
 void init_mem_list () { mem_list.init_mem_list (); }
 int search_mem (void *mem) { return mem_list.search_mem (mem); }
 void *register_mem (void *mem) { return mem_list.register_mem (mem); }
 void *unregister_mem (void *mem) { return mem_list.unregister_mem (mem); }
 void *sf_alloc (int size) { return mem_list.sf_alloc (size); }
 void sf_free (void *dat) { mem_list.sf_free (dat); }
 void clear_mem_list (void) { mem_list.free_all (); }
#endif //_STATIC_MM_

 void save_vars_mem (void); // Save the current variables in the hash table to the mem array for
                            // memory management
 char *dupString (const char *src); // Duplicate a string and register it in the string list
 void destroyvars (void); // Destroy all variables in the hash table

 inline unsigned string_hash_function (const char *p); // Hash function for strings
 symbol *add (t_symbol tag, const char *name, void *func = nullptr); // Add a symbol to the hash table
 symbol *add (t_symbol tag, v_func fidx, const char *name,
              void *func = nullptr); // Add a symbol with function index to the hash table
 symbol *find (const char *name);    // Find a symbol in the hash table by name
 symbol *addUF (const char *name, const char *expr); // Add a user-defined function to the calculator
                                                    // with the given name and expression
 t_operator sscan (symbol *sym); // Scan body of the solve, integr and diff 

 t_operator sqbraces (void); // Scan [..] matrix/vector constructor 

 
 t_operator braces (void);    // Scan the expression for parentheses and return the operator type of
                              // the next token after the parentheses
 t_operator dqscan (char qc); // Scan the double quote string in the expression and return its operator
                           // type, used for main scan
 t_operator dscan (bool operand,
                   bool percent); // Scan the digits in the expression and return its operator type,
                          // used for main scan
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
 
 float__t Solve (const char *expr, t_symbol tag); // Solve an equation given by the expression and
                                                  // return the solution as a floating-point value

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

 float__t Integr (const char *expr, // Integrate an equation given by the expression and return the
                  t_symbol tag);    // result as a floating-point value
                                    
 float__t Diff (const char *expr); // Differentiate an equation given by the expression and return the


 float__t *mxAlloc (int rows, int cols);
 float__t *dupMatrix (value &val);
 bool mxElemOp (value &res, value &left, value &right, int op);
 bool mxScalarOp (value &res, value &mx, float__t scalar, int op, bool scalar_left);
 bool mxMatMul (value &res, value &left, value &right);

 bool mxGaussJordan (float__t *aug, int n, float__t &det);
 float__t *mxMakeAug (value &M);
 bool mxInv (value &res, value &M);
 bool mxAbs (value &res, value &M);
 bool mxNeg (value &res, value &M);
 bool mxTranspose (value &res, value &M);

 t_mresult matrixbin (value &res, value &left, value &right, t_operator cop);
 t_mresult matrixuno (value &res, value &left, t_operator cop);
 void mxerror (const char *msg);
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

 float__t mxTrace (value &M);
 float__t mxDet (value &M);
 float__t mxNorm (value &M);
 float__t mxDim (value &M, t_mxDim dim);

 bool mxDot (value &res, value &A, value &B);
 bool mxCross (value &res, value &A, value &B);

 void addvar (const char *name, value &val); // Add a variable with a specified value to the calculator
 void addfconst (const char *name, float__t val); // Add a floating-point constant to the calculator
 void addfvar (const char *name, float__t val); // Add a floating-point variable to the calculator
 void addivar (const char *name, int_t val); // Add an integer variable to the calculator
 void addlvar (const char *name, float__t fval, int_t ival); // Add a long variable to the calculator
 void addfn (const char *name, void *func) { add (tsFFUNC1, name, func); } // Add a function to the calculator
 void addfn2 (const char *name, void *func) { add (tsFFUNC2, name, func); } // Add a function with two arguments to the calculator
 int varlist (char *buf, int bsize, // Get a list of variables in the calculator and store it in the provided
              int *maxlen = nullptr); // buffer, with an optional maximum length for variable names 
                          
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
 inline mxresult_t get_mx_res ()
 {
  mxresult_t res;
  res.rows = res_rows;
  res.cols = res_cols;
  res.mval = res_mval;
  return res;
 };

 int print (char *str, int Options, int binwide,
            int *size = nullptr); // Print a string representation of the result with specified
                                  // options and binary width,
 int mxprint (int8_t res_rows, 
              int8_t res_cols, 
              float__t *res_mval,
              char *str,
              bool nl, // Print matrix result in a formatted way, with an option for a new line
              int *size = nullptr); // and an optional pointer to store the size of the output 

 ~calculator (void); // Destructor to clean up resources
};

extern bool IsNaN (const double fVal); // Function to check if a double-precision floating-point
                                       // value is NaN (Not a Number)
extern bool IsNaNL (const long double ldVal); // Function to check if a long double-precision floating-point
                                              // value is NaN (Not a Number)
#define isnan(a) (a != a) // Macro to check if a value is NaN (Not a Number) by comparing it to itself

#endif // scalcH

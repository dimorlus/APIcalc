#pragma once
#ifndef scriptH
#define scriptH

#ifdef __BORLANDC__
#include <stdint.h>

#pragma warn - 8027
#define nullptr NULL
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <cstdint>
#endif

//// Debug callback function type
//typedef void (*debug_callback_t)(const char *fmt, ...);

#ifdef __BORLANDC__
enum opcodes
{
 opJMP = 1,
 opJZ,
 opJNZ,
 opCALL,
 opCALLZ,
 opCALLNZ,
 opRET,
 opNUM // Total number of opcodes
};

const char *const ops[opNUM] = {
 "",       // 0 - placeholder to align indices
 "JMP",    // 1
 "JZ",     // 2
 "JNZ",    // 3
 "CALL",   // 4
 "CALLZ",  // 5
 "CALLNZ", // 6
 "RET",    // 7
};
#endif

class script
{
 private:
  typedef struct
   {
    char label[8];
    uint16_t ptr;
   } tLblTable;

  enum tstate
   {
    stNl,
    stLbl,
    stChr,
   } state;

#ifndef __BORLANDC__
  enum opcodes
   {
    opJMP = 1,
    opJZ,
    opJNZ,
    opCALL,
    opCALLZ,
    opCALLNZ,
    opRET,
    opNUM // Total number of opcodes
   };

  const char *const ops[opNUM] =
   {
    "",       // 0 - placeholder to align indices
    "JMP",    // 1
    "JZ",     // 2
    "JNZ",    // 3
    "CALL",   // 4
    "CALLZ",  // 5
    "CALLNZ", // 6
    "RET",    // 7
   };
#endif

   char *buffer;
   int pass;
   int num_labels;
   int num_lines;
   tLblTable *LblTable;
   uint16_t *lineidx;
   char *label;
   char *plb;
   char serr[80]; // Error message buffer
   int errln;
   void *parent; // Pointer to parent calculator instance, set by calc engine
   calculator *child;
   debug_callback_t debug;
   int (*EscFn) (void);


   bool pass_buf ();
   bool compile ();
   uint16_t find_label (const char *lbl);
   bool is_zero (const value &v);
   t_br_result check_break (uint64_t init_ms, uint64_t last_gui_check);
 public:
   script(void *par = nullptr);
   ~script ();
   bool load(const char* filename);
   bool execute ();
   inline char *serror (void) { return serr; } // Get error message
   void set_calculator (calculator *calc) { child = calc; }
   void set_debug_callback (debug_callback_t callback) { debug = callback; }
   void setEscFn (int (__cdecl *fn) (void)) {EscFn = fn;} // Set the escape function for long calculations
};


#endif // scriptH


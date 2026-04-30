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

// Debug callback function type
typedef void (*debug_callback_t)(const char *fmt, ...);

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
   
   char *buffer;
   int pass;
   int num_labels;
   int num_lines;
   tLblTable *LblTable;
   uint16_t *lineidx;
   char *label;
   char *plb;

   calculator *child;
   debug_callback_t debug;

   bool pass_buf ();
   bool compile ();
   uint16_t find_label (const char *lbl);
   bool is_zero (const value &v);

 public:
	script();
   ~script ();
	bool load(const char* filename);
   bool execute ();
   void set_calculator (calculator *calc) { child = calc; }
   void set_debug_callback (debug_callback_t callback) { debug = callback; }
};


#endif // scriptH


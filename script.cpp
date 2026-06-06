#pragma region Include Headers and Define Constants
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#ifdef __BORLANDC__
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits>
#include <malloc.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#pragma warn - 8004 // assigned a value that is never used

#else //__BORLANDC__

#define __USE_MINGW_ANSI_STDIO 1
#include <cstdint>
#include <ctime>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits>
#include <malloc.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#endif //__BORLANDC__


#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"
#include "script.h"

#ifdef _float128_
#include <quadmath.h>
#endif

script::script()
{
 buffer = nullptr;
 int pass = 0;
 LblTable = nullptr;
 lineidx = nullptr;
 label  = nullptr;
 plb = label;
 state = stNl;
 num_labels = 0;
 num_lines = 0;
 child = nullptr;
 debug = nullptr;
 EscFn = nullptr;
 serr[0] = '\0';
 errln  = 0;
 pass = 0;

}

script::~script()
{
 if (buffer) free(buffer);
 if (LblTable) free(LblTable);
 if (lineidx) free(lineidx);
 if (label) free (label);
}

bool script::load (const char *filename)
{
 FILE *f = fopen (filename, "rb");
 if (!f) return false;
 fseek (f, 0, SEEK_END);
 long fsize = ftell (f);
 fseek (f, 0, SEEK_SET);
 if (fsize <= 0||fsize > 0x7FFF)
  {
   fclose (f);
   sprintf (serr, "File size error: %ld bytes", fsize);
   errln = 0;
   return false;
  }
 buffer = (char *)malloc (fsize + 1);
 if (!buffer)
  {
   fclose (f);
   sprintf (serr, "Out of memory");
  }
 fread (buffer, 1, fsize, f);
 buffer[fsize] = '\0';
 fclose (f);

 // Parse and compile
 if (!pass_buf ()) return false;

 if (!compile ()) return false;

 return true;
}


bool script::pass_buf ()
{
 for (pass = 0; pass < 2; pass++)
  {
   char *cp         = buffer;
   int lines        = 0;
   int labels       = 0;
   char *line_start = cp;

   while (*cp)
    {
     if (*cp == '\t') *cp = ' ';
     switch (state)
      {
      case stNl:
       line_start = cp;

       // Skip empty lines
       while (*cp == '\r' || *cp == '\n' || *cp == ' ')
        {
         if (pass == 1 && (*cp == '\r' || *cp == '\n'))
          {
           *cp = '\0'; // Replace only on second pass
          }
         cp++;
        }

       if (!*cp) break; // End of buffer

       // This is a non-empty line - record its position
       if (pass == 1) 
        lineidx[lines] = (uint16_t)(cp - buffer);

       if (pass == 1 && LblTable && labels < num_labels)
        plb = LblTable[labels].label;
       else
        plb = nullptr;

       if (*cp == ':')
        {
         cp++;
         state = stLbl;
        }
       else
        {
         state = stChr;
        }
       
       lines++; // Increment after recording position
       break;

      case stLbl:
       while (*cp == ':') cp++; // Skip multiple :::
       while (*cp == ' ') cp++; // Skip spaces after :

       if (pass == 1 && plb)
        {
         LblTable[labels].ptr = lines - 1;
         int i                = 0;
         while (i < 8 && (isalnum (*cp & 0x7f)||*cp == '_'))
          {
           plb[i++] = toupper (*cp++ & 0x7f);
          }
         while (i < 8) plb[i++] = '\0';
        }
       else
        {
         // First pass - just count
         while (isalnum (*cp & 0x7f)||*cp == '_') cp++;
        }

       labels++;

       // Skip rest of label
       while (*cp && *cp != '\r' && *cp != '\n' && *cp != ' ') cp++;
       while (*cp == ' ') cp++;

       if (*cp == '\r' || *cp == '\n' || *cp == '\0')
        state = stNl;
       else
        state = stChr;
       break;

      case stChr:
       if (*cp == '\r' || *cp == '\n')
        state = stNl;
       else
        cp++;
       break;
      }
    }

   if (pass == 0)
    {
     num_labels = labels;
     num_lines  = lines;

     if (labels > 0) LblTable = (tLblTable *)malloc (labels * sizeof (tLblTable));
     if (lines > 0) lineidx = (uint16_t *)malloc (lines * sizeof (uint16_t));

     if ((labels > 0 && !LblTable) || (lines > 0 && !lineidx))
      {
       sprintf (serr, "Out of memory");
       errln = 0;
       return false;
      }

     if (LblTable) memset (LblTable, 0, labels * sizeof (tLblTable));

     // Reset for second pass
     state = stNl;
     cp = buffer;
    }
  }
 
 // Reset state after parsing complete
 state = stNl;
 return true;
}

bool script::compile ()
{
 // Third pass - replace opcodes and labels with bytecode
 for (int i = 0; i < num_lines; i++)
  {
   char *line = buffer + lineidx[i];

   // Skip leading spaces
   while (*line == ' ') line++;

   // Skip empty lines and comments
   if (!*line || *line == ';' || *line == '\0') continue;

   // Check if this is a label definition - mark it as empty
   if (*line == ':')
    {
     // Replace label line with null to skip during execution
     *(buffer + lineidx[i]) = '\0';
     continue;
    }

   // Check each opcode (starting from 1, skipping placeholder at 0)
   for (int op = 1; op < opNUM; op++)
    {
     const char *opcode = ops[op];
     char *p = line;
     bool match = true;
     
     // Compare opcode character by character, case-insensitive
     while (*opcode)
      {
       if (toupper(*p & 0x7f) != *opcode)
        {
         match = false;
         break;
        }
       p++;
       opcode++;
      }
     
     // Check that opcode ends with delimiter (space, null, semicolon)
     if (match && (*p == ' ' || *p == '\0' || *p == ';'))
      {
       // Replace opcode with byte (1-7) - op is already 1-based
       *line = (char)op;

       // Find label argument (for all except RET)
       if (op != opRET)
        {
         char *arg = p;
         while (*arg == ' ') arg++;

         if (*arg && *arg != ';')
          {
           // Find label
           uint16_t target = find_label (arg);
           if (target == 0xFFFF)
            {
             sprintf (serr, "Undefined label: %s", arg);
             return false; // Label not found
            }

           // Replace with target line pointer (2 bytes)
           *(line + 1) = (char)(target & 0xFF);
           *(line + 2) = (char)((target >> 8) & 0xFF);

           // Null-terminate after opcode+arg
           *(line + 3) = '\0';
          }
         else
          {
           // Opcode without argument
           *(line + 1) = '\0';
          }
        }
       else
        {
         // RET - just opcode
         *(line + 1) = '\0';
        }

       break;
      }
    }
  }

 // Free label table - no longer needed
 if (LblTable)
  {
   free (LblTable);
   LblTable = nullptr;
  }

 return true;
}

bool script::is_zero (const value &v)
{
 switch (v.tag)
  {
  case tvINT:
   return v.ival == 0;
  case tvFLOAT:
   return v.fval == (float__t)0.0L;
  case tvCOMPLEX:
   return v.fval == (float__t)0.0L && v.imval == (float__t)0.0L;
  default:
   return false;
  }
}

t_br_result script::check_break (uint64_t init_ms, uint64_t last_gui_check)
{
#ifdef NDEBUG
 uint64_t current_ms = GetTickCount64 ();
 if (debug) return brNONE;
 if (current_ms - init_ms > TIMEOUT)
  {
   if (!EscFn)
    {
     sprintf(serr, "Operation took too long");
     return brTIMEOUT;
    }
   if (EscFn && EscFn ())
    {
     sprintf (serr, "Operation cancelled by user");
     return brESC;
    }
   else
    {
     if (current_ms - last_gui_check > 1000)
      {
       last_gui_check = current_ms;
       Sleep (1); // Sleep briefly to allow GUI to remain responsive
      }
     if (current_ms - init_ms > TIMEOUT) // 10 second time limit for summation
      {
       sprintf (serr, "Operation took too long");
       return brTIMEOUT;
      }
    }
  }
#endif // NDEBUG
 return brNONE;
}

bool script::execute ()
{
 if (!buffer || !lineidx)
  {
   if (debug) debug (child,"ERROR: No script loaded!\n");
   sprintf (serr, "No script loaded");
   return false;
  }

 int br      = drNONE;
 uint16_t ip = 0;     // instruction pointer
 uint16_t stack[256]; // return stack
 int sp = 0;          // stack pointer
 value last_result;

 last_result.tag  = tvERR;
 last_result.ival = 0;
 last_result.fval = 0.0;

 uint64_t init_ms = GetTickCount64 ();
 uint64_t last_gui_check = 0;


 if (!child) return false;

 if (debug) debug (child, ""); // Reset debug execution mode to step-by-step if it was set to auto before
 if (debug) br = debug (child, 
                "===                         Script started, %d lines.                              ===\n"
                "=== Press any key for the next step, F9 for restart, F8 for skip, Ctrl+C for exit  ===\n"
                "=== F5 for running without breaks, F7 - interactive mode, F10 for running silently ===\n", num_lines);

 while (ip < num_lines)
  {
   char *line = buffer + lineidx[ip];

   if (check_break (init_ms, last_gui_check) != brNONE) return false;
   if (br == drBREAK) break; //return false;
   else if (br == drRESTART)
    {
     ip = 0;
     sp = 0;
     last_result.tag  = tvERR;
     last_result.ival = 0;
     last_result.fval = 0.0;
     if (debug) br = debug (child, "=== Script restarted ===\n");
     continue;
    }

   // Skip leading spaces
   while (*line == ' ') line++;

   // Empty line or comment
   if (!*line || *line == ';' || *line == '\0')
    {
     if (debug) br = debug (child, "[%04d]\n", ip);
     ip++;
     continue;
    }

   // Check if this is a compiled opcode (byte < 32)
   if ((unsigned char)*line < 32)
    {
     unsigned char opcode = (unsigned char)*line;

     switch (opcode)
      {
      case opBREAK:
       if (debug) br = debug (child, "", ip, sp);
       if (debug) br = debug (child, "[%04d] BREAK\n", ip);
       ip++;
       break;
      case opRET:
       if (debug) br = debug(child, "[%04d] RET (sp=%d)\n", ip, sp);
       if (br != drSKIP)
        {
         if (sp == 0)
          {
           if (debug) br = debug (child, "=== Script finished (RET from main) ===\n");
           return true; // Exit script
          }
         ip = stack[--sp];
         if (debug) br = debug (child, "       Returned to line %d\n", ip + 1);
        }
       ip++;
       break;

      case opJMP:
       {
        uint16_t target = (unsigned char)line[1] | ((unsigned char)line[2] << 8);
        if (debug) br = debug(child, "[%04d] JMP %d\n", ip, target);
        if (br != drSKIP) ip = target;
        else ip++;
       }
       break;

      case opJZ:
       {
        uint16_t target = (unsigned char)line[1] | ((unsigned char)line[2] << 8);
        bool is_z = is_zero (last_result);
        if (debug) br = debug(child, "[%04d] JZ %d (condition=%s)\n", ip, target, is_z ? "true" : "false");
        if (br != drSKIP)
         {
          if (is_z)
           ip = target;
          else
           ip++;
         }
        else ip++;
       }
       break;

      case opJNZ:
       {
        uint16_t target = (unsigned char)line[1] | ((unsigned char)line[2] << 8);
        bool is_nz = !is_zero (last_result);
        if (debug) br = debug(child, "[%04d] JNZ %d (condition=%s)\n", ip, target, is_nz ? "true" : "false");
        if (br != drSKIP)
         {
          if (is_nz)
           ip = target;
          else
           ip++;
         }
        else
         ip++;
       }
       break;

      case opCALL:
       {
        uint16_t target = (unsigned char)line[1] | ((unsigned char)line[2] << 8);
        if (debug) br = debug(child, "[%04d] CALL %d (sp=%d)\n", ip, target, sp);
        if (br != drSKIP)
         {
          if (sp >= 256)
           {
            if (debug) br = debug (child, "ERROR: Stack overflow!\n");
            sprintf (serr, "Stack overflow");
            return false; // Stack overflow
           }
          stack[sp++] = ip;
          ip          = target;
         }
        else
         ip++;
       }
       break;

      case opCALLZ:
       {
        uint16_t target = (unsigned char)line[1] | ((unsigned char)line[2] << 8);
        bool is_z = is_zero (last_result);
        if (debug) debug(child, "[%04d] CALLZ %d (condition=%s, sp=%d)\n", ip, target, is_z ? "true" : "false", sp);
        if (br != drSKIP)
         {
          if (is_z)
           {
            if (sp >= 256)
             {
              if (debug) br = debug (child, "ERROR: Stack overflow!\n");
              sprintf (serr, "Stack overflow");
              return false;
             }
            stack[sp++] = ip;
            ip          = target;
           }
          else
           ip++;
         }
        else
         ip++;
       }
       break;

      case opCALLNZ:
       {
        uint16_t target = (unsigned char)line[1] | ((unsigned char)line[2] << 8);
        bool is_nz = !is_zero (last_result);
        if (debug) br = debug(child, "[%04d] CALLNZ %d (condition=%s, sp=%d)\n", ip, target, is_nz ? "true" : "false", sp);
        if (br != drSKIP)
         {
          if (is_nz)
           {
            if (sp >= 256)
             {
              if (debug) br = debug (child, "ERROR: Stack overflow!\n");
              sprintf (serr, "Stack overflow");
              return false;
             }
            stack[sp++] = ip;
            ip          = target;
           }
          else
           ip++;
         }
        else
         ip++;
       }
       break;

      default:
       if (debug) br = debug(child, "[%04d] ERROR: Unknown opcode %d\n", ip, opcode);
       return false;
      }
    }
   else
    {
     // Evaluate expression
     if (debug) br = debug(child, "[%04d] EVAL: %s\n", ip, line);
     if (br != drSKIP)
      {
       double result = child->evaluate (line);
       if (child->error ()[0] && child->errt () == teSyntax)
        {
         if (debug) br = debug (child, "Evaluation error: %s\n", child->error ());
         sprintf (serr, "%s", child->error ());
         return false;
        }
       last_result.tag   = child->get_res_tag ();
       last_result.ival  = child->get_int_res ();
       last_result.fval  = child->get_re_res ();
       last_result.imval = child->get_im_res ();

       if (debug)
        {
         switch (last_result.tag)
          {
          case tvINT:
           br = debug (child, "       Result: %lld (int)\n", last_result.ival);
           break;
          case tvFLOAT:
           br = debug (child, "       Result: %.15g (float)\n", (double)last_result.fval);
           break;
          case tvCOMPLEX:
           br = debug (child, "       Result: %.15g + %.15gi (complex)\n", (double)last_result.fval,
                       (double)last_result.imval);
           break;
          case tvSTR:
           br = debug (child, "       Result: \"%s\" (string)\n", child->get_str_res ());
           break;
          case tvMATRIX:
           br = debug (child, "       Result: %d x %d matrix\n", child->get_mx_res ().rows,
                       child->get_mx_res ().cols);
           break;
          case tvBMP:
           br = debug (child, "       Result: Bitmap\n");
           break;
          default:
           br = debug (child, "       Result: (other type %d)\n", last_result.tag);
           break;
          }
        }
      }
     ip++;
    }
  }

  if (br == drBREAK)
  {
   sprintf (serr, "User break");
   return false;
  }
 if (debug) printf ("=== Script finished (end of lines) ===\n");
 return true;
}

uint16_t script::find_label (const char *lbl)
{
 char label_upper[9];
 int i = 0;

 while (i < 8 && lbl[i] && lbl[i] != ' ' && lbl[i] != ';')
  {
   label_upper[i] = toupper (lbl[i] & 0x7f);
   i++;
  }
 label_upper[i] = '\0';

 for (int j = 0; j < num_labels; j++)
  {
   if (strncmp (LblTable[j].label, label_upper, 8) == 0) return LblTable[j].ptr;
  }

 return 0xFFFF; // Not found
}

bool calculator::Run (const char *expr, v_func fidx, value &res) // Run a script or expression and store the result in res
{                                 // return the result in res
 char filename[2048] = { 0 };
 char *buffer = nullptr;
 script *sct  = nullptr;
 res.tag = tvERR;
 res.ival = 0;
 res.fval = qnan;
 bool success = true;

 if (!expr || !*expr)
 {
     error(pos, "Empty script name");
     return false; // empty script name
 }

 calculator *child = new calculator (scfg | SNAN | NBLK, hash_table,
                                     (MASK_DEFAULT | MASK_VARIABLE | MASK_PLOT), deep);
 if (!child)
 {
     errorf(pos, "Out of memory");
     return false;
 }

 if (fidx == scRun)
 {
     sct = new script();
     if (!sct)
     {
         error(pos, "Out of memory");
         return false; // failed to create script object
     }

     NormalizePath(expr, filename, STRBUF);
     if (!sct->load(filename))
     {
       if (sct->serror()[0]) errorf (pos, "%s%", sct->serror ());
       else  errorf(pos, "Failed to load script: %s", filename);
       return false; // failed to load script
     }

     child->setEscFn(EscFn);
     sct->setEscFn(EscFn);
     sct->set_debug_callback (nullptr);
     if (scfg & DBG) console (1);
     if (debugFn) sct->set_debug_callback(debugFn);
     else 
     if ((scfg & DBG) && is_console_open ()) 
         sct->set_debug_callback (Debug); // Use calculator's debug function if available
     
     child->add (tsSCRIPT, scVars, "vars", nullptr, false);
     sct->set_calculator(child);

     success = sct->execute();
     if ((scfg & DBG) && is_console_open())
     {
       Debug (nullptr, ""); // Reset debug execution mode to step-by-step if it was set to auto before
       Debug (nullptr, "=== Console closed ===\n");
       console (0);
     }
 }
 else if (fidx == scEval)
 {
     strncpy(filename, expr, sizeof(filename) - 1);
     filename[sizeof(filename) - 1] = '\0';
     child->evaluate(filename);
     import_child (child, MSK_ALLVAR);
 }
 else
 {
     error(pos, "Invalid function index");
     delete child;
     return false; // invalid function index
 }

#ifdef _ENABLE_BMP_RES_
 strcpy (err, child->error ());
 {
  value val; 
  child->getvar ("answer", val); // Get 'answer' variable if set by script
  dupvar (res, val);
 }
 fflags |= child->isfflags ();
#else
 GetChildRes (child, res);
#endif
 delete child;
 if (fidx == scRun && sct) delete sct;
 return success;
}

void calculator::GetChildRes(calculator *child, value &res)
{
 strcpy (err, child->error ());

 //getvar ("answer", res); // Get 'answer' variable if set by script

 res.tag   = child->get_res_tag ();
 res.ival  = child->get_int_res ();
 res.fval  = child->get_re_res ();
 res.imval = child->get_im_res ();
 if (res.tag != tvBMP) res.sval  = dupString (child->get_str_res ());
 if (res.imval != (float__t)0.0L)
  res.tag = tvCOMPLEX; // Upgrade to complex if imaginary part is non-zero
 else if (res.tag == tvFLOAT && res.fval == (float__t)(int64_t)res.fval)
  res.tag = tvINT; // Downgrade to int if float is actually an integer
 // copy string and matrix result if applicable

 if (child->get_res_tag () == tvMATRIX)
  {
   mxresult_t mxr = child->get_mx_res ();
   res.tag        = tvMATRIX;
   res.mcols      = mxr.cols;
   res.mrows      = mxr.rows;
   int msize      = mxr.rows * mxr.cols * sizeof (float__t);
   if (msize)
    {
     float__t *new_mval = (float__t *)sf_alloc (msize, ptMALLOC);
     if (new_mval)
      {
       memcpy (new_mval, mxr.mval, msize);
       res.mval = new_mval;
      }
     else
      {
       res.tag  = tvERR;
       res.fval = qnan;
       errorf (res.pos, "Out of memory");
      }
    }
  }
 else if (child->get_res_tag () == tvSTR)
  {
   res.tag = tvSTR;
   fflags |= STR;
  }
  else if (child->get_res_tag () == tvBMP)
  {
   res.tag = tvERR;
   res.fval = qnan;
   errorf (res.pos, "Cannot return bitmap result");
  }
 fflags |= child->isfflags ();
}

bool calculator::Eval (char *expr, char *sres)
{
 value res;
 calculator *child = new calculator (scfg | SNAN, hash_table, (MASK_DEFAULT | MASK_VARIABLE), deep);
 if (!child)
  {
   sprintf (sres, "Out of memory");
   return false;
  }
 child->evaluate (expr);
 import_child (child, MSK_ALLVAR);
 if (child->error ()[0] && child->errt () == teSyntax)
  {
   sprintf (sres, "Evaluation error: %s", child->error ());
   delete child;
   return false;
  }
 getvar ("answer", res);
 strval (sres, res);
 delete child;  
 return true;
}

int_t calculator::ScriptService(int_t x, v_func fidx)
{
    switch (fidx)
    {
     case scVars:
      {
       char buf[4096];
       varlist (buf,sizeof(buf));
       printf (buf);
      }
     break;
    }
    return x;
}
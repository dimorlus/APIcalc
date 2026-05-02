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
#include "files.h"
#pragma endregion

// 1. Check if filename in the files array, if found use the existing file pointer, otherwise open
// the file and add it to the array.
// 2. Read line fromthe file into the str, ensuring not to exceed strsize.
// 3. Return the number of characters read, or -1 on error or -2 on end of file.
// 4. If the file is closed or cannot be opened, return -1.
// 5. If strsize is 0 or str is nullptr, reset the file pointer to the beginning of the file and
// return 0 (or goto 1 if filename not in the list).

 int filesystem::readstring (const char *filename, char *str, int strsize)
 {
  if (!filename || !*filename) return -1; // Invalid filename

  // Check if strsize is 0 or str is nullptr - close file
  if (strsize == 0 || str == nullptr)
   {
    // Find file in the array
    for (int i = 0; i < file_count; i++)
     {
      if (files[i].name && strcmp (files[i].name, filename) == 0)
       {
        // Close file if open
        if (files[i].fp)
         {
          fclose (files[i].fp);
          files[i].fp = nullptr;
         }
        return 0;
       }
     }
    // File not in list - nothing to close
    return 0;
   }

  // Normal read operation
  FILE *fp     = nullptr;
  int file_idx = -1;

  // Step 1: Check if filename in the files array
  for (int i = 0; i < file_count; i++)
   {
    if (files[i].name && strcmp (files[i].name, filename) == 0)
     {
      file_idx = i;
      fp       = files[i].fp;
      break;
     }
   }

  // If found but file is closed, reopen it
  if (file_idx >= 0 && fp == nullptr)
   {
    fp = fopen (filename, "r");
    if (!fp) return -1; // Cannot open
    files[file_idx].fp = fp;
   }

  // If not found in array, open the file and add to array
  if (file_idx < 0)
   {
    if (file_count >= 10) return -1; // Array full

    fp = fopen (filename, "r");
    if (!fp) return -1; // Cannot open

    files[file_count].name = (char *)malloc (strlen (filename) + 1);
    if (!files[file_count].name)
     {
      fclose (fp);
      return -1; // Out of memory
     }

    strcpy (files[file_count].name, filename);
    files[file_count].fp = fp;
    file_idx             = file_count;
    file_count++;
   }

  // Step 2: Read line from file
  if (!fgets (str, strsize, fp))
   {
    // Step 3: Check for EOF or error
    if (feof (fp))
     {
      // Close file on EOF
      fclose (fp);
      files[file_idx].fp = nullptr;
      return -2; // End of file
     }
    else
     return -1; // Error
   }

  // Step 3: Return number of characters read
  int len = (int)strlen (str);
  return len;
 }

 int calculator::dataf(char* fname, char* sfmt, int args, value* v_stack)
 {
  int nr;
  char str[STRBUF];
  double vals[10];
   
  if (sfmt && sfmt[0]) nr = fs.readstring(fname, str, STRBUF);
  else
   {
    nr = fs.readstring (fname, str, 0); // Reset file pointer if no format provided
    return 0;                           // No format, just reset file pointer
   }
  if (str[0] == '\0') return da_FileErr; // Error reading file
  if (nr == -1) return da_FileErr; // Error reading file
  int mn = scanmasknum (sfmt);
  if (mn != args) return da_ArgNum; // Format does not match number of arguments

  int ns = strscan (str, sfmt, args, &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], 
                                     &vals[5], &vals[6], &vals[7], &vals[8], &vals[9]);

  for (int n = 0; n < ns; n++)
   {
    if ((v_stack[n].tag == tvERR || v_stack[n].tag == tvFLOAT) &&
        (v_stack[n].var && 
         (v_stack[n].var->val.tag  == tvERR ||
         v_stack[n].var->val.tag  == tvFLOAT)))
     {
      v_stack[n].tag  = tvFLOAT;
      v_stack[n].fval = vals[n];
      v_stack[n].ival = (int_t)vals[n];
      if (v_stack[n].var)
       {
        v_stack[n].var->val.tag  = tvFLOAT;
        v_stack[n].var->val.fval = vals[n];
        v_stack[n].var->val.ival = (int_t)vals[n];
       }
     }
    else return da_ArgType - v_stack[n].pos; // Argument type mismatch
   }
 
  return ns;
 }

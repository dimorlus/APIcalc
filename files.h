#pragma once
#ifndef filesH
#define filesH

#ifdef __BORLANDC__
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>



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
#include <cstdio>
#include <cstring>
#endif

enum da_error
{
 da_ok = 0,
 da_FileErr = -1,
 da_ArgNum = -2,
 da_ArgType = -3,
};

class filesystem
{
 struct filelist
  {
   char *name; // File name
   FILE *fp;   // File pointer
  };

 private:
  filelist files[10]; // Array of open files for file management
  int file_count;     // Number of currently open files

 public:

 filesystem()
  {
   file_count = 0;
   memset (files, 0, sizeof (files));
  }

  ~filesystem ()
  {
   for (int i = 0; i < file_count; i++)
    {
     if (files[i].fp) fclose (files[i].fp);
     if (files[i].name) free (files[i].name);
    }
  }
  // 1. Check if filename in the files array, if found use the existing file pointer, otherwise open
  // the file and add it to the array. 
  // 2. Read line from the file into the str, ensuring not to exceed strsize.
  // 3. Return the number of characters read, or -1 on error or -2 on end of file.
  // 4. If the file is closed or cannot be opened, return -1.
  // 5. If strsize is 0 or str is nullptr, close the file if it's open and return 0.
  //    The file will be reopened on the next read request.
  int readstring (const char *filename, char *str, int strsize);
};


#endif // filesH


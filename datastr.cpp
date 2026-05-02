// #include "pch.h"
#ifdef __BORLANDC__
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"

#define M_PI_2l 1.5707963267948966192313216916398L
#define nullptr NULL
#pragma warn - 8066
#pragma warn - 8017
#pragma warn - 8008
#pragma warn - 8004

#define GetTickCount64 GetTickCount

#else
#define __USE_MINGW_ANSI_STDIO 1
#include <cstdint>
#include <ctime>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"
#endif

#ifdef __BORLANDC__
int get_timezone (void) // return seconds
{
 return -_timezone;
}
#else
// Get the timezone offset in seconds
int get_timezone (void) // return seconds
{
 TIME_ZONE_INFORMATION tzi;
 DWORD tzResult    = GetTimeZoneInformation (&tzi);
 long timezoneBias = tzi.Bias; // in minutes
 int tzHours       = timezoneBias * 60;
 return tzHours;
}
#endif


//ISO 8601 (2026(-|.|/)05-01( ,)12:00[:00[.12345]])
//  Convert a date-time string to a timestamp (seconds since epoch) with fractional seconds

double scandatatime (char *tstr, char** endp)
{
 if (!tstr)
  {
   if (endp) *endp = tstr;
   return qnan;
  }

 char *cp = tstr;
 
 // Skip leading spaces
 while (*cp == ' ' || *cp == '\t') cp++;
 
 char *start = cp;
 
 // Parse year (4 digits)
 if (!isdigit(*cp))
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 int year = 0;
 for (int i = 0; i < 4 && isdigit(*cp); i++)
  year = year * 10 + (*cp++ - '0');
 
 if (year < 1900 || year > 9999)
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 // Check for date separator
 if (*cp != '-' && *cp != '.' && *cp != '/')
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 char separator = *cp++;
 
 // Parse month (1-2 digits)
 if (!isdigit(*cp))
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 int month = 0;
 while (isdigit(*cp))
  month = month * 10 + (*cp++ - '0');
 
 if (month < 1 || month > 12)
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 // Check for same separator
 if (*cp != separator)
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 cp++;
 
 // Parse day (1-2 digits)
 if (!isdigit(*cp))
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 int day = 0;
 while (isdigit(*cp))
  day = day * 10 + (*cp++ - '0');
 
 if (day < 1 || day > 31)
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 // Date parsed successfully - initialize time to 00:00:00
 int hour = 0, min = 0, sec = 0;
 double frac = 0.0;
 
 // Check for time part (space or comma separator)
 if (*cp == ' ' || *cp == ',')
  {
   cp++;
   // Skip additional spaces
   while (*cp == ' ' || *cp == '\t') cp++;
   
   // Parse hour (1-2 digits)
   if (isdigit(*cp))
    {
     while (isdigit(*cp))
      hour = hour * 10 + (*cp++ - '0');
     
     if (hour < 0 || hour > 23)
      {
       if (endp) *endp = tstr;
       return qnan;
      }
     
     // Check for colon
     if (*cp == ':')
      {
       cp++;
       
       // Parse minute (1-2 digits)
       if (!isdigit(*cp))
        {
         if (endp) *endp = tstr;
         return qnan;
        }
       
       while (isdigit(*cp))
        min = min * 10 + (*cp++ - '0');
       
       if (min < 0 || min > 59)
        {
         if (endp) *endp = tstr;
         return qnan;
        }
       
       // Optional seconds
       if (*cp == ':')
        {
         cp++;
         
         // Parse second (1-2 digits)
         if (!isdigit(*cp))
          {
           if (endp) *endp = tstr;
           return qnan;
          }
         
         while (isdigit(*cp))
          sec = sec * 10 + (*cp++ - '0');
         
         if (sec < 0 || sec > 59)
          {
           if (endp) *endp = tstr;
           return qnan;
          }
         
         // Optional fractional seconds
         if (*cp == '.')
          {
           cp++;
           
           if (!isdigit(*cp))
            {
             if (endp) *endp = tstr;
             return qnan;
            }
           
           double divisor = 0.1;
           while (isdigit(*cp))
            {
             frac += (*cp++ - '0') * divisor;
             divisor *= 0.1;
            }
          }
        }
      }
     else if (*cp != '\0' && *cp != ' ' && *cp != '\t')
      {
       // Hour must be followed by colon, space, or end of string
       if (endp) *endp = tstr;
       return qnan;
      }
    }
  }
 
 // Validate date using mktime trick
 struct tm breakdown = { 0 };
 breakdown.tm_year   = year - 1900; /* years since 1900 */
 breakdown.tm_mon    = month - 1;   /* 0-11 */
 breakdown.tm_mday   = day;
 breakdown.tm_hour   = hour;
 breakdown.tm_min    = min;
 breakdown.tm_sec    = sec;
 breakdown.tm_isdst  = -1; /* let mktime determine DST */
 
 // Save original values for validation
 int orig_year  = breakdown.tm_year;
 int orig_month = breakdown.tm_mon;
 int orig_day   = breakdown.tm_mday;
 int orig_hour  = breakdown.tm_hour;
 int orig_min   = breakdown.tm_min;
 int orig_sec   = breakdown.tm_sec;
 
 time_t timestamp = mktime (&breakdown);
 
 // Check if mktime normalized the date (invalid date like 2026-02-30)
 if (breakdown.tm_year != orig_year || 
     breakdown.tm_mon != orig_month || 
     breakdown.tm_mday != orig_day ||
     breakdown.tm_hour != orig_hour ||
     breakdown.tm_min != orig_min ||
     breakdown.tm_sec != orig_sec)
  {
   if (endp) *endp = tstr;
   return qnan;
  }
 
 // Set endp to first character after parsed date-time
 if (endp) *endp = cp;
 
 double result = (double)(timestamp - get_timezone ()) + frac;
 
 return result;
}

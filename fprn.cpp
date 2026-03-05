// prn("%1.30LG",(x:=1.84467440737095536e19;(x-1)/x))
// Format a string with the given format and arguments, and copy it to the destination buffer
// Fixes:
//   - param[] and p are now reset for each format specifier
//   - tSci, tFract, tNrm, tDeg now use float__t instead of double
//   - added dest size limit (STRBUF) to prevent buffer overrun
//   - clarified sfmt-- comment at end of inner loop

int_t fprn (char *dest, char *sfmt, int args, value *v_stack)
{
 char pfmt[STRBUF];
 enum ftypes
 {
  tNone,
  tBin,
  tComp,
  tChar,
  tSpc,
  tSci,
  tNrm,
  tTime,
  tInt,
  tFloat,
  tFract,
  tDeg,
  tString,
  tPtr
 } fmt;
 char c, cc;
 char param[16];
 int p;
 int i;
 int n = 0;
 bool flag;
 char *dst     = dest;
 char *dst_end = dest + STRBUF - 1; // leave room for '\0'
 if (!sfmt) return 0;
 do
  {
   c    = '\0';
   i    = 0;
   flag = false;
   fmt  = tNone;
   // reset per-specifier state
   memset (param, 0, sizeof (param));
   p = 0;
   do
    {
     cc = c;
     c = pfmt[i++] = *sfmt++;
     pfmt[i]       = '\0';
     if (flag)
      {
       if (c == '%')
        {
         flag = false;
         continue;
        }
       else if ((c == 'f') || (c == 'e') || (c == 'E') || (c == 'g') || (c == 'G'))
        {
         fmt = tFloat;
         break;
        }
       else if ((c == 'd') || (c == 'i') || (c == 'u') || (c == 'x') || (c == 'X') || (c == 'o'))
        {
         fmt = tInt;
         break;
        }
       else if (c == 'c')
        {
         fmt = tChar;
         break;
        }
       else if (c == 'C')
        {
         fmt = tSpc;
         break;
        }
       else if (c == 'D')
        {
         fmt = tDeg;
         break;
        }
       else if (c == 'S')
        {
         fmt = tSci;
         break;
        }
       else if (c == 'F')
        {
         fmt = tFract;
         break;
        }
       else if (c == 'N')
        {
         fmt = tNrm;
         break;
        }
       else if (c == 't')
        {
         fmt = tTime;
         break;
        }
       else if (c == 'b')
        {
         fmt = tBin;
         break;
        }
       else if (c == 'B')
        {
         fmt = tComp;
         break;
        }
       else if (c == 's')
        {
         fmt = tString;
         break;
        }
       else if ((c == 'n') || (c == 'p'))
        {
         fmt = tPtr;
         break;
        }
       else
        {
         if (p < (int)sizeof (param) - 1) param[p++] = c;
         param[p] = '\0';
         continue;
        }
      }
     else if (c == '%')
      {
       flag = true;
       continue;
      }
    }
   while (c && (i < STRBUF - 2));

   if (!c) sfmt--; // sfmt stepped past '\0'; step back so outer while (*sfmt) terminates

   if (n < args)
    {
     switch (fmt)
      {
      case tNone:
       if (dst + i < dst_end)
        dst += sprintf (dst, pfmt);
       break;
      case tPtr:
       if (dst + i < dst_end)
        {
         strcpy (dst, pfmt);
         dst += strlen (pfmt);
        }
       break;
      case tComp:
       {
        float__t cd = v_stack[n].get ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += b2scistr (dst, cd);
       }
       break;
      case tBin:
       {
        __int64 bi = v_stack[n].get_int ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += b2str (dst, pfmt, bi);
       }
       break;
      case tChar:
       {
        if (cc == 'l')
         {
          int ii = v_stack[n].get_int ();
          if (dst < dst_end) dst += fmtc (dst, pfmt);
          if (dst < dst_end) dst += wchr2str (dst, ii);
         }
        else
         {
          char ci = (char)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, ci);
         }
       }
       break;
      case tSpc:
       {
        char ci = (char)v_stack[n].get_int ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += chr2str (dst, ci);
       }
       break;
      case tSci:
       {
        float__t dd = v_stack[n].get (); // use float__t, not double
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += d2scistr (dst, dd);
       }
       break;
      case tFract:
       {
        int      en = 0;
        float__t dd = v_stack[n].get (); // use float__t, not double
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (param[0]) en = atoi (param);
        if (dst < dst_end) dst += d2frcstr (dst, dd, en);
       }
       break;
      case tNrm:
       {
        float__t dd = v_stack[n].get (); // use float__t, not double
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += d2nrmstr (dst, dd);
       }
       break;
      case tTime:
       {
        __int64 bi = v_stack[n].get_int ();
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += t2str (dst, bi);
       }
       break;
      case tInt:
       {
        if (cc == 'l')
         {
          long li = (long)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, li);
         }
        else if (cc == 'L')
         {
          __int64 Li = v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, Li);
         }
        else if (cc == 'h')
         {
          short hi = (short)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, hi);
         }
        else
         {
          int ii = (int)v_stack[n].get_int ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, ii);
         }
       }
       break;
      case tFloat:
       {
        if (cc == 'L')
         {
          long double Ld = (long double)v_stack[n].get ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, Ld);
         }
        else
         {
          double dd = (double)v_stack[n].get ();
          if (dst < dst_end) dst += sprintf (dst, pfmt, dd);
         }
       }
       break;
      case tDeg:
       {
        float__t dd = v_stack[n].get (); // use float__t, not double
        if (dst < dst_end) dst += fmtc (dst, pfmt);
        if (dst < dst_end) dst += dgr2str (dst, dd);
       }
       break;
      case tString:
       if (dst < dst_end) dst += sprintf (dst, pfmt, v_stack[n].sval);
       break;
      }
     n++;
    }
   else
    {
     // no more arguments — output remaining literal text from format string
     if (dst < dst_end) dst += sprintf (dst, pfmt);
    }
  }
 while (*sfmt && (i < STRBUF) && (n <= args));

 *dst = '\0';
 return dst - dest;
}

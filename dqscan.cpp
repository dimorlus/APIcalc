// "...." or '....'
// Supported escape sequences:
//   \n  -> newline
//   \r  -> carriage return
//   \t  -> tab
//   \\  -> backslash
//   \"  -> double quote (alternative to "" inside ""-quoted strings)
//   \'  -> single quote (alternative to '' inside ''-quoted strings)
// Doubled quote character (same as opening quote) also represents a single quote:
//   "Hello! ""World"""  -> Hello! "World"
//   'It''s fine'        -> It's fine

t_operator calculator::dqscan (char qc)
{
 char *ipos;
 char  sbuf[STRBUF];
 int   sidx = 0;
 ipos = buf + pos;

 while (*ipos && (sidx < STRBUF - 1))
  {
   if (*ipos == '\\')
    {
     // escape sequence
     ipos++;
     if (!*ipos) break; // unexpected end of string
     switch (*ipos)
      {
      case 'n':  sbuf[sidx++] = '\n'; break;
      case 'r':  sbuf[sidx++] = '\r'; break;
      case 't':  sbuf[sidx++] = '\t'; break;
      case '\\': sbuf[sidx++] = '\\'; break;
      case '"':  sbuf[sidx++] = '"';  break;
      case '\'': sbuf[sidx++] = '\''; break;
      default:   // unknown escape - keep as-is (e.g. \x -> \x)
       if (sidx < STRBUF - 2) sbuf[sidx++] = '\\';
       sbuf[sidx++] = *ipos;
       break;
      }
     ipos++;
    }
   else if (*ipos == qc)
    {
     // check for doubled quote: "" or ''
     if (*(ipos + 1) == qc)
      {
       sbuf[sidx++] = qc; // one quote character in result
       ipos += 2;         // skip both
      }
     else
      break; // end of string literal
    }
   else
    {
     sbuf[sidx++] = *ipos++;
    }
  }

 sbuf[sidx] = '\0';

 if (*ipos == qc)
  {
   if (sbuf[0]) scfg |= STR;
   v_stack[v_sp].tag  = tvSTR;
   v_stack[v_sp].ival = 0;
   v_stack[v_sp].sval = (char *)malloc (sidx + 1);
   if (v_stack[v_sp].sval)
    {
     strcpy (v_stack[v_sp].sval, sbuf);
     registerString (v_stack[v_sp].sval);
    }
   pos = ipos - buf + 1;
   v_stack[v_sp].pos   = pos;
   v_stack[v_sp++].var = nullptr;
   return toOPERAND;
  }
 else
  {
   error ("unterminated string literal");
   return toERROR;
  }
}

bool script::execute ()
{
 if (!buffer || !lineidx)
  return false;
 
 uint16_t ip = 0;        // instruction pointer
 uint16_t stack[256];    // return stack
 int sp = 0;             // stack pointer
 value last_result;
 
 last_result.tag = tvINT;
 last_result.ival = 0;
 last_result.fval = 0.0;
 
 if (!child)
  return false;
 
 while (true)
  {
   char *line = buffer + lineidx[ip];
   
   // Skip leading spaces
   while (*line == ' ') line++;
   
   // Empty line or comment
   if (!*line || *line == ';')
    {
     ip++;
     continue;
    }
   
   // Check for operators
   if (strncmp (line, "RET", 3) == 0)
    {
     if (sp == 0)
      return true; // Exit script
     ip = stack[--sp];
     ip++;
     continue;
    }
   
   if (strncmp (line, "JMP", 3) == 0)
    {
     char *lbl = line + 3;
     while (*lbl == ' ') lbl++;
     uint16_t target = find_label (lbl);
     if (target == 0xFFFF)
      return false; // Label not found
     ip = target;
     continue;
    }
   
   if (strncmp (line, "JZ", 2) == 0)
    {
     if (is_zero (last_result))
      {
       char *lbl = line + 2;
       while (*lbl == ' ') lbl++;
       uint16_t target = find_label (lbl);
       if (target == 0xFFFF)
        return false;
       ip = target;
       continue;
      }
     ip++;
     continue;
    }
   
   if (strncmp (line, "JNZ", 3) == 0)
    {
     if (!is_zero (last_result))
      {
       char *lbl = line + 3;
       while (*lbl == ' ') lbl++;
       uint16_t target = find_label (lbl);
       if (target == 0xFFFF)
        return false;
       ip = target;
       continue;
      }
     ip++;
     continue;
    }
   
   if (strncmp (line, "CALL", 4) == 0)
    {
     char *lbl = line + 4;
     
     // Check for CALLZ/CALLNZ
     if (*lbl == 'Z')
      {
       lbl++;
       if (is_zero (last_result))
        {
         while (*lbl == ' ') lbl++;
         uint16_t target = find_label (lbl);
         if (target == 0xFFFF)
          return false;
         if (sp >= 256)
          return false; // Stack overflow
         stack[sp++] = ip;
         ip = target;
         continue;
        }
       ip++;
       continue;
      }
     else if (strncmp (lbl, "NZ", 2) == 0)
      {
       lbl += 2;
       if (!is_zero (last_result))
        {
         while (*lbl == ' ') lbl++;
         uint16_t target = find_label (lbl);
         if (target == 0xFFFF)
          return false;
         if (sp >= 256)
          return false;
         stack[sp++] = ip;
         ip = target;
         continue;
        }
       ip++;
       continue;
      }
     else
      {
       // Regular CALL
       while (*lbl == ' ') lbl++;
       uint16_t target = find_label (lbl);
       if (target == 0xFFFF)
        return false;
       if (sp >= 256)
        return false;
       stack[sp++] = ip;
       ip = target;
       continue;
      }
    }
   
   // Otherwise - evaluate as expression
   if (!child->Calc (line, last_result))
    return false;
   
   ip++;
  }
 
 return true;
}
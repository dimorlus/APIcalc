bool script::pass_buf (int n)
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
       if (pass == 1) lineidx[lines] = (uint16_t)(cp - buffer);

       // Skip empty lines
       while (*cp == '\r' || *cp == '\n' || *cp == ' ')
        {
         if (*cp == '\r' || *cp == '\n')
          {
           *cp = '\0';
          }
         cp++;
        }

       if (!*cp) break;

       lines++;

       if (pass == 1 && LblTable && labels < n)
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
       break;

      case stLbl:
       while (*cp == ':') cp++; // Skip multiple :::
       while (*cp == ' ') cp++; // Skip spaces after :

       if (pass == 1 && plb)
        {
         LblTable[labels].ptr = lines - 1;
         int i                = 0;
         while (i < 8 && isalnum (*cp & 0x7f))
          {
           plb[i++] = toupper (*cp++ & 0x7f);
          }
         while (i < 8) plb[i++] = '\0';
        }
       else
        {
         // First pass - just count
         while (isalnum (*cp & 0x7f)) cp++;
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
     num_lines = lines;
     
     if (labels > 0) LblTable = (tLblTable *)malloc (labels * sizeof (tLblTable));
     if (lines > 0) lineidx = (uint16_t *)malloc (lines * sizeof (uint16_t));

     if ((labels > 0 && !LblTable) || (lines > 0 && !lineidx)) 
      return false;

     if (LblTable)
      memset (LblTable, 0, labels * sizeof (tLblTable));

     // Reset for second pass
     state = stNl;
     cp    = buffer;
    }
  }

 return true;
}
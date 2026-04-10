bool calculator::For(const char* expr, value& res)
{
 if (expr && *expr)
  {
   char sexpr[STRBUF];
   char sfrom[MAXOP];
   char sto[MAXOP];
   char svar[STRBUF];
   
   // Initialize buffers
   sexpr[0] = '\0';
   sfrom[0] = '\0';
   sto[0] = '\0';
   svar[0] = '\0';
   
   const char* p = expr;
   int part = 0; // 0 = expr, 1 = from, 2 = to, 3 = var
   int depth = 0; // depth of nested parentheses and brackets
   int idx = 0;
   
   // Skip leading whitespace
   while (*p && isspace((unsigned char)*p)) p++;
   
   while (*p)
    {
     char ch = *p;
     
     // Track bracket/parenthesis depth
     if (ch == '(' || ch == '[')
      {
       depth++;
      }
     else if (ch == ')' || ch == ']')
      {
       depth--;
       if (depth < 0)
        {
         error(p - expr, "Unmatched closing bracket");
         return false;
        }
      }
     // Process comma only at depth 0 (outside of all brackets)
     else if (ch == ',' && depth == 0)
      {
       // Terminate current part
       switch (part)
        {
        case 0:
         sexpr[idx] = '\0';
         break;
        case 1:
         sfrom[idx] = '\0';
         break;
        case 2:
         sto[idx] = '\0';
         break;
        }
       
       part++;
       if (part > 3)
        {
         error(p - expr, "Too many arguments in for loop");
         return false;
        }
       
       idx = 0;
       p++;
       // Skip whitespace after comma
       while (*p && isspace((unsigned char)*p)) p++;
       continue;
      }
     
     // Copy character to appropriate buffer
     switch (part)
      {
      case 0: // expr
       if (idx < STRBUF - 1)
        sexpr[idx++] = ch;
       else
        {
         error(p - expr, "Expression too long");
         return false;
        }
       break;
      case 1: // from
       if (idx < MAXOP - 1)
        sfrom[idx++] = ch;
       else
        {
         error(p - expr, "From expression too long");
         return false;
        }
       break;
      case 2: // to
       if (idx < MAXOP - 1)
        sto[idx++] = ch;
       else
        {
         error(p - expr, "To expression too long");
         return false;
        }
       break;
      case 3: // var
       if (idx < STRBUF - 1)
        svar[idx++] = ch;
       else
        {
         error(p - expr, "Variable name too long");
         return false;
        }
       break;
      }
     
     p++;
    }
   
   // Terminate the last part
   switch (part)
    {
    case 0:
     sexpr[idx] = '\0';
     break;
    case 1:
     sfrom[idx] = '\0';
     break;
    case 2:
     sto[idx] = '\0';
     break;
    case 3:
     svar[idx] = '\0';
     break;
    }
   
   if (depth != 0)
    {
     error(0, "Unmatched opening bracket");
     return false;
    }
   
   if (part < 3)
    {
     error(0, "Not enough arguments for for loop (expected: expr, from, to, var)");
     return false;
    }
   
   // Trim trailing whitespace from all parts
   auto trim_end = [](char* str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1]))
     str[--len] = '\0';
   };
   
   trim_end(sexpr);
   trim_end(sfrom);
   trim_end(sto);
   trim_end(svar);
   
   // Validate variable name
   if (svar[0] == '\0')
    {
     error(0, "Variable name is empty");
     return false;
    }
   
   // Check that variable starts with letter or underscore
   if (!isalpha((unsigned char)svar[0]) && svar[0] != '_')
    {
     error(0, "Variable name must start with a letter or underscore");
     return false;
    }
   
   // Check that variable contains only letters, digits, and underscores
   for (int i = 1; svar[i] != '\0'; i++)
    {
     if (!isalnum((unsigned char)svar[i]) && svar[i] != '_')
      {
       error(0, "Variable name can only contain letters, digits, and underscores");
       return false;
      }
    }
   
   // TODO: Implement the actual for loop logic here
   // For now, just return success
   return true;
  }
 
 return false;
}
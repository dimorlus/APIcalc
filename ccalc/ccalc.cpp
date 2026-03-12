#include "pch.h"
#include "ccalc.h"
#include <fstream>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <stdio.h>
#include <io.h>

#include "../scalc.h"

char errMsg[512] = { 0 };

int32_t scan_opt (char *str, int32_t initial_opts, int *binwide, char *filename);

ccalc_config::ccalc_config (uint32_t dconf)
{
 opts.calc_flags = dconf;
 opts.binary_width = 64;
 opts.filename[0]  = '\0';
}

bool ccalc_config::parse_single_option (const char *opt)
{
 // Skip initial '/'
 if (*opt == '/') opt++;

 // Extract option name
 char name[32];
 int i = 0;
 while (*opt && *opt != '+' && *opt != '-' && *opt != '=' && i < 31)
  {
   name[i++] = toupper (*opt++);
  }
 name[i] = '\0';

 // Special handling for BW
 if (strcmp (name, "BW") == 0)
  {
   if (*opt == '=')
    {
     opts.binary_width = atoi (opt + 1);
     return true;
    }
   return false;
  }

 // Determine action (+/-)
 bool enable = true;
 if (*opt == '+')
  enable = true;
 else if (*opt == '-')
  enable = false;
 else
  return false;

 // Search for the option in the table
 for (int j = 0; all_options[j].name != NULL; j++)
  {
   if (strcmp (name, all_options[j].name) == 0)
    {
     int flag = all_options[j].flag;

     // Apply the flag
     if (enable)
      {
       opts.calc_flags |= flag;
      }
     else
      {
       opts.calc_flags &= ~flag;
      }

     return true;
    }
  }

 return false;
}

int ccalc_config::parse_cmdline_options (char *cmdline)
{
 int first_option_pos = -1;
 char *p              = cmdline;

 // Search for the first option (starts with /)
 while (*p)
  {
   // Skip whitespace
   while (*p && isspace (*p)) p++;
   if (!*p) break;

   if (*p == '/')
    {
     // Found the first option
     first_option_pos = (int)(p - cmdline);
     break;
    }
   else
    {
     // Not an option - skip to the next whitespace
     while (*p && !isspace (*p)) p++;
    }
  }

 // If options were found - process them
 if (first_option_pos >= 0)
  {
   // Use scan_opt to parse all options
   opts.calc_flags = scan_opt (cmdline + first_option_pos, opts.calc_flags, &opts.binary_width, opts.filename);

   // Trim the string (remove options)
   cmdline[first_option_pos] = '\0';

   // Remove trailing whitespace
   int len = first_option_pos - 1;
   while (len >= 0 && isspace (cmdline[len])) cmdline[len--] = '\0';
  }

 return first_option_pos;
}

int32_t scan_opt (char *str, int32_t initial_opts, int *binwide, char *filename)
{
 int i, j, k, l;
 char c, cc;
 bool cmnt    = false;
 int32_t opts = initial_opts;

 l = 0;
 while (str[l])
  {
   // Skip whitespace
   while (str[l] && (str[l] == ' ' || str[l] == '\t' || str[l] == '\r' || str[l] == '\n')) l++;

   if (!str[l]) break;

   // Handle comments
   if (str[l] == ';' || str[l] == '#')
    {
     // Skip to the end of the line
     while (str[l] && str[l] != '\n') l++;
     continue;
    }

   // Search for an option
   if (str[l] != '/')
    {
     l++;
     continue;
    }

   l++; // Skip '/'

   int cmp = _strnicmp (&str[l], "FILE=",5);
   if (cmp== 0)
    {
     l += 5; // Skip "FILE="
     if (str[l] == '"') l++; // Skip opening quote if present
     int j = 0;
     for (; str[l] && str[l] != '/' && str[l] != '"'; l++)
     {
       filename[j++] = str[l];
     }
     filename[j] = '\0'; 
     //strcpy_s (filename, i, &str[l]);
    }

   // Check for BW=n
   if ((str[l] == 'B' || str[l] == 'b') && (str[l + 1] == 'W' || str[l + 1] == 'w')
       && str[l + 2] == '=')
    {
     l += 3;
     if (binwide) *binwide = atoi (&str[l]);
     // Skip digits
     while (str[l] >= '0' && str[l] <= '9') l++;
     continue;
    }

   // Search for a matching option
   bool found = false;
   for (i = 0; i < OPTS - 1; i++) // -1 to avoid checking NULL sentinel
    {
     j = l;
     k = 0;

     // Compare option name
     while (all_options[i].name[k])
      {
       c = str[j];
       if (c >= 'a' && c <= 'z') c -= ('a' - 'A'); // To upper
       cc = all_options[i].name[k];

       if (c != cc) break;

       j++;
       k++;
      }

     // Check if the name matched completely
     if (all_options[i].name[k] == '\0')
      {
       c = str[j];
       if (c == '+' || c == '-')
        {
         // Found the option!
         if (c == '+')
          opts |= all_options[i].flag;
         else
          opts &= ~all_options[i].flag;

         l     = j + 1;
         found = true;
         break;
        }
      }
    }

   if (!found)
    {
     // Unknown option - skip to whitespace
     while (str[l] && str[l] != ' ' && str[l] != '\t' && str[l] != '\r' && str[l] != '\n'
            && str[l] != ';' && str[l] != '#')
      l++;
    }
  }

 return opts;
}

bool ccalc_config::load_config (const char *filename)
{
 std::ifstream file (filename);
 if (!file.is_open ()) return false;

 // Читаем весь файл в строку
 std::string content ((std::istreambuf_iterator<char> (file)), std::istreambuf_iterator<char> ());
 file.close ();

 // Используем scan_opt для парсинга
 char *str       = const_cast<char *> (content.c_str ());
 opts.calc_flags = scan_opt (str, opts.calc_flags, &opts.binary_width, opts.filename);

 return true;
}

void print_options (int32_t flags, int binary_width)
{
 int count = 0;

 // Print all options from the table
 for (int i = 0; all_options[i].name != NULL; i++)
  {
   if ((flags & all_options[i].flag) == all_options[i].flag) printf ("/%s+", all_options[i].name);
   else printf ("/%s-", all_options[i].name);

   count++;
   if (count % 8 == 0) printf ("\n");
   else printf (" ");
  }

 // Print binary width
 printf ("/BW=%d", binary_width);

 printf ("\n");
}

float__t fhelp (float__t x)
{
 switch ((int)x)
  {
  case 0:
   show_help_overview ();
   break;
  case 1:
   show_help_functions ();
   break;
  case 2:
   show_help_operators ();
   break;
  case 3:
   show_help_formats ();
   break;
  case 4:
   show_help_constants ();
   break;
  case 5:
   show_help_prefixes ();
   break;
  case 6:
   show_help_examples ();
   break;
  case 7:
   show_options_help ();
   break;
  default:
   // Show all sections
   show_help_overview ();
   std::cout << std::endl << std::endl;
   show_help_functions ();
   std::cout << std::endl << std::endl;
   show_help_operators ();
   std::cout << std::endl << std::endl;
   show_help_formats ();
   std::cout << std::endl << std::endl;
   show_help_constants ();
   std::cout << std::endl << std::endl;
   show_help_prefixes ();
   std::cout << std::endl << std::endl;
   show_help_examples ();
   std::cout << std::endl << std::endl;
   show_options_help ();
   break;
  }
 return x;
}

void load_user_constants (calculator &calc, const char *Fname)
{// Load user consts from consts.txt
 char exePath[MAX_PATH];
 int lineNum      = 0;

 if (GetModuleFileNameA (NULL, exePath, MAX_PATH))
  {
   char *lastSlash = strrchr (exePath, '\\');
   if (lastSlash)
    {
     *(lastSlash + 1) = '\0'; // Truncate to directory
     strcat_s (exePath, Fname);

     FILE *f = nullptr;
     if (fopen_s (&f, exePath, "r") == 0 && f)
      {
       char line[1024];
       while (fgets (line, sizeof (line), f))
        {
         lineNum++;
         // Skip empty or comment lines (simple check)
         bool hasContent = false;
         for (char *p = line; *p; ++p)
          {
           if (!isspace ((unsigned char)*p))
            {
             hasContent = true;
             break;
            }
          }
         if (!hasContent) continue;

         // Evaluate line
         float__t result = calc.evaluate (line);
         if (isnan (result))
          {
           // Error
           char msg[128];
           snprintf (msg, sizeof (msg), "Error in %s line: %d", Fname, lineNum);
           snprintf (errMsg, sizeof (errMsg), "%-67.67s\r\n%-67.67s\r\n%-67.67s", msg, line,
                     calc.error ());
           break;
          }
        }
       fclose (f);
      }
    }
  }
}


int main ()
{
 // Get the original command line
 char *cmdline = GetCommandLineA ();

 // Skip the program name
 // It may be in quotes: "c:\path\ccalc.exe" args
 // Or without quotes: c:\path\ccalc.exe args
 bool in_quotes = false;
 if (*cmdline == '"')
  {
   in_quotes = true;
   cmdline++; // Skip the opening quote
  }

 // Skip the program name
 while (*cmdline && (in_quotes ? (*cmdline != '"') : !isspace (*cmdline))) cmdline++;

 if (in_quotes && *cmdline == '"') cmdline++; // Skip the closing quote

 // Skip spaces after the program name
 while (*cmdline && isspace (*cmdline)) cmdline++;

 // If there are no arguments, show usage
 if (*cmdline == '\0')
  {
   show_usage ();
   return 0;
  }

 // Load configuration
 ccalc_config config (PAS + FFLOAT + NRM + CMP + IGR + UNS + HEX + CHR + FBIN + DAT + DEG
                      + STR + FRC + FRI);

 // Find the program directory path
 char exe_path[MAX_PATH];
 GetModuleFileNameA (NULL, exe_path, MAX_PATH);
 char *last_slash = strrchr (exe_path, '\\');
 if (last_slash) *(last_slash + 1) = '\0';

 // Form the path to the configuration file
 char config_path[MAX_PATH];
 strcpy_s (config_path, sizeof (config_path), exe_path);
 strcat_s (config_path, sizeof (config_path), "ccalc.cfg");

 config.load_config (config_path);


 // Copy to a mutable buffer
 char expression[max_expression_length];
 strncpy_s (expression, sizeof (expression), cmdline, _TRUNCATE);

 // Remove quotes around the expression (if any)
 char *expr_ptr  = expression;
 size_t expr_len = strlen (expr_ptr);

 // Remove leading spaces
 while (*expr_ptr && isspace (*expr_ptr)) expr_ptr++;

 // If the expression starts and ends with quotes, remove them
 expr_len = strlen (expr_ptr);
 if (expr_len >= 2 && expr_ptr[0] == '"' && expr_ptr[expr_len - 1] == '"')
  {
   expr_ptr[expr_len - 1] = '\0';
   expr_ptr++;
   expr_len -= 2;
  }

 // Copy back to the beginning of the buffer if needed  
 if (expr_ptr != expression)
  {
   memmove (expression, expr_ptr, strlen (expr_ptr) + 1);
  }

 // Parse options and trim the string
 config.parse_cmdline_options (expression);

 // Check for empty expression
 expr_len = strlen (expression);
 while (expr_len > 0 && isspace (expression[expr_len - 1])) expression[--expr_len] = '\0';


 // Create the calculator
 calculator calc (config.get_options ().calc_flags);

 calc.addfn ("help", (void *)(float__t (*) (float__t))fhelp);

 load_user_constants (calc, "consts.txt");
 load_user_constants (calc, "user.txt");

 if (errMsg[0] != '\0')
  {
   std::cerr << errMsg << std::endl;
   return 1;
  }

  if (!_isatty (_fileno (stdin)))
  {
    char line[1024];
    char result_str[256];
    calc.clrfflags (); // Clear flags before processing the file
    while (fgets (line, sizeof (line), stdin))
    {
      int_t len = strlen (line);
      while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'
                 || isspace ((unsigned char)line[len - 1]))) line[--len] = '\0';
      if (line[0] == '\0')
       {
        printf ("\n");
        continue; // Skip empty lines
       }
      float__t res = calc.evaluate (line);
      calc.printres (result_str, config.get_options ().calc_flags,
                     config.get_options ().binary_width);
      if (config.get_options ().calc_flags & SRC)
       printf ("%s => %s\n", line, result_str);
      else
       printf ("%s\n", result_str);
     }
    return 0;
  }

 if (config.get_options ().calc_flags & OPT)
   print_options (config.get_options ().calc_flags, config.get_options ().binary_width);

  
if (config.get_options().filename[0])
 {
  FILE *f = nullptr;
  char *fname = config.get_options ().filename;
  if (fopen_s (&f, fname, "r") == 0 && f)
   {
    char line[1024];
    char result_str[256];
    calc.clrfflags (); // Clear flags before processing the file
    while (fgets (line, sizeof (line), f))
     {
      int_t len      = strlen (line);
      while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' || 
          isspace ((unsigned char)line[len - 1]))) line[--len] = '\0';
      if (line[0] == '\0')
       {
        printf ("\n");
        continue; // Skip empty lines
       }
      calc.clrfflags (); // Clear flags before processing the file
      calc.evaluate (line);
      calc.printres (result_str, config.get_options ().calc_flags,
                    config.get_options ().binary_width);
      if (config.get_options ().calc_flags & SRC)
       printf ("%s => %s\n", line, result_str);
      else
       printf ("%s\n", result_str);
     }
    fclose (f);
   }
  return 0;
 }

 // Evaluate
 if (expression[0] == '\0')
  {
   printf("Error: Empty expression\n");
   return 1;
  }

{
  char result_str[1600];
  calc.evaluate (expression);
  calc.print (result_str, config.get_options ().calc_flags, config.get_options ().binary_width);
  printf ("%s\n%s\n", expression, result_str);
  return 0;
 }
}
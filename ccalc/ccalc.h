#ifndef CCALC_H
#define CCALC_H

#include "../scalc.h"

struct option_def
{
    const char* name;    // Название опции
    int flag;            // Битовая маска
};


// All supported options definitions
static const option_def all_options[] = 
{
    { "PAS",    PAS    },
    { "SCI",    SCI    },
    { "UPCASE", UPCASE },
  //{ "UTMP",   UTMP   },
    { "FFLOAT", FFLOAT },
    { "IMUL",   IMUL   },
    
    { "DEG",    DEG    },
  //{ "SCF",    SCF    },
  //{ "ENG",    ENG    },
    { "STR",    STR    },
    { "HEX",    HEX    },
    { "OCT",    OCT    },
    { "FBIN",   FBIN   },
    { "DAT",    DAT    },
    { "CHR",    CHR    },
    { "WCH",    WCH    },
  //{ "ESC",    ESC    },
    { "CMP",    CMP    },
    
    { "NRM",    NRM    },
    { "IGR",    IGR    },
    { "UNS",    UNS    },
    { "ALL",    SCI+DEG+SCI+STR+HEX+OCT+FBIN+DAT+CHR+WCH+CMP+NRM+IGR+UNS+FRC+FRI+UTM    },
    { "FRC",    FRC    },
    { "FRI",    FRI    },
    { "FRH",    FRH    },
 //{ "AUT",    AUTO   },
 //   { "AUTO",   AUTO   },
  //{ "MIN",    MIN    },
  //{ "MNU",    MNU    },
    { "UTM",    UTM    },
  //{ "TOP",    TOP    },
	{ "OPT",    OPT    },
    { NULL,     0      } // Sentinel
};

#define OPTS sizeof(all_options)/sizeof(option_def)

struct ccalc_options
{
    int calc_flags;     // Flags for calculator and output
    int binary_width;   // Width for binary format
    
    ccalc_options()
    {
        // Default values
        calc_flags = PAS | SCI | UPCASE | FFLOAT | IMUL;
        binary_width = 64;
    }
};

class ccalc_config
{
private:
    //ccalc_options opts;
    
    bool parse_single_option(const char* opt);
    
public:
    ccalc_options opts;

    ccalc_config(uint32_t dconf);
    
    bool load_config(const char* filename = "ccalc.cfg");
    int parse_cmdline_options(char* cmdline);
    
    const ccalc_options& get_options() const { return opts; }
    ccalc_options& get_options() { return opts; }
};

void show_options_help();
void show_usage();
void show_help(int page);
void print_options(int32_t flags, int binary_width);

// Help functions (from help.cpp)
void show_help_overview();
void show_help_functions();
void show_help_operators();
void show_help_formats();
void show_help_constants();
void show_help_prefixes();
void show_help_examples();

#endif // CCALC_H
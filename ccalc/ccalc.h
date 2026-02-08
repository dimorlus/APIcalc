#ifndef CCALC_H
#define CCALC_H

#include "../scalc.h"

struct option_def
{
    const char* name;    // Название опции
    int flag;            // Битовая маска
};

//#define PAS     (1<<0)  // (RW) Pascal assingment and comparison style (:= =) <-> (= ==)
//#define SCI     (1<<1)  // (WO) Scientific numbers format (2k == 2000)
//#define UPCASE  (1<<2)  // (WO) Case insensetive variables
//#define UTMP    (1<<3)  // (WO) Using $n for temp
//#define FFLOAT  (1<<4)  // (WO) Forced float
//
//#define DEG     (1<<5)  // (RO) Degrees format found
//#define SCF     (1<<6)  // (RO) Scientific (6.8k) format found
//#define ENG     (1<<7)  // (RO) Engineering (6k8) format found
//#define STR     (1<<8)  // (RO) String format found
//#define HEX     (1<<9)  // (RO) Hex format found
//#define OCT     (1<<10) // (RO) Octal format found
//#define fBIN    (1<<11) // (RO) Binary format found
//#define FBIN    (1<<11) // (RO) Binary format found
//#define DAT     (1<<12) // (RO) Date time format found
//#define CHR     (1<<13) // (RO) Char format found
//#define WCH     (1<<14) // (RO) WChar format found
//#define ESC     (1<<15) // (RO) Escape format found
//#define CMP     (1<<16) // (RO) Computing format found
//
//#define NRM     (1<<17) // (UI) Normalized output
//#define IGR     (1<<18) // (UI) Integer output
//#define UNS     (1<<19) // (UI) Unsigned output
//#define ALL     (1<<20) // (UI) All outputs
//#define MIN     (1<<21) // (UI) Esc minimized feature
//#define MNU     (1<<22) // (UI) Show/hide menu feature
//#define UTM     (1<<23) // (RO) Unix time
//#define FRC     (1<<24) // (UI) Fraction output
//#define FRI     (1<<25) // (UI) Fraction inch output
//#define AUTO    (1<<26) // (UI) Auto output
//#define AUT     (1<<26) // (UI) Auto output
//#define TOP     (1<<27) // (UI) Always on top
//#define IMUL	  (1<<28) // (WO) Implicit multiplication

// Определение всех поддерживаемых опций
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
    int calc_flags;     // Флаги для калькулятора и вывода
    int binary_width;   // Ширина для двоичного формата
    
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
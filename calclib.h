// calc.h — pure C interface
#ifdef __cplusplus
extern "C" {
#endif

// RW - set both by calc engine and application
// WO - set only from application
// RO - set only by calc engine
// UI - set and used only from application
#define PAS    (1 << 0) // (RW) Pascal assingment and comparison style (:= =) <-> (= ==)
#define SCI    (1 << 1) // (WO) Scientific numbers format (2k == 2000)
#define UPCASE (1 << 2) // (WO) Case insensetive variables
#define UTMP   (1 << 3) // (WO) Using $n for temp
#define FFLOAT (1 << 4) // (WO) Forced float

#define DEG  (1 << 5)  // (RO) Degrees format found
#define CPX  (1 << 6)  // (RO) Complex format found
#define ENG  (1 << 7)  // (RO) Engineering (6k8) format found
#define STR  (1 << 8)  // (RO) String format found
#define HEX  (1 << 9)  // (RO) Hex format found
#define OCT  (1 << 10) // (RO) Octal format found
#define fBIN (1 << 11) // (RO) Binary format found
#define FBIN (1 << 11) // (RO) Binary format found
#define DAT  (1 << 12) // (RO) Date time format found
#define CHR  (1 << 13) // (RO) Char format found
#define WCH  (1 << 14) // (RO) WChar format found
#define ESC  (1 << 15) // (RO) Escape format found
#define CMP  (1 << 16) // (RO) Computing format found

#define NRM  (1 << 17) // (UI) Normalized output
#define IGR  (1 << 18) // (UI) Integer output
#define UNS  (1 << 19) // (UI) Unsigned output
#define ALL  (1 << 20) // (UI) All outputs
#define MIN  (1 << 21) // (UI) Esc minimized feature (GUI only)
#define OPT  (1 << 21) // (UI) Print options (CLI only)
#define MNU  (1 << 22) // (UI) Show/hide menu feature (GUI only)
#define SRC  (1 << 22) // (UI) Print source expression (CLI only)
#define UTM  (1 << 23) // (UI) Unix time
#define FRC  (1 << 24) // (UI) Fraction output
#define FRI  (1 << 25) // (UI) Fraction inch output
#define FRH  (1 << 26) // (UI) Farenheit input/output
#define TOP  (1 << 27) // (UI) Always on top (GUI only)
#define FLT  (1 << 27) // (UI) Floating point output (CLI only)
#define IMUL (1 << 28) // (WO) Implicit multiplication
#define AUTO (1 << 29) // (UI) Auto output format
#define FCTR (1 << 30) // (UI) Factorization output

 typedef void* HCALC;  // opaque handle, no one outside knows what's inside

 typedef bool (*fnShowImage) (void *bmpObject);

#ifdef CALCLIB_EXPORTS
 #define CALCAPI __declspec(dllexport)
#else
 #define CALCAPI __declspec(dllimport)
#endif


CALCAPI HCALC   __cdecl calc_create (int cfg);
CALCAPI void	__cdecl calc_destroy (HCALC h);
CALCAPI double	__cdecl calc_evaluate(HCALC h, char* expr);
CALCAPI int     __cdecl calc_print (HCALC h, char *str, int options, int binw, int *size);
CALCAPI int		__cdecl calc_print_res (HCALC h, char* str, int options, int binw);
CALCAPI char *	__cdecl calc_error (HCALC h);
CALCAPI void	__cdecl calc_addfn (HCALC h, const char* name, void *fn); //(void *)(int (*) (int))fn
CALCAPI int		__cdecl calc_get_flags(HCALC h);
CALCAPI void	__cdecl calc_clr_flags(HCALC h);
CALCAPI void	__cdecl calc_set_syntax(HCALC h, int syntax);
CALCAPI int		__cdecl calc_get_syntax (HCALC h);
CALCAPI int		__cdecl calc_varlist (HCALC h, char *buf, int bsize, int *maxlen);
CALCAPI void    __cdecl calc_setEscFn (HCALC h, int (__cdecl*fn) (void));
CALCAPI void __cdecl calc_setFileDlgFn (HCALC h, bool (__cdecl *fn) (char *, int));
CALCAPI void __cdecl calc_setShowImageFn (HCALC h, fnShowImage fn);
#ifdef __cplusplus
}
#endif
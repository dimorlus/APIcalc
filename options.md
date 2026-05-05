## Calculator options
The **opt(n)**, **opton(n)**, and **optoff(n)** functions set, enable, and disable the option specified by the bits of the integer n. 
Returns the set options. The calculator's behavior can critically depend on options typically set via the menu and stored 
in the registry (for the GUI version). For example, disabling *Scientific format* causes entering `2k` to generate an error, 
rather than entering `2000`. Disabling *Implicit multiplication* will obviously lead to errors when executing code like `2 pi f` 
; expressions written in *PAS style* are incorrectly interpreted in *C style*. To ensure that expressions defined in `consts.txt`, 
`user.txt`, and *scripts* are correctly interpreted, use the following code at the beginning. This overrides previously set and 
saved options and ensures that the rest of the text is interpreted correctly.
```
PAS    := (1 << 0) ;; (RW) Pascal assingment and comparison style (:= =) <-> (= ==)
SCI    := (1 << 1) ;; (WO) Scientific numbers format (2k == 2000)
UPCASE := (1 << 2) ;; (WO) Case insensetive variables
FFLOAT := (1 << 4) ;; (WO) Forced float
IMUL   := (1 << 28) ;; (WO) Implicit multiplication

opton(PAS+SCI+UPCASE+FFLOAT+IMUL) ;;Switch ON critical options opton(268435479).


;; RW - set both by calc engine and application
;; WO - set only from application
;; RO - set only by calc engine
;; UI - set and used only from application
PAS    :=	(1 << 0)  ;; (RW) Pascal assingment and comparison style (:= =) <-> (= ==)
SCI    :=	(1 << 1)  ;; (WO) Scientific numbers format (2k == 2000)
UPCASE :=	(1 << 2)  ;; (WO) Case insensetive variables
UTMP   :=	(1 << 3)  ;; (WO) Using $n for temp
FFLOAT :=	(1 << 4)  ;; (WO) Forced float

DEG  	 :=	(1 << 5)  ;; (RO) Degrees format found
CPX  	 :=	(1 << 6)  ;; (RO) Complex format found
ENG  	 :=	(1 << 7)  ;; (RO) Engineering (6k8) format found
STR  	 :=	(1 << 8)  ;; (RO) String format found
HEX  	 :=	(1 << 9)  ;; (RO) Hex format found
OCT  	 :=	(1 << 10) ;; (RO) Octal format found
FBIN 	 :=	(1 << 11) ;; (RO) Binary format found
DAT  	 :=	(1 << 12) ;; (RO) Date time format found
CHR  	 :=	(1 << 13) ;; (RO) Char format found
WCH  	 :=	(1 << 14) ;; (RO) WChar format found
ESC  	 :=	(1 << 15) ;; (RO) Escape format found
CMP  	 :=	(1 << 16) ;; (RO) Computing format found

NRM  	 :=	(1 << 17) ;; (UI) Normalized output
IGR  	 :=	(1 << 18) ;; (UI) Integer output
UNS  	 :=	(1 << 19) ;; (UI) Unsigned output
ALL  	 :=	(1 << 20) ;; (UI) All outputs
MIN  	 :=	(1 << 21) ;; (UI) Esc minimized feature (GUI only)
OPT  	 :=	(1 << 21) ;; (UI) Print options (CLI only)
MNU  	 :=	(1 << 22) ;; (UI) Show/hide menu feature (GUI only)
SRC  	 :=	(1 << 22) ;; (UI) Print source expression (CLI only)
UTM  	 :=	(1 << 23) ;; (UI) Unix time
FRC  	 :=	(1 << 24) ;; (UI) Fraction output
FRI  	 :=	(1 << 25) ;; (UI) Fraction inch output
FRH  	 :=	(1 << 26) ;; (UI) Farenheit input/output
TOP  	 :=	(1 << 27) ;; (UI) Always on top (GUI only)
FLT  	 :=	(1 << 27) ;; (UI) Floating point output (CLI only)
IMUL 	 :=	(1 << 28) ;; (WO) Implicit multiplication
AUTO 	 :=	(1 << 29) ;; (UI) Auto output format
FCTR 	 :=	(1 << 30) ;; (UI) Factorization output
SNAN 	 :=	(1 << 31) ;; (RO) Silent NaN
DBG  	 :=	(1 << 32) ;; (WO) Debug mode (prints internal details) (CLI only)
NBLK 	 := (1 << 33) ;; (WO) No block operators (GUI only)
```
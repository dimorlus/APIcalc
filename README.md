# WinAPI Calculator

[Русская версия / Russian version](Readme_ru.md)

A scientific calculator with both **GUI** and **CLI** versions, built using pure Win32 API without MFC dependencies. Supports various number formats, binary operations with configurable width, advanced mathematical functions, and complex numbers.

This calculator project on WinAPI (VS2022) is based on my old project on Cbuilder VCL (BCB6) [fcalc](https://github.com/dimorlus/fcalc), which, in turn, is based on DOS programs and [Ccalc](http://www.garret.ru/ccalc.zip) sources that have been heavily reworked since then.

```
//-< CCALC.CPP >-----------------------------------------------------*--------*
// Ccalc                      Version 1.02       (c) 1998  GARRET    *     ?  *
// (C expression command line calculator)                            *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Oct-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Oct-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------* 
```

## Two Versions Available

### 📊 GUI Version (WinApiCalc / fcalc.exe)

Windows desktop application with native interface, history dropdown, and real-time calculation.

### 💻 CLI Version (ccalc.exe)

Command-line calculator for scripts, automation, and terminal use.

## Features

### GUI Version (WinApiCalc)

* **Scientific Calculator**: Support for trigonometric functions, logarithms, exponentials, and more
* **Expression Parser**: Advanced mathematical expression evaluation with proper operator precedence
* **History Dropdown**: Keep track of previous calculations with easy access through dropdown menu (auto-saves on exit)
* **DPI Awareness**: Automatic scaling for high-DPI displays
* **CHM Help Support**: F1 help integration for external help files
* **Standard Windows Interface**: Native menus, keyboard shortcuts, and clipboard integration
* **Multiple Output Formats**: Display results in decimal, hexadecimal, octal, binary, scientific notation, etc.
* **Configurable Binary Width**: Choose binary display width from 8 to 64 bits
* **Variables Dialog**: View and manage calculation variables
* **Customizable Options**: Case sensitivity, forced float mode, ESC minimization, opacity control
* **Implicit Multiplication**: Optionally omit multiplication operator in common cases

### CLI Version (ccalc)

* **Command-line interface**: Perfect for scripts and automation
* **Same calculation engine**: Identical mathematical capabilities as GUI version
* **Multiple output formats**: Supports all format options via command-line flags
* **Configuration file support**: Load default options from `ccalc.cfg`
* **Batch processing**: Process expressions from scripts or command line
* **Built-in help system**: Access help via `ccalc "help(n)"` command

#### CLI Usage

```bash
# Basic calculations (always use quotes!)
ccalc "2+2"
ccalc "sqrt(3^2+4^2)"
ccalc "sin(pi/4)"

# With options
ccalc "0xFF + 0b1010" /HEX+ /BIN+
ccalc "2k5" /SCI+              # 2500 with scientific notation
ccalc "2+3i" /NRM+              # Complex numbers

# Show help
ccalc "help(0)"                 # Overview
ccalc "help(1)"                 # Functions
ccalc "help(4)"                 # Constants (including physical constants)
ccalc "help(7)"                 # Options

# Configuration file (ccalc.cfg)
# Place in same directory as ccalc.exe
/SCI+ /IMUL+ /BW=64
```

**IMPORTANT**: Always use quotes around expressions! Symbols like `^`, `|`, `\&`, `<`, `>` have special meaning in PowerShell/CMD.

## Supported Functions

### Basic Operations

* Addition (+), Subtraction (-), Multiplication (\*), Division (/)
* Modulo (%), Power (^)
* Parentheses for grouping

### Mathematical Functions

* **Trigonometric**: sin, cos, tan, asin, acos, atan
* **Hyperbolic**: sinh, cosh, tanh
* **Logarithmic**: ln (natural log), log (base 10)
* **Other**: sqrt, exp, abs, floor, ceil, round

### **Complex Number Support**

* All mathematical operations and functions (including trigonometric, hyperbolic, exponential, logarithmic, power, and square root) support complex arguments and return complex results where appropriate.
* Complex numbers can be entered in the form `a+bi` (e.g., `1+2i`, `3-4i`).
* Functions like `sin`, `cos`, `exp`, `abs`, etc., work with both real and complex arguments.
* The result is displayed in the form `a+bi` if the imaginary part is nonzero.

### Constants

* **pi**: 3.14159265358979323846
* **e**: 2.71828182845904523536
* **phi**: 1.61803398874989484820 (Golden ratio)

#### Physical Constants (CODATA 2018)

**Fundamental:**

* **c0**: Speed of light in vacuum (299792458 m/s)
* **hp**: Planck constant (6.62607015e-34 J·s)
* **hb**: Reduced Planck constant ℏ (1.054571817e-34 J·s)
* **gn**: Gravitational constant (6.67430e-11 m³/(kg·s²))
* **na**: Avogadro constant (6.02214076e23 mol⁻¹)
* **kb**: Boltzmann constant (1.380649e-23 J/K)
* **rg**: Universal gas constant (8.314462618 J/(mol·K))

**Electromagnetic:**

* **e0**: Electric constant (8.8541878128e-12 F/m)
* **u0**: Magnetic constant (1.25663706212e-6 H/m)
* **z0**: Impedance of vacuum (376.730313668 Ω)

**Particle:**

* **qe**: Elementary charge (1.602176634e-19 C)
* **me**: Electron mass (9.1093837015e-31 kg)
* **mp**: Proton mass (1.67262192369e-27 kg)
* **mn**: Neutron mass (1.67492749804e-27 kg)
* **rel**: Classical electron radius (2.8179403262e-15 m)
* **a0**: Bohr radius (5.29177210903e-11 m)

**Astronomical:**

* **au**: Astronomical unit (1.495978707e11 m)
* **ly**: Light year (9.4607304725808e15 m)
* **pc**: Parsec (3.0856775814914e16 m)

**Additional:**

* **ry**: Rydberg constant (10973731.568160 m⁻¹)
* **sb**: Stefan-Boltzmann constant (5.670374419e-8 W/(m²·K⁴))

**Integer Limits:**

* **max32**, **maxint**: Maximum signed 32-bit (2147483647)
* **maxu32**, **maxuint**: Maximum unsigned 32-bit (4294967295)
* **max64**, **maxlong**: Maximum signed 64-bit
* **maxu64**, **maxulong**: Maximum unsigned 64-bit

**System:**

* **version**: Calculator version
* **timezone**: System timezone offset (hours)
* **daylight**: Daylight saving time flag
* **tz**: Current timezone with DST

## Output Formats

* Scientific
* Normalized
* Fraction
* Computing
* Integer
* Unsigned
* Hexadecimal
* Octal
* Binary (configurable width)
* Character
* Wide Character
* Date/Time
* Unix Timestamp
* Degrees
* String
* Inch
* Auto

## Usage

1. Type mathematical expressions in the input field
2. Results are calculated automatically as you type
3. Press Enter to add calculations to history
4. Use the history dropdown to recall previous calculations
5. Copy results with Ctrl+C or paste expressions with Ctrl+V
6. Access format options and binary width settings via the Calc menu

### Example Expressions

```
2 + 3 \* 4
sin(pi/2)
sqrt(16) + log(100)
2^3 \* e
(5 + 3) / (2 - 1)

# With Implicit Multiplication enabled:
2sin(pi/2)          # Same as 2 \* sin(pi/2) = 2
3(4+5)              # Same as 3 \* (4+5) = 27
(1+2)(3+4)          # Same as (1+2) \* (3+4) = 21
2PI                 # Same as 2 \* PI ≈ 6.28 (uppercase PI to avoid pico suffix)
3e                  # Same as 3 \* e ≈ 8.15 (with uppercase E is 3e+18, 3 exa)
```

### Implicit Multiplication

When **Implicit Multiplication** is enabled (via Calc menu), you can omit the `\*` operator in these common cases:

1. **Number before function**: `2sin(x)` → `2 \* sin(x)`
2. **Number before parenthesis**: `3(4+5)` → `3 \* (4+5)`
3. **Parenthesis after parenthesis**: `(1+2)(3+4)` → `(1+2) \* (3+4)`
4. **Number before variable/constant**: `2PI` → `2 \* PI` (uppercase recommended)

**Important notes about scientific suffixes and imaginary unit:**

* This feature is **disabled by default** — enable it via **Calc → Implicit multiplication** menu.

* **Scientific suffixes have highest priority**: Single-letter suffixes (k, M, G, m, u, n, **p**, f, a, etc.) are recognized first if followed by space, operator, or end of expression.
  
  * `3k` → `3×10³` = 3000 (kilo)
  * `3p` → `3×10⁻¹²` = 3e-12 (pico)
  * `3p+5` → `3×10⁻¹² + 5` (pico suffix applies)

* **Lowercase `i` and `j` are reserved for imaginary unit**: They are recognized **after** scientific suffixes.
  
  * `5i` → `0+5i` (imaginary number, not `5 \* i` variable)
  * `5j` → `0+5j` (alternative imaginary unit notation)
  * `3pi` → `0+3e-12i` (interpreted as 3 pico + imaginary unit `i`)

* **Use uppercase for variables to avoid conflicts**:
  
  * `3PI` → `3 \* PI` ≈ 9.42 (implicit multiplication with PI constant)
  * `3pI` → `3 \* pI` (also works: `p` followed by uppercase `I` avoids pico suffix)
  * `5I` → `5 \* I` (uppercase `I` as variable, not imaginary unit)
  * `2e` → `2 \* e` ≈ 5.44 (uppercase `E` as constant, not exponent notation)
  * `2E` → `22e+18` (exa)

**Recommended naming convention**: Use **UPPERCASE** for constants/variables to avoid conflicts with scientific suffixes and imaginary unit (e.g., `PI`, `E`, `PHI`, `X`, `Y`, `Z`).

## Keyboard Shortcuts

* **Ctrl+N**: Clear input field
* **Ctrl+Shift++**: Increase window opacity
* **Ctrl+Shift+-**: Decrease window opacity
* **Ctrl+C**: Copy result to clipboard
* **Ctrl+V**: Paste from clipboard to expression
* **F1**: Show help contents

## Menu Options

### Calc

* **Pas style / C style**: Toggle between Pascal-like syntax (use `^` for power, `:=` for assignment) and C-like syntax
* **Case sensitive**: Toggle case sensitivity for variables/functions
* **Forced float**: Force all results to floating-point format
* **Implicit multiplication**: Allow omitting `\*` operator (e.g., `2sin(x)`, `3PI`)
* **ESC minimized**: Minimize window on ESC key
* **Always on top**: Keep calculator window on top of other windows
* **Opacity**: Adjust window transparency
* Format submenu: Select output format
* Binary width submenu: Select binary display width (8, 16, 24, 32, 48, 64 bits)
* View variables: Open variables dialog
* Exit: Close application

### Edit

* Copy: Copy current result
* Paste: Paste text into expression field
* Clear History: Remove all history items

### View

* Standard Mode: Basic calculator interface
* Scientific Mode: Extended scientific functions

### Help

* Help Contents: Open CHM help file (F1)
* About: Application information

## Build Requirements

* Visual Studio 2019 or later
* Windows SDK 10.0 or later
* C++17 standard

### Building GUI Version (WinApiCalc)

1. Open `WinApiCalc.sln` in Visual Studio
2. Select configuration (Debug/Release) and platform (x86/x64)
3. Build solution (F7)
4. Executable will be in `x64/Release/fcalc.exe` or similar

### Building CLI Version (ccalc)

1. Open `ccalc.vcxproj` in Visual Studio
2. Select configuration and platform
3. Build project
4. Executable will be in configured output directory

Both projects share the same calculation engine (`scalc.cpp`, `sfmts.cpp`, `sfunc.cpp`).

## Dependencies

* comctl32.lib (Windows Common Controls)
* htmlhelp.lib (CHM help support)

## Architecture

The application uses a clean separation of concerns:

* **Common calculation engine** (shared by both GUI and CLI):
  
  * **scalc.cpp/h**: Mathematical expression parser and evaluator
  * **sfmts.cpp/h**: Number format conversion and output
  * **sfunc.cpp/h**: Mathematical functions implementation

* **GUI version** (WinApiCalc):
  
  * **WinApiCalc.cpp/h**: Main application window and UI management
  * **Resources**: Menus, dialogs, and accelerators

* **CLI version** (ccalc):
  
  * **ccalc.cpp/h**: Command-line interface and argument parsing
  * **help.cpp**: Built-in help system

## Project Structure

```
APICalc/
├── scalc.cpp/h          # Calculation engine (shared)
├── sfmts.cpp/h          # Format handling (shared)
├── sfunc.cpp/h          # Math functions (shared)
├── WinApiCalc.cpp/h     # GUI application
├── WinApiCalc.rc        # GUI resources
├── ccalc/               # CLI version
│   ├── ccalc.cpp/h      # CLI main
│   ├── help.cpp         # Help system
│   └── ccalc.cfg        # Configuration file
├── fcalc.chm            # Help file
└── README.md            # This file
```

## Help File Integration

Place your CHM help file as `fcalc.chm` in the same directory as the executable to enable F1 help functionality.

## License

Copyright (C) 2024. All rights reserved.

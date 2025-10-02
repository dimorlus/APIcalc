# WinAPI Calculator

A scientific calculator application built using pure Win32 API without MFC dependencies. Supports various number formats, binary operations with configurable width, and advanced mathematical functions.

## Features

- **Scientific Calculator**: Support for trigonometric functions, logarithms, exponentials, and more
- **Expression Parser**: Advanced mathematical expression evaluation with proper operator precedence
- **History Dropdown**: Keep track of previous calculations with easy access through dropdown menu
- **DPI Awareness**: Automatic scaling for high-DPI displays
- **CHM Help Support**: F1 help integration for external help files
- **Standard Windows Interface**: Native menus, keyboard shortcuts, and clipboard integration
- **Multiple Output Formats**: Display results in decimal, hexadecimal, octal, binary, scientific notation, etc.
- **Configurable Binary Width**: Choose binary display width from 8 to 64 bits
- **Variables Dialog**: View and manage calculation variables
- **Customizable Options**: Case sensitivity, forced float mode, ESC minimization, opacity control

## Supported Functions

### Basic Operations

- Addition (+), Subtraction (-), Multiplication (*), Division (/)
- Modulo (%), Power (^)
- Parentheses for grouping

### Mathematical Functions

- **Trigonometric**: sin, cos, tan, asin, acos, atan
- **Hyperbolic**: sinh, cosh, tanh
- **Logarithmic**: ln (natural log), log (base 10)
- **Other**: sqrt, exp, abs, floor, ceil, round

### **Complex Number Support**

- All mathematical operations and functions (including trigonometric, hyperbolic, exponential, logarithmic, power, and square root) support complex arguments and return complex results where appropriate.
- Complex numbers can be entered in the form `a+bi` (e.g., `1+2i`, `3-4i`).
- Functions like `sin`, `cos`, `exp`, `abs`, etc., work with both real and complex arguments.
- The result is displayed in the form `a+bi` if the imaginary part is nonzero.

### Constants

- **pi**: 3.14159265358979323846
- **e**: 2.71828182845904523536
- **phi**: 1.61803398874989484820 (Golden ratio)

## Output Formats

- Scientific
- Normalized
- Fraction
- Computing
- Integer
- Unsigned
- Hexadecimal
- Octal
- Binary (configurable width)
- Character
- Wide Character
- Date/Time
- Unix Timestamp
- Degrees
- String
- Inch
- Auto

## Usage

1. Type mathematical expressions in the input field
2. Results are calculated automatically as you type
3. Press Enter to add calculations to history
4. Use the history dropdown to recall previous calculations
5. Copy results with Ctrl+C or paste expressions with Ctrl+V
6. Access format options and binary width settings via the Calc menu

### Example Expressions

```bash
2 + 3 * 4
sin(pi/2)
sqrt(16) + log(100)
2^3 * e
(5 + 3) / (2 - 1)
```

## Keyboard Shortcuts

- **Ctrl+N**: New calculation (clear expression)
- **Ctrl+C**: Copy result to clipboard
- **Ctrl+V**: Paste from clipboard to expression
- **F1**: Show help contents

## Menu Options

### Calc

- Pas style: Toggle pascal-style syntax
- Case sensitive: Toggle case sensitivity
- Forced float: Force floating-point results
- Format submenu: Select output format
- Binary width submenu: Select binary display width (8, 16, 24, 32, 48, 64 bits)
- View variables: Open variables dialog
- ESC minimized: Minimize on ESC key
- Opacity: Adjust window opacity
- Exit: Close application

### Edit

- Copy: Copy current result
- Paste: Paste text into expression field
- Clear History: Remove all history items

### View

- Standard Mode: Basic calculator interface
- Scientific Mode: Extended scientific functions

### Help

- Help Contents: Open CHM help file (F1)
- About: Application information

## Build Requirements

- Visual Studio 2019 or later
- Windows SDK 10.0 or later
- C++17 standard

## Dependencies

- comctl32.lib (Windows Common Controls)
- htmlhelp.lib (CHM help support)

## Architecture

The application uses a clean separation of concerns:

- **WinApiCalc**: Main application window and UI management
- **Calculator**: Mathematical expression parser and evaluator
- **Resources**: Menus, dialogs, and accelerators

## Help File Integration

Place your CHM help file as `help.chm` in the same directory as the executable to enable F1 help functionality.

## License

Copyright (C) 2024. All rights reserved.

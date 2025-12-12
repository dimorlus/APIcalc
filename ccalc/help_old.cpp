#include "pch.h"
#include "ccalc.h"
#include <iostream>

void show_help_functions()
{
    std::cout << "=== MATHEMATICAL FUNCTIONS ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Trigonometric (radians):" << std::endl;
    std::cout << "  sin(x|z), cos(x|z), tan(x|z), cot(x|z)" << std::endl;
    std::cout << "  asin(x), acos(x), atan(x), acot(x)" << std::endl;
    std::cout << std::endl;
    std::cout << "Trigonometric (degrees):" << std::endl;
    std::cout << "  sing(x), cosg(x), tang(x), cotg(x)" << std::endl;
    std::cout << "  asing(x), acosg(x), atang(x), acotg(x)" << std::endl;
    std::cout << std::endl;
    std::cout << "Hyperbolic:" << std::endl;
    std::cout << "  sinh(x|z), cosh(x|z), tanh(x|z), ctnh(x|z)" << std::endl;
    std::cout << "  asinh(x), acosh(x), atanh(x), acoth(x)" << std::endl;
    std::cout << std::endl;
    std::cout << "Exponential & Logarithmic:" << std::endl;
    std::cout << "  exp(x|z)      - e^x" << std::endl;
    std::cout << "  log(x|z)      - Natural log" << std::endl;
    std::cout << "  log10(x)      - Base 10 log" << std::endl;
    std::cout << "  log2(x)       - Base 2 log" << std::endl;
    std::cout << "  logn(x,n)     - Base n log" << std::endl;
    std::cout << "  sqrt(x|z)     - Square root" << std::endl;
    std::cout << "  rootn(x,n)    - n-th root" << std::endl;
    std::cout << std::endl;
    std::cout << "Other:" << std::endl;
    std::cout << "  abs(x|z)      - Absolute value / Complex magnitude" << std::endl;
    std::cout << "  pow(x,y)      - x^y" << std::endl;
    std::cout << "  fact(n)       - Factorial" << std::endl;
    std::cout << "  floor(x), ceil(x), round(x)" << std::endl;
    std::cout << "  min(x,y), max(x,y)" << std::endl;
    std::cout << std::endl;
    std::cout << "Complex:" << std::endl;
    std::cout << "  cmplx(re,im)  - Create complex number" << std::endl;
    std::cout << "  re(z)         - Real part" << std::endl;
    std::cout << "  im(z)         - Imaginary part" << std::endl;
    std::cout << "  pol(z)        - Phase angle" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: (x|z) means function supports both real and complex arguments" << std::endl;
}

void show_help_operators()
{
    std::cout << "=== OPERATORS ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Arithmetic:" << std::endl;
    std::cout << "  +, -, *, /, %         - Basic operations" << std::endl;
    std::cout << "  ^                     - Power" << std::endl;
    std::cout << "  ++, --                - Increment/Decrement (prefix & postfix)" << std::endl;
    std::cout << std::endl;
    std::cout << "Bitwise:" << std::endl;
    std::cout << "  ~                     - Complement" << std::endl;
    std::cout << "  &                     - Bitwise AND" << std::endl;
    std::cout << "  |                     - Bitwise OR" << std::endl;
    std::cout << "  ^^ (or xor)           - Bitwise XOR" << std::endl;
    std::cout << "  <<                    - Arithmetic shift left" << std::endl;
    std::cout << "  >>                    - Arithmetic shift right" << std::endl;
    std::cout << "  >>>                   - Logical shift right" << std::endl;
    std::cout << std::endl;
    std::cout << "Comparison:" << std::endl;
    std::cout << "  ==, !=                - Equal, Not equal" << std::endl;
    std::cout << "  <, <=, >, >=          - Less, Less/Equal, Greater, Greater/Equal" << std::endl;
    std::cout << std::endl;
    std::cout << "Logical:" << std::endl;
    std::cout << "  !                     - Logical NOT" << std::endl;
    std::cout << std::endl;
    std::cout << "Assignment:" << std::endl;
    std::cout << "  =                     - Simple assignment" << std::endl;
    std::cout << "  +=, -=, *=, /=, %=    - Compound assignment" << std::endl;
    std::cout << "  &=, |=, ^=            - Bitwise compound assignment" << std::endl;
    std::cout << "  <<=, >>=, >>>=        - Shift compound assignment" << std::endl;
    std::cout << std::endl;
    std::cout << "Pascal mode (/PAS+):" << std::endl;
    std::cout << "  :=                    - Assignment" << std::endl;
    std::cout << "  =                     - Comparison (instead of ==)" << std::endl;
}

void show_help_formats()
{
    std::cout << "=== NUMBER FORMATS ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Integer formats:" << std::endl;
    std::cout << "  0xFF, 0xDEADBEEF      - Hexadecimal (0x prefix)" << std::endl;
    std::cout << "  0o77, 0o755           - Octal (0o prefix)" << std::endl;
    std::cout << "  0b1010, 0b11110000    - Binary (0b prefix)" << std::endl;
    std::cout << "  123, -456             - Decimal" << std::endl;
    std::cout << std::endl;
    std::cout << "Float formats:" << std::endl;
    std::cout << "  3.14, -2.5, 1.23e-4   - Standard notation" << std::endl;
    std::cout << "  .5, 5.                - Partial notation" << std::endl;
    std::cout << std::endl;
    std::cout << "Scientific notation (/SCI+):" << std::endl;
    std::cout << "  2k = 2000             - k (kilo)" << std::endl;
    std::cout << "  2M = 2000000          - M (mega)" << std::endl;
    std::cout << "  2.5k = 2500" << std::endl;
    std::cout << "  6k8 = 6800            - Engineering notation" << std::endl;
    std::cout << "  6.8k = 6800           - Scientific notation" << std::endl;
    std::cout << std::endl;
    std::cout << "Special formats:" << std::endl;
    std::cout << "  12`34'56\"            - Degrees/minutes/seconds" << std::endl;
    std::cout << "  @2024-12-11           - Date format" << std::endl;
    std::cout << "  'A', 'AB'             - Character constants" << std::endl;
    std::cout << "  \"string\"              - String literals" << std::endl;
    std::cout << std::endl;
    std::cout << "Complex numbers:" << std::endl;
    std::cout << "  2+3i                  - Rectangular form" << std::endl;
    std::cout << "  -1-2i" << std::endl;
    std::cout << "  i, j                  - Imaginary unit (lowercase only!)" << std::endl;
}

void show_help_constants()
{
    std::cout << "=== PREDEFINED CONSTANTS ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Mathematical:" << std::endl;
    std::cout << "  PI, pi                - π (3.14159265358979...)" << std::endl;
    std::cout << "  E, e                  - Euler's number (2.71828182845904...)" << std::endl;
    std::cout << "  i, j                  - Imaginary unit (√-1)" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: Constants are case-insensitive (with /UPCASE+ option)" << std::endl;
}

void show_help_prefixes()
{
    std::cout << "=== SI PREFIXES (with /SCI+) ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Large:" << std::endl;
    std::cout << "  Y  - yotta  (10^24)" << std::endl;
    std::cout << "  Z  - zetta  (10^21)" << std::endl;
    std::cout << "  E  - exa    (10^18)" << std::endl;
    std::cout << "  P  - peta   (10^15)" << std::endl;
    std::cout << "  T  - tera   (10^12)" << std::endl;
    std::cout << "  G  - giga   (10^9)" << std::endl;
    std::cout << "  M  - mega   (10^6)" << std::endl;
    std::cout << "  k  - kilo   (10^3)" << std::endl;
    std::cout << "  da - deka   (10^1)" << std::endl;
    std::cout << std::endl;
    std::cout << "Small:" << std::endl;
    std::cout << "  d  - deci   (10^-1)" << std::endl;
    std::cout << "  c  - centi  (10^-2)" << std::endl;
    std::cout << "  m  - milli  (10^-3)" << std::endl;
    std::cout << "  u  - micro  (10^-6)" << std::endl;
    std::cout << "  n  - nano   (10^-9)" << std::endl;
    std::cout << "  p  - pico   (10^-12)" << std::endl;
    std::cout << "  f  - femto  (10^-15)" << std::endl;
    std::cout << "  a  - atto   (10^-18)" << std::endl;
    std::cout << "  z  - zepto  (10^-21)" << std::endl;
    std::cout << "  y  - yocto  (10^-24)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  5k = 5000" << std::endl;
    std::cout << "  2.5M = 2500000" << std::endl;
    std::cout << "  100u = 0.0001" << std::endl;
    std::cout << "  6k8 = 6800   (engineering notation)" << std::endl;
}

void show_help_examples()
{
    std::cout << "=== EXAMPLES ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Basic arithmetic:" << std::endl;
    std::cout << "  2+2" << std::endl;
    std::cout << "  2^10" << std::endl;
    std::cout << "  sqrt(3^2+4^2)" << std::endl;
    std::cout << std::endl;
    std::cout << "Hex/Binary:" << std::endl;
    std::cout << "  0xFF + 0b1010" << std::endl;
    std::cout << "  0xDEADBEEF & 0xFF" << std::endl;
    std::cout << std::endl;
    std::cout << "Scientific notation:" << std::endl;
    std::cout << "  1k + 500       (= 1500)" << std::endl;
    std::cout << "  6k8            (= 6800)" << std::endl;
    std::cout << "  2.2M           (= 2200000)" << std::endl;
    std::cout << std::endl;
    std::cout << "Complex numbers:" << std::endl;
    std::cout << "  (2+3i)*(4-5i)" << std::endl;
    std::cout << "  sqrt(-1)" << std::endl;
    std::cout << std::endl;
    std::cout << "Implicit multiplication (/IMUL+):" << std::endl;
    std::cout << "  2PI            (= 2*PI)" << std::endl;
    std::cout << "  2sin(PI/4)     (= 2*sin(PI/4))" << std::endl;
    std::cout << "  (1+2)(3+4)     (= (1+2)*(3+4))" << std::endl;
    std::cout << std::endl;
    std::cout << "Variables:" << std::endl;
    std::cout << "  x=5; y=10; x+y" << std::endl;
    std::cout << "  r=10; PI*r^2" << std::endl;
}

void show_help_overview()
{
    std::cout << "=== CALCULATOR HELP ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Available help topics:" << std::endl;
    std::cout << "  help(0)  - This overview" << std::endl;
    std::cout << "  help(1)  - Mathematical functions" << std::endl;
    std::cout << "  help(2)  - Operators" << std::endl;
    std::cout << "  help(3)  - Number formats" << std::endl;
    std::cout << "  help(4)  - Predefined constants" << std::endl;
    std::cout << "  help(5)  - SI prefixes" << std::endl;
    std::cout << "  help(6)  - Usage examples" << std::endl;
    std::cout << std::endl;
    std::cout << "Quick start:" << std::endl;
    std::cout << "  2+2                    - Simple calculation" << std::endl;
    std::cout << "  2^8                    - Power" << std::endl;
    std::cout << "  sqrt(2)                - Functions" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  - C-style expressions with operators and functions" << std::endl;
    std::cout << "  - Complex number support" << std::endl;
    std::cout << "  - Multiple number formats (hex, octal, binary, scientific)" << std::endl;
    std::cout << "  - Variables and assignments" << std::endl;
    std::cout << "  - Implicit multiplication (2PI, 2sin(x))" << std::endl;
    std::cout << "  - SI prefixes (k, M, G, m, u, n, etc.)" << std::endl;
}

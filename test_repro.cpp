#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scalc.h"

// Mock implementation of symbol table functions for the test
symbol* calculator::add(t_symbol tag, const char* name, void* func) { return NULL; }
symbol* calculator::add(t_symbol tag, v_func fidx, const char* name, void* func) { return NULL; }
symbol* calculator::find(const char* name, void* func) { return NULL; }
int calculator::hscanf(char* str, int_t &ival, int &nn) { return 0; }
int calculator::bscanf(char* str, int_t &ival, int &nn) { return 0; }
int calculator::oscanf(char* str, int_t &ival, int &nn) { return 0; }
int calculator::xscanf(char* str, int len, int_t &ival, int &nn) { return 0; }
float__t calculator::dstrtod(char *s, char **endptr) { return 0.0; }
float__t calculator::tstrtod(char *s, char **endptr) { return 0.0; }
void calculator::engineering(float__t mul, char * &fpos, float__t &fval) {}
void calculator::scientific(char * &fpos, float__t &fval) {}
void calculator::addfvar(const char* name, float__t val) {}
void calculator::addivar(const char* name, int_t val) {}
void calculator::addlvar(const char* name, float__t fval, int_t ival) {}
int calculator::varlist(char* buf, int bsize, int* maxlen) { return 0; }
unsigned calculator::string_hash_function(char* p) { return 0; }
int calculator::format_out(int Options, int binwide, int n, float__t fVal, float__t imVal, int64_t iVal, char* expr, char strings[20][80]) { return 0; }
bool IsNaN(const double fVal) { return fVal != fVal; }
bool IsNaNL(const long double ldVal) { return ldVal != ldVal; }

double Factorial(double x) { return 1.0; }
long double Factorial(long double x) { return 1.0; }

// Minimal main to test evaluate
int main() {
    calculator calc;
    calc.syntax(IMUL | PAS | SCI | UPCASE); // Enable implicit multiplication

    // Test 1: 10abs(5) - Should work
    char expr1[] = "10abs(5)";
    printf("Testing: %s\n", expr1);
    float__t res1 = calc.evaluate(expr1);
    printf("Result 1: %f (Expected 50.0)\n", (double)res1);

    // Test 2: 1*10abs(5) - Fails
    char expr2[] = "1*10abs(5)";
    printf("Testing: %s\n", expr2);
    float__t res2 = calc.evaluate(expr2);
    printf("Result 2: %f (Expected 50.0)\n", (double)res2);
    if (IsNaN(res2)) printf("Error: %s\n", calc.error());

    // Test 3: 1 10abs(5) - Fails
    char expr3[] = "1 10abs(5)";
    printf("Testing: %s\n", expr3);
    float__t res3 = calc.evaluate(expr3);
    printf("Result 3: %f (Expected 50.0)\n", (double)res3);
    if (IsNaN(res3)) printf("Error: %s\n", calc.error());

    return 0;
}

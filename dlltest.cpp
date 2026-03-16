#include "calclib.h"

int main() {
    HCALC calc = calc_create();
    calc_set_syntax(calc, PAS | SCI | UPCASE);
    
    calc_evaluate(calc, "2+2*2");
    
    char result[256];
    calc_print_res(calc, result, FFLOAT, 64);
    printf("%s", result);
    
    calc_destroy(calc);
    return 0;
}
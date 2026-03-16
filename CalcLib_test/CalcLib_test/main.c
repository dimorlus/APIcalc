#include <stdio.h>
#include <stdlib.h>
#include "calclib.h"

int main(void)
{
    printf("=== Calculator DLL Test ===\n\n");

    // Создаём калькулятор
    HCALC calc = calc_create();
    if (!calc) {
        printf("Error: Failed to create calculator\n");
        return 1;
    }

    // Настраиваем синтаксис
    calc_set_syntax(calc, PAS | SCI | UPCASE);

    // Тест 1: Простое выражение
    printf("Test 1: 2+2*2\n");
    calc_evaluate(calc, "2+2*2");
    
    char result[512];
    calc_print_res(calc, result, FFLOAT, 64);
    printf("Result: %s\n", result);
    printf("Flags: 0x%X\n\n", calc_get_flags(calc));

    // Тест 2: Научная нотация
    printf("Test 2: 1.5k + 500\n");
    calc_clr_flags(calc);
    calc_evaluate(calc, "1.5k + 500");
    calc_print_res(calc, result, FFLOAT, 64);
    printf("Result: %s\n", result);
    printf("Flags: 0x%X\n\n", calc_get_flags(calc));

    // Тест 3: Шестнадцатеричные числа
    printf("Test 3: 0xFF + 0x10\n");
    calc_clr_flags(calc);
    calc_evaluate(calc, "0xFF + 0x10");
    calc_print_res(calc, result, HEX, 64);
    printf("Result: %s\n", result);
    printf("Flags: 0x%X\n\n", calc_get_flags(calc));

    // Тест 4: Функции
    printf("Test 4: sin(pi/2)\n");
    calc_clr_flags(calc);
    calc_evaluate(calc, "sin(pi/2)");
    calc_print_res(calc, result, FFLOAT, 64);
    printf("Result: %s\n", result);
    printf("Flags: 0x%X\n\n", calc_get_flags(calc));

    // Тест 5: Переменные
    printf("Test 5: x:=10; y:=20; x+y\n");
    calc_clr_flags(calc);
    calc_evaluate(calc, "x:=10; y:=20; x+y");
    calc_print_res(calc, result, FFLOAT, 64);
    printf("Result: %s\n", result);
    printf("Flags: 0x%X\n\n", calc_get_flags(calc));

    // Тест 6: long double size
    printf ("Test 6: float_sz\n");
    calc_clr_flags (calc);
    calc_evaluate (calc, "float_sz");
    calc_print_res (calc, result, FFLOAT, 64);
    printf ("Result: %s\n", result);
    printf ("Flags: 0x%X\n\n", calc_get_flags (calc));

    // Освобождаём ресурсы
    calc_destroy(calc);

    printf("\n=== All tests completed ===\n");
    printf("Press Enter to exit...");
    getchar();
    
    return 0;
}
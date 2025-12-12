#include "pch.h"
#include "ccalc.h"
#include <fstream>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include "../scalc.h"


int32_t scan_opt(char* str, int32_t initial_opts, int* binwide);

ccalc_config::ccalc_config(uint32_t dconf)
{
	opts.calc_flags = dconf;
}

bool ccalc_config::parse_single_option(const char* opt)
{
    // Пропускаем начальный '/'
    if (*opt == '/') opt++;
    
    // Извлекаем имя опции
    char name[32];
    int i = 0;
    while (*opt && *opt != '+' && *opt != '-' && *opt != '=' && i < 31)
    {
        name[i++] = toupper(*opt++);
    }
    name[i] = '\0';
    
    // Специальная обработка для BW
    if (strcmp(name, "BW") == 0)
    {
        if (*opt == '=')
        {
            opts.binary_width = atoi(opt + 1);
            return true;
        }
        return false;
    }
    
    // Определяем действие (+/-)
    bool enable = true;
    if (*opt == '+')
        enable = true;
    else if (*opt == '-')
        enable = false;
    else
        return false;
    
    // Ищем опцию в таблице
    for (int j = 0; all_options[j].name != NULL; j++)
    {
        if (strcmp(name, all_options[j].name) == 0)
        {
            int flag = all_options[j].flag;
            
            // Применяем флаг
            if (enable)
            {
                opts.calc_flags |= flag;
            }
            else
            {
                opts.calc_flags &= ~flag;
            }
            
            return true;
        }
    }
    
    return false;
}

int ccalc_config::parse_cmdline_options(char* cmdline)
{
    int first_option_pos = -1;
    char* p = cmdline;
    
    // Ищем первую опцию (начинается с /)
    while (*p)
    {
        // Пропускаем пробелы
        while (*p && isspace(*p)) p++;
        if (!*p) break;
        
        if (*p == '/')
        {
            // Нашли первую опцию
            first_option_pos = (int)(p - cmdline);
            break;
        }
        else
        {
            // Не опция - пропускаем до следующего пробела
            while (*p && !isspace(*p)) p++;
        }
    }
    
    // Если нашли опции - обрабатываем их
    if (first_option_pos >= 0)
    {
        // Используем scan_opt для парсинга всех опций
        opts.calc_flags = scan_opt(cmdline + first_option_pos, opts.calc_flags, &opts.binary_width);
        
        // Обрезаем строку (убираем опции)
        cmdline[first_option_pos] = '\0';
        
        // Убираем trailing пробелы
        int len = first_option_pos - 1;
        while (len >= 0 && isspace(cmdline[len]))
            cmdline[len--] = '\0';
    }
    
    return first_option_pos;
}

int32_t scan_opt(char* str, int32_t initial_opts, int* binwide)
{
    int i, j, k, l;
    char c, cc;
    bool cmnt = false;
    int32_t opts = initial_opts;

    l = 0;
    while (str[l])
    {
        // Пропускаем whitespace
        while (str[l] && (str[l] == ' ' || str[l] == '\t' || str[l] == '\r' || str[l] == '\n'))
            l++;
        
        if (!str[l]) break;
        
        // Обработка комментариев
        if (str[l] == ';' || str[l] == '#')
        {
            // Пропускаем все до конца строки
            while (str[l] && str[l] != '\n')
                l++;
            continue;
        }
        
        // Ищем опцию
        if (str[l] != '/')
        {
            l++;
            continue;
        }
        
        l++; // Пропускаем '/'
        
        // Проверяем на BW=n
        if ((str[l] == 'B' || str[l] == 'b') && 
            (str[l+1] == 'W' || str[l+1] == 'w') && 
            str[l+2] == '=')
        {
            l += 3;
            if (binwide)
                *binwide = atoi(&str[l]);
            // Пропускаем цифры
            while (str[l] >= '0' && str[l] <= '9')
                l++;
            continue;
        }
        
        // Ищем совпадение с опцией
        bool found = false;
        for (i = 0; i < OPTS - 1; i++) // -1 чтобы не проверять NULL sentinel
        {
            j = l;
            k = 0;
            
            // Сравниваем имя опции
            while (all_options[i].name[k])
            {
                c = str[j];
                if (c >= 'a' && c <= 'z') c -= ('a' - 'A'); // To upper
                cc = all_options[i].name[k];
                
                if (c != cc) break;
                
                j++;
                k++;
            }
            
            // Проверяем, что имя совпало полностью
            if (all_options[i].name[k] == '\0')
            {
                c = str[j];
                if (c == '+' || c == '-')
                {
                    // Нашли опцию!
                    if (c == '+')
                        opts |= all_options[i].flag;
                    else
                        opts &= ~all_options[i].flag;
                    
                    l = j + 1;
                    found = true;
                    break;
                }
            }
        }
        
        if (!found)
        {
            // Неизвестная опция - пропускаем до whitespace
            while (str[l] && str[l] != ' ' && str[l] != '\t' && 
                   str[l] != '\r' && str[l] != '\n' && 
                   str[l] != ';' && str[l] != '#')
                l++;
        }
    }
    
    return opts;
}

bool ccalc_config::load_config(const char* filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    
    // Читаем весь файл в строку
    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    file.close();
    
    // Используем scan_opt для парсинга
    char* str = const_cast<char*>(content.c_str());
    opts.calc_flags = scan_opt(str, opts.calc_flags, &opts.binary_width);
    
    return true;
}

void print_options(int32_t flags, int binary_width)
{
    int count = 0;
    
    // Печатаем все опции из таблицы
    for (int i = 0; all_options[i].name != NULL; i++)
    {
        if (flags & all_options[i].flag)
            std::cout << "/" << all_options[i].name << "+";
        else
            std::cout << "/" << all_options[i].name << "-";
        
        count++;
        if (count % 8 == 0)
            std::cout << std::endl;
        else
            std::cout << " ";
    }
    
    // Печатаем binary width
    std::cout << "/BW=" << binary_width;
    
    std::cout << std::endl;
}


float__t fhelp(float__t x)
{
    switch ((int)x)
    {
        case 0:
            show_help_overview();
            break;
        case 1:
            show_help_functions();
            break;
        case 2:
            show_help_operators();
            break;
        case 3:
            show_help_formats();
            break;
        case 4:
            show_help_constants();
            break;
        case 5:
            show_help_prefixes();
            break;
        case 6:
            show_help_examples();
            break;
        case 7:
            show_options_help();
            break;
        default:
            // Показываем все разделы
            show_help_overview();
            std::cout << std::endl << std::endl;
            show_help_functions();
            std::cout << std::endl << std::endl;
            show_help_operators();
            std::cout << std::endl << std::endl;
            show_help_formats();
            std::cout << std::endl << std::endl;
            show_help_constants();
            std::cout << std::endl << std::endl;
            show_help_prefixes();
            std::cout << std::endl << std::endl;
            show_help_examples();
            std::cout << std::endl << std::endl;
            show_options_help();
            break;
	}
    return x;
}   

int main()
{
    // Получаем исходную командную строку
    char* cmdline = GetCommandLineA();
    
    // Пропускаем имя программы
    // Может быть в кавычках: "c:\path\ccalc.exe" args
    // Или без кавычек: c:\path\ccalc.exe args
    bool in_quotes = false;
    if (*cmdline == '"')
    {
        in_quotes = true;
        cmdline++; // Пропускаем открывающую кавычку
    }
    
    // Пропускаем имя программы
    while (*cmdline && (in_quotes ? (*cmdline != '"') : !isspace(*cmdline)))
        cmdline++;
    
    if (in_quotes && *cmdline == '"')
        cmdline++; // Пропускаем закрывающую кавычку
    
    // Пропускаем пробелы после имени программы
    while (*cmdline && isspace(*cmdline))
        cmdline++;
    
    // Если нет аргументов - показываем usage
    if (*cmdline == '\0')
    {
        show_usage();
        return 0;
    }
    
    // Загружаем конфигурацию
    ccalc_config config(PAS+ FFLOAT + SCF + NRM + CMP + IGR + UNS + HEX + CHR + fBIN + DAT + DEG + STR + FRC + FRI);
    
    // Находим путь к каталогу программы
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash)
        *(last_slash + 1) = '\0';
    
    // Формируем путь к файлу конфигурации
    char config_path[MAX_PATH];
    strcpy_s(config_path, sizeof(config_path), exe_path);
    strcat_s(config_path, sizeof(config_path), "ccalc.cfg");
    
    config.load_config(config_path);
    
    // Копируем в изменяемый буфер
    char expression[max_expression_length];
    strncpy_s(expression, sizeof(expression), cmdline, _TRUNCATE);
    
    // Убираем кавычки вокруг выражения (если есть)
    char* expr_ptr = expression;
    size_t expr_len = strlen(expr_ptr);
    
    // Убираем начальные пробелы
    while (*expr_ptr && isspace(*expr_ptr))
        expr_ptr++;
    
    // Если выражение начинается и заканчивается кавычками - убираем их
    expr_len = strlen(expr_ptr);
    if (expr_len >= 2 && expr_ptr[0] == '"' && expr_ptr[expr_len - 1] == '"')
    {
        expr_ptr[expr_len - 1] = '\0';
        expr_ptr++;
        expr_len -= 2;
    }
    
    // Копируем обратно в начало буфера если нужно
    if (expr_ptr != expression)
    {
        memmove(expression, expr_ptr, strlen(expr_ptr) + 1);
    }
    
    // Парсим опции и обрезаем строку
    config.parse_cmdline_options(expression);
    
    // Проверяем на пустое выражение
    expr_len = strlen(expression);
    while (expr_len > 0 && isspace(expression[expr_len - 1]))
        expression[--expr_len] = '\0';
    
    if (expr_len == 0)
    {
        std::cerr << "Error: Empty expression" << std::endl;
        return 1;
    }
    
    // Создаём калькулятор
    calculator calc(config.get_options().calc_flags);
    
    calc.addfn("help", (void*)(float__t(*)(float__t))fhelp);
    
    // Вычисляем
    __int64 iVal = 0;
    float__t imVal = 0;
    float__t result = calc.evaluate(expression, &iVal, &imVal);
    
    // Проверяем ошибки
 /*   if (calc.error()[0] != '\0')
    {
        std::cerr << "Error: " << calc.error();
        if (calc.errps() >= 0)
            std::cerr << " at position " << calc.errps();
        std::cerr << std::endl;
        return 1;
    }
 */   
    // Выводим результат
    //int calculator::print(char* str, int Options, int binwide, float__t fVal, float__t imVal, int64_t iVal, int* size)

	char result_str[1600];
    calc.print(
		result_str,
        config.get_options().calc_flags,
        config.get_options().binary_width,
        result,
        imVal,
        iVal
    );
    
    if (config.get_options().calc_flags & OPT)
        print_options(config.get_options().calc_flags, config.get_options().binary_width);
	printf("%s\r\n%s\r\n", expression, result_str);
    return 0;
}
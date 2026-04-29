Синтаксис:
;;Comment
JMP start

:subpr
expression
RET

:start
expession;;comment
expession;expession;;comment
:loop
expession
CALL subpr
JNZ loop

Операторы ':' - В начале строки перед меткой - первое слово за ним - метка (до 8 символов, остальные обрезаются), дальше игнорируется 
(или, как вариант, выражение до конца строки). 'JMP', 'JZ', 'JNZ', 'CALL', 'CALLZ', 'CALLNZ', 'RET' (если стек возврата пуст - завершаем скрипт, 
если переполнился - выходим с ошибкой).

1 Загружаю весь файл как бинарник.
FILE* f = fopen("script.txt", "rb");
if (f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f); // Получаем размер
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1); 
    if (buffer) {
        fread(buffer, 1, size, f);
        buffer[size] = 0; // Терминатор для безопасности
    }
    fclose(f);
}

HANDLE hFile = CreateFileA("script.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
if (hFile != INVALID_HANDLE_VALUE) {
    DWORD size = GetFileSize(hFile, NULL); // Для файлов < 4Гб этого достаточно
    char* buffer = (char*)VirtualAlloc(NULL, size + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    DWORD bytesRead;
    ReadFile(hFile, buffer, size, &bytesRead, NULL);
    buffer[size] = 0;
    
    CloseHandle(hFile);
}

2 сканирую буфер первый раз, считаю число строк и число меток.
3 выделяю массивы под индекс строк и метки.
3 сканирую буфер второй раз, заполняю индексный массив и адреса меток (тут можно вместо JMP label 
  или JZ/JNZ label вставить трехбайтный код перехода (op, index) получится ограничение на 
  64k строк (хватит на все (c)). Оператор условного перехода проверяет на 0 скалярный результат 
  последнего выражения. 
4 Можно выполнять. 
5 Как индекс до последнего дошел - возвращаем результат последнего вычисления любого типа.  

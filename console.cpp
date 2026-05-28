void OpenDebugConsole() 
{
    // 1. Проверяем, вдруг консоль уже создана, чтобы не плодить окна
    if (GetStdHandle(STD_OUTPUT_HANDLE) == INVALID_HANDLE_VALUE || !GetConsoleWindow()) 
	{
        AllocConsole();
        
        // 2. Перенаправляем стандартные потоки Си-рантайма в новое окно
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
        
        // 3. (Опционально) Настраиваем кодировку UTF-8, чтобы prnf не ломал русские буквы
        SetConsoleCP(65001);
        SetConsoleOutputCP(65001);

        HWND hwnd = GetConsoleWindow();
        if (hwnd) 
		{
          HMENU hMenu = GetSystemMenu(hwnd, FALSE);
          if (hMenu) 
		  {
           // Отключаем кнопку "Закрыть" (крестик) и пункт меню Alt+F4
           EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
          }
        }
        
        // 4. Символически чистим экран и приветствуем инженера
        printf("--- Engine Debug Console Activated ---\n");
    }
}

void CloseDebugConsole() 
{
    if (GetConsoleWindow()) 
	 {
        // 1. Сначала закрываем потоки, которые удерживают CONOUT$
        fclose(stdout);
        fclose(stderr);
        fclose(stdin);
        
        // 2. Уничтожаем само консольное окно Windows
        FreeConsole();
        
        // 3. ВАЖНО: Возвращаем stdout в "черную дыру" (NUL). 
        // Если этого не сделать, следующий вызов printf() вызовет краш программы (Access Violation), 
        // так как структура stdout внутри msvcrt.dll останется привязанной к уничтоженному хэндлу!
        freopen("NUL", "w", stdout);
        freopen("NUL", "w", stderr);
        freopen("NUL", "r", stdin);
    }
}

void OpenDebugConsole ()
{
 // 1. Check if the console is already created to avoid spawning multiple windows
 if (GetStdHandle (STD_OUTPUT_HANDLE) == INVALID_HANDLE_VALUE || !GetConsoleWindow ())
  {
   AllocConsole ();
   SetConsoleCtrlHandler (NULL, TRUE);
   
   // 2. Redirect standard C runtime streams to the new window
   (void)freopen ("CONOUT$", "w", stdout);
   (void)freopen ("CONOUT$", "w", stderr);
   (void)freopen ("CONIN$", "r", stdin);

   // 3. (Optional) Set UTF-8 encoding to prevent printf from breaking Russian characters
   SetConsoleCP (65001);
   SetConsoleOutputCP (65001);

   HWND hwnd = GetConsoleWindow ();
   if (hwnd)
    {
     // === Управление позицией и размером консоли ===
     
     // Вариант 1: Установить фиксированную позицию и размер
     // SetWindowPos (hwnd, HWND_TOP, 100, 100, 800, 600, SWP_SHOWWINDOW);
     // Параметры: (окно, Z-order, X, Y, ширина, высота, флаги)
     
     // Вариант 2: Разместить справа от главного окна калькулятора
     HWND hMainWnd = GetForegroundWindow (); // Или FindWindow() с классом вашего окна
     if (hMainWnd && hMainWnd != hwnd)
      {
       RECT mainRect;
       if (GetWindowRect (hMainWnd, &mainRect))
        {
         int consoleX = mainRect.right + 10; // Справа от главного окна с отступом 10px
         int consoleY = mainRect.top;
         int consoleW = 800;
         int consoleH = mainRect.bottom - mainRect.top;
         
         SetWindowPos (hwnd, HWND_TOPMOST, consoleX, consoleY, consoleW, consoleH, 
                       SWP_SHOWWINDOW);
        }
      }
     else
      {
       // Вариант 3: Разместить в правом нижнем углу экрана
       RECT workArea;
       SystemParametersInfo (SPI_GETWORKAREA, 0, &workArea, 0);
       
       int consoleW = 800;
       int consoleH = 600;
       int consoleX = workArea.right - consoleW - 10;
       int consoleY = workArea.bottom - consoleH - 10;
       
       SetWindowPos (hwnd, HWND_TOP, consoleX, consoleY, consoleW, consoleH, 
                     SWP_SHOWWINDOW);
      }
     
     // === Дополнительные настройки ===
     
     // Отключить кнопку закрытия
     HMENU hMenu = GetSystemMenu (hwnd, FALSE);
     if (hMenu)
      {
       EnableMenuItem (hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }
     
     // Установить размер буфера консоли (чтобы больше текста влезало)
     HANDLE hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
     if (hConsole != INVALID_HANDLE_VALUE)
      {
       CONSOLE_SCREEN_BUFFER_INFO csbi;
       if (GetConsoleScreenBufferInfo (hConsole, &csbi))
        {
         COORD bufferSize;
         bufferSize.X = csbi.dwSize.X; // Ширина оставляем как есть
         bufferSize.Y = 9999;           // Большой буфер для прокрутки
         SetConsoleScreenBufferSize (hConsole, bufferSize);
        }
      }
     
     // Установить заголовок окна
     SetConsoleTitle (TEXT ("APICalc Debug Console"));
    }

   // 4. Symbolically clear the screen and welcome the engineer
   printf ("--- Engine Debug Console Activated ---\n");
  }
}


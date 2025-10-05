#include "pch.h"
#include "WinApiCalc.h"
#include <windowsx.h>
#include <shellapi.h>
#include <htmlhelp.h>
#include <fstream>
#include <ctime>

// Debug logging macro - can be enabled/disabled
#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
// Debug logging function
void DebugLog(const char* message)
{
    static std::ofstream debugFile("debug_layout.txt", std::ios::app);
    debugFile << message << std::endl;
    debugFile.flush();
}

// Функция логирования в файл
void LogKeyEvent(const char* location, int key, bool ctrlPressed = false)
{
    std::ofstream logFile("fcalc_keys.log", std::ios::app);
    if (logFile.is_open())
    {
        time_t now = time(0);
        char timeStr[64];
        ctime_s(timeStr, sizeof(timeStr), &now);
        timeStr[strlen(timeStr) - 1] = '\0'; // убираем \n
        
        logFile << timeStr << " - " << location << ": key=" << key;
        if (ctrlPressed) logFile << " (Ctrl pressed)";
        logFile << std::endl;
        logFile.close();
    }
}
#else
// Stub functions when debug logging is disabled
#define DebugLog(x) ((void)0)
#define LogKeyEvent(a,b,c) ((void)0)
#endif

// Global pointer for pseudo-functions access
static WinApiCalc* g_pCalcInstance = nullptr;

// Global variable for variables dialog edit control  
static HWND g_hVariablesEdit = nullptr;

// Forward declarations for pseudo-functions
static long double MenuFunction(long double x);
static long double HelpFunction(long double x);
static long double OpacityFunction(long double x);
static long double BinwideFunction(long double x);
static long double HomeFunction(long double x);
static long double FontFunction(long double x);
static long double VarsFunction(long double x);

WinApiCalc::WinApiCalc()
    : m_hInst(nullptr)
    , m_hWnd(nullptr)
    , m_hExpressionEdit(nullptr)
    , m_hResultEdit(nullptr)
    , m_hHistoryCombo(nullptr)
    , m_hHistoryButton(nullptr)
    , m_hMenu(nullptr)
    , m_pCalculator(nullptr)
    , m_options(NRM | FRC)  // Default flags from SOW
    , m_binWidth(64)
    , m_fontSize(-12)  // Default font size in pixels (negative value) - font(12)
    , m_opacity(255)    // Полностью непрозрачное по умолчанию
    , m_menuVisible(true)
    , m_uiReady(false)      // UI not ready until fully initialized
    , m_windowX(100)        // Дефолтная позиция X
    , m_windowY(100)        // Дефолтная позиция Y
    , m_dpiX(96)
    , m_dpiY(96)
    , m_resultLines(1)
    , m_hWhiteBrush(nullptr)
    , m_originalEditProc(nullptr)
    , m_originalResultEditProc(nullptr)
    , m_originalListBoxProc(nullptr)
{
    // Calculator will be created in OnCreate
    m_hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // White background
    
    // Set global pointer for pseudo-functions
    g_pCalcInstance = this;
}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Global application instance
WinApiCalc* g_pApp = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPICALC, szWindowClass, MAX_LOADSTRING);

    // Create application instance
    g_pApp = new WinApiCalc();
    if (!g_pApp)
    {
        return FALSE;
    }

    // Register window class and initialize
    if (!g_pApp->RegisterClass(hInstance))
    {
        delete g_pApp;
        return FALSE;
    }

    // Perform application initialization:
    if (!g_pApp->InitInstance(hInstance, nCmdShow))
    {
        delete g_pApp;
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPICALC));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // Для акселераторов используем hwnd основного окна, а не msg.hwnd
        HWND hMainWnd = (g_pApp) ? g_pApp->GetMainWindow() : msg.hwnd;
        if (!TranslateAccelerator(hMainWnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete g_pApp;
    return (int) msg.wParam;
}

//
// WinApiCalc Implementation
//

WinApiCalc::~WinApiCalc()
{
    // Clear global pointer immediately to prevent pseudo-functions from accessing this instance
    if (g_pCalcInstance == this)
        g_pCalcInstance = nullptr;
    
    SaveSettings();
    SaveHistory();
    
    if (m_pCalculator)
    {
        delete m_pCalculator;
        m_pCalculator = nullptr;
    }
    
    if (m_hWhiteBrush)
    {
        DeleteObject(m_hWhiteBrush);
    }
}

ATOM WinApiCalc::RegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXA wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPICALC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEA(IDC_WINAPICALC);
    wcex.lpszClassName  = "WinApiCalc";  // Используем простую ANSI строку
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExA(&wcex);
}

BOOL WinApiCalc::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    m_hInst = hInstance;

    // Initialize DPI (without SetProcessDpiAware for compatibility)
    InitializeDPI();

    // Load settings first
    LoadSettings();
    LoadHistory();

    m_hWnd = CreateWindowA("WinApiCalc", "Calc",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        m_windowX, m_windowY, ScaleDPI(WINDOW_MIN_WIDTH), ScaleDPI(WINDOW_MIN_HEIGHT),
        nullptr, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    {
        return FALSE;
    }

    // Настраиваем поддержку прозрачности - ОТКЛЮЧЕНО
    // SetWindowLongA(m_hWnd, GWL_EXSTYLE, GetWindowLongA(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    // SetLayeredWindowAttributes(m_hWnd, 0, m_opacity, LWA_ALPHA);

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    
    // Mark UI as fully ready for pseudo-functions
    m_uiReady = true;

    return TRUE;
}

LRESULT CALLBACK WinApiCalc::EditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WinApiCalc* pThis = (WinApiCalc*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
    
    // КРИТИЧНО: Обрабатываем WM_GETDLGCODE чтобы перехватить системные клавиши
    if (message == WM_GETDLGCODE)
    {
        // Говорим Windows что мы хотим получать ВСЕ клавиши
        return DLGC_WANTALLKEYS;
    }
    
    // Пробуем перехватить через WM_SYSKEYDOWN (системные клавиши как Alt+F4, F1)
    if (message == WM_SYSKEYDOWN)
    {
        LogKeyEvent("EditSubclassProc SYSKEYDOWN", (int)wParam, false);
        
        if (wParam == VK_F1)
        {
            if (pThis) pThis->ShowHelp();
            return 0;
        }
    }
    
    // Отладка - проверяем что subclass вообще работает
    if (message == WM_KEYDOWN)
    {
        // Логируем только специальные клавиши для отладки
        if (wParam == VK_F1 || wParam == VK_ESCAPE)
        {
            LogKeyEvent("EditSubclassProc", (int)wParam, false);
        }
        
        // СИСТЕМНЫЕ КЛАВИШИ ОБРАБАТЫВАЕМ ЗДЕСЬ (не в Main WndProc)
        // Main WndProc не получает фокус клавиатуры!
        
        if (wParam == VK_F1)
        {
            if (pThis) pThis->ShowHelp();
            return 0;
        }
        else if (wParam == VK_ESCAPE)
        {
            if (pThis) pThis->OnKeyDown(wParam);
            return 0;
        }
        else if (wParam == VK_RETURN)
        {
            if (pThis) pThis->OnKeyDown(wParam);
            return 0;
        }
       else if (wParam == VK_DOWN || wParam == VK_UP)
       {
           // Open history on Down/Up when focus is in expression edit.
           // Down -> select first (newest), Up -> select last (oldest).
           if (pThis && pThis->m_hHistoryCombo)
           {
               int count = (int)SendMessageA(pThis->m_hHistoryCombo, LB_GETCOUNT, 0, 0);
               if (count > 0 && !IsWindowVisible(pThis->m_hHistoryCombo))
               {
                   ShowWindow(pThis->m_hHistoryCombo, SW_SHOW);
                   int sel = (wParam == VK_DOWN) ? 0 : max(0, count - 1);
                   SendMessageA(pThis->m_hHistoryCombo, LB_SETCURSEL, sel, 0);
                   SetFocus(pThis->m_hHistoryCombo);
                   pThis->ResizeWindow();
                   pThis->UpdateLayout();
                   return 0; // handled
               }
           }
           // If history already visible or empty, fall through to default handling
        }
        else if (GetKeyState(VK_CONTROL) < 0 && wParam != VK_CONTROL)
        {
            // Delegate Ctrl+key combinations to the main handler for consistency
            if (pThis) {
                pThis->OnKeyDown(wParam);
                return 0;
            }
        }
    }
    
    if (message == WM_PASTE)
    {
        // Handle Paste from context menu - same logic as Ctrl+V
        if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(hWnd))
        {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData)
            {
                char* pszText = (char*)GlobalLock(hData);
                if (pszText)
                {
                    // Get current selection
                    DWORD dwStart, dwEnd;
                    SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
                    
                    // Replace selection with clipboard text
                    SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)pszText);
                    
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
        return 0; // Message handled
    }
    
    if (message == WM_LBUTTONDOWN && pThis)
    {
        // Check if history window is visible and close it
        if (IsWindowVisible(pThis->m_hHistoryCombo))
        {
            ShowWindow(pThis->m_hHistoryCombo, SW_HIDE);
            ShowWindow(pThis->m_hResultEdit, SW_SHOW);
            SetFocus(pThis->m_hExpressionEdit);
            pThis->ResizeWindow(); // Изменяем размер окна при закрытии истории
            pThis->UpdateLayout();
        }
    }
    
    if (pThis && pThis->m_originalEditProc)
    {
        return CallWindowProcA(pThis->m_originalEditProc, hWnd, message, wParam, lParam);
    }
    
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WinApiCalc::ResultEditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WinApiCalc* pThis = (WinApiCalc*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
    
    if (message == WM_LBUTTONDOWN && pThis)
    {
        // Check if history window is visible and close it
        if (IsWindowVisible(pThis->m_hHistoryCombo))
        {
            ShowWindow(pThis->m_hHistoryCombo, SW_HIDE);
            ShowWindow(pThis->m_hResultEdit, SW_SHOW);
            SetFocus(pThis->m_hExpressionEdit);
            pThis->UpdateLayout();
        }
    }
    else if (message == WM_KEYDOWN && pThis)
    {
        if (wParam == VK_F1)
        {
            pThis->ShowHelp();
            return 0;
        }
        else if (wParam == VK_ESCAPE)
        {
            pThis->OnKeyDown(wParam); // Pass to main handler
            return 0;
        }
        // For Ctrl combinations, call main handler
        else if (GetKeyState(VK_CONTROL) < 0)
        {
            pThis->OnKeyDown(wParam);
            return 0;
        }
    }
    
    if (pThis && pThis->m_originalResultEditProc)
    {
        return CallWindowProcA(pThis->m_originalResultEditProc, hWnd, message, wParam, lParam);
    }
    
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WinApiCalc::ListBoxSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WinApiCalc* pThis = (WinApiCalc*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
    static POINT lastMousePos = {-1, -1}; // Track last mouse position
    static bool keyboardInUse = false;    // Track if keyboard was just used
    
    if (message == WM_KEYDOWN && pThis)
    {
        keyboardInUse = true; // Mark keyboard as just used
        
        if (wParam == VK_DELETE)
        {
            pThis->DeleteSelectedHistoryItem();
            return 0; // Handled
        }
        else if (wParam == VK_ESCAPE)
        {
            ShowWindow(pThis->m_hHistoryCombo, SW_HIDE);
            ShowWindow(pThis->m_hResultEdit, SW_SHOW); // Show result field back
            SetFocus(pThis->m_hExpressionEdit);
            pThis->ResizeWindow(); // Изменяем размер окна при закрытии истории
            pThis->UpdateLayout(); // Update layout to properly show result field
            keyboardInUse = false; // Reset flag
            lastMousePos = {-1, -1}; // Reset mouse tracking
            return 0; // Handled
        }
        else if (wParam == VK_RETURN)
        {
            // Handle Enter key like selection
            int sel = (int)SendMessage(hWnd, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR && sel >= 0 && sel < (int)pThis->m_history.size())
            {
                pThis->LoadHistoryItem(sel);
                ShowWindow(pThis->m_hHistoryCombo, SW_HIDE);
                ShowWindow(pThis->m_hResultEdit, SW_SHOW); // Show result field back
                SetFocus(pThis->m_hExpressionEdit);
                pThis->EvaluateExpression();
                pThis->ResizeWindow(); // Изменяем размер окна при закрытии истории
                pThis->UpdateLayout(); // Update layout to properly show result field
            }
            keyboardInUse = false; // Reset flag
            lastMousePos = {-1, -1}; // Reset mouse tracking
            return 0; // Handled
        }
        else if (wParam == VK_UP || wParam == VK_DOWN)
        {
            // Let default handler process arrow keys first
            LRESULT result = 0;
            if (pThis->m_originalListBoxProc)
            {
                result = CallWindowProcA(pThis->m_originalListBoxProc, hWnd, message, wParam, lParam);
            }
            else
            {
                result = DefWindowProcA(hWnd, message, wParam, lParam);
            }
            return result; // Arrow keys have priority over mouse
        }
    }
    else if (message == WM_MOUSEMOVE && pThis)
    {
        // Track mouse movement - allow mouse to take control after real movement
        POINT currentPos = { LOWORD(lParam), HIWORD(lParam) };
        
        // If mouse actually moved, allow it to take control
        if (lastMousePos.x != currentPos.x || lastMousePos.y != currentPos.y)
        {
            // If keyboard was used but mouse moved significantly, give control to mouse
            if (keyboardInUse)
            {
                // Check if mouse moved more than just a tiny amount (avoid accidental movements)
                int deltaX = abs(currentPos.x - lastMousePos.x);
                int deltaY = abs(currentPos.y - lastMousePos.y);
                if (deltaX > 2 || deltaY > 2) // More than 2 pixels movement
                {
                    keyboardInUse = false; // Mouse takes control
                }
            }
            
            if (!keyboardInUse)
            {
                // VCL-like behavior: mouse hover selects item immediately
                int itemIndex = (int)SendMessage(hWnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(currentPos.x, currentPos.y));
                
                if (HIWORD(itemIndex) == 0) // Valid item (HIWORD is 0 when point is in client area)
                {
                    int actualIndex = LOWORD(itemIndex);
                    if (actualIndex >= 0 && actualIndex < (int)pThis->m_history.size())
                    {
                        // Select item under mouse cursor
                        SendMessage(hWnd, LB_SETCURSEL, actualIndex, 0);
                    }
                }
            }
            
            lastMousePos = currentPos; // Always update last position
        }
        
        return 0; // Handled
    }
    else if (message == WM_LBUTTONDOWN && pThis)
    {
        keyboardInUse = false; // Mouse click always resets keyboard mode
        
        // VCL-like behavior: click selects and closes history
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        int itemIndex = (int)SendMessage(hWnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
        
        if (HIWORD(itemIndex) == 0) // Valid item
        {
            int actualIndex = LOWORD(itemIndex);
            if (actualIndex >= 0 && actualIndex < (int)pThis->m_history.size())
            {
                // Select and load the item
                SendMessage(hWnd, LB_SETCURSEL, actualIndex, 0);
                pThis->LoadHistoryItem(actualIndex);
                ShowWindow(pThis->m_hHistoryCombo, SW_HIDE);
                ShowWindow(pThis->m_hResultEdit, SW_SHOW);
                SetFocus(pThis->m_hExpressionEdit);
                pThis->EvaluateExpression();
                pThis->UpdateLayout();
            }
        }
        lastMousePos = {-1, -1}; // Reset mouse tracking
        return 0; // Handled
    }
    
    if (pThis && pThis->m_originalListBoxProc)
    {
        return CallWindowProcA(pThis->m_originalListBoxProc, hWnd, message, wParam, lParam);
    }
    
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WinApiCalc::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        if (g_pApp)
        {
            g_pApp->m_hWnd = hWnd; // Set window handle first!
            g_pApp->OnCreate();
        }
        break;
    case WM_COMMAND:
        if (g_pApp)
        {
            g_pApp->OnCommand(wParam);
        }
        break;
    case WM_SIZE:
        if (g_pApp)
        {
            g_pApp->OnSize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_MOVE:
        if (g_pApp)
        {
            g_pApp->OnMove(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_LBUTTONDOWN:
        if (g_pApp)
        {
            g_pApp->OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_NCLBUTTONDOWN:
        // Also try to catch non-client clicks
        if (g_pApp && IsWindowVisible(g_pApp->m_hHistoryCombo))
        {
            // Convert screen coordinates to client coordinates
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            ScreenToClient(hWnd, &pt);
            g_pApp->OnLButtonDown(pt.x, pt.y);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_KEYDOWN:
        if (g_pApp)
        {
            // Логируем в основном WndProc (только для отладки)
            bool ctrlPressedMain = (GetKeyState(VK_CONTROL) < 0);
            LogKeyEvent("Main WndProc", (int)wParam, ctrlPressedMain);
            
            // Прямая обработка горячих клавиш в main WndProc
            bool ctrlPressed = (GetKeyState(VK_CONTROL) < 0);
            
            if (wParam == VK_F1)
            {
                g_pApp->ShowHelp();
                return 0;
            }
            else if (wParam == VK_ESCAPE)
            {
                g_pApp->OnKeyDown(wParam);
                return 0;
            }
            else if (ctrlPressed && wParam != VK_CONTROL)
            {
                switch (wParam)
                {
                case 'R':
                    g_pApp->WrapExpressionWith("root2(", ")");
                    return 0;
                case 'S':
                    g_pApp->WrapExpressionWith("(", ")^2");
                    return 0;
                case 'I':
                    g_pApp->WrapExpressionWith("1/(", ")");
                    return 0;
                case VK_OEM_4: // Ctrl+[
                case VK_OEM_6: // Ctrl+]
                    g_pApp->WrapExpressionWith("(", ")");
                    return 0;
                case VK_HOME:
                    SetWindowPos(hWnd, nullptr, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                    g_pApp->UpdateMenuChecks();
                    return 0;
                }
            }
            
            g_pApp->OnKeyDown(wParam);
        }
        break;
    case WM_PAINT:
        if (g_pApp)
        {
            g_pApp->OnPaint();
        }
        break;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
        // Set white background for edit controls (including readonly ones)
        if (g_pApp && g_pApp->m_hWhiteBrush)
        {
            HWND hControl = (HWND)lParam;
            // Check if this is one of our edit controls or variables dialog edit
            if (hControl == g_pApp->m_hExpressionEdit || hControl == g_pApp->m_hResultEdit || hControl == g_hVariablesEdit)
            {
                SetBkColor((HDC)wParam, RGB(255, 255, 255)); // White background
                SetTextColor((HDC)wParam, RGB(0, 0, 0));     // Black text
                return (LRESULT)g_pApp->m_hWhiteBrush;
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_DESTROY:
        if (g_pApp)
        {
            g_pApp->OnDestroy();
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void WinApiCalc::OnCreate()
{
    // Initialize calculator engine
    m_pCalculator = new calculator(m_options);
    
    // Register pseudo-functions for calculator engine
    m_pCalculator->addfn("menu", (void*)(float__t(*)(float__t))MenuFunction);
    m_pCalculator->addfn("help", (void*)(float__t(*)(float__t))HelpFunction);
    m_pCalculator->addfn("opacity", (void*)(float__t(*)(float__t))OpacityFunction);
    m_pCalculator->addfn("binwide", (void*)(float__t(*)(float__t))BinwideFunction);
    m_pCalculator->addfn("font", (void*)(float__t(*)(float__t))FontFunction);
    m_pCalculator->addfn("vars", (void*)(float__t(*)(float__t))VarsFunction);
    m_pCalculator->addfn("home", (void*)(float__t(*)(float__t))HomeFunction);

    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    // Create history button first
    m_hHistoryButton = CreateWindowExA(
        0,
        "BUTTON", 
        ">>",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        ScaleDPI(WINDOW_MIN_WIDTH - 27), // Moved 2 pixels left and made 2 pixels narrower
        0,
        ScaleDPI(23), 
        ScaleDPI(CONTROL_HEIGHT),
        m_hWnd, 
        (HMENU)IDC_HISTORY_BUTTON, 
        m_hInst, 
        nullptr);

    // Create result output field (multiline, readonly, white background)
    int resultY = GetControlHeight() + ScaleDPI(5); // Add extra 5px gap
    char debugMsg[200];
    sprintf_s(debugMsg, "OnCreate: GetControlHeight()=%d, resultY=%d, ScaleDPI(5)=%d", GetControlHeight(), resultY, ScaleDPI(5));
    DebugLog(debugMsg);
    
    m_hResultEdit = CreateWindowExA(
        0, // Remove WS_EX_CLIENTEDGE for flat appearance
        "EDIT", 
        "",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | ES_RIGHT | WS_BORDER,
        0, // No margins 
        resultY, // Position after input field using dynamic height + gap
        ScaleDPI(WINDOW_MIN_WIDTH), // Full width, no margins
        ScaleDPI(CONTROL_HEIGHT * 5),  // Enough for several lines
        m_hWnd, 
        (HMENU)IDC_RESULT_EDIT, 
        m_hInst, 
        nullptr);

    if (!m_hResultEdit)
    {
        MessageBoxA(m_hWnd, "Failed to create result edit control", "Error", MB_OK);
        return;
    }

    // Subclass result edit to handle clicks when history is visible
    m_originalResultEditProc = (WNDPROC)SetWindowLongPtrA(m_hResultEdit, GWLP_WNDPROC, (LONG_PTR)ResultEditSubclassProc);
    SetWindowLongPtrA(m_hResultEdit, GWLP_USERDATA, (LONG_PTR)this);

    // Create history as an owned popup so it can extend beyond the main window bounds.
    // Owner = m_hWnd so it will hide with the main window; WS_EX_TOOLWINDOW avoids taskbar entry.
    m_hHistoryCombo = CreateWindowExA(
        WS_EX_TOOLWINDOW,
        "LISTBOX",
        "",
        WS_POPUP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        0, // initial pos set by UpdateLayout (screen coords)
        0,
        ScaleDPI(WINDOW_MIN_WIDTH),
        ScaleDPI(CONTROL_HEIGHT * 5),
        m_hWnd, // owner window (keeps it tied to main window)
        NULL,   // no menu handle for popup windows (do not pass control ID here)
        m_hInst,
        nullptr);

    if (!m_hHistoryCombo)
    {
        MessageBoxA(m_hWnd, "Failed to create history listbox control", "Error", MB_OK);
        return;
    }

    // Ensure the popup is initially hidden (CreateWindowEx with WS_POPUP may still be visible on some platforms)
    ShowWindow(m_hHistoryCombo, SW_HIDE);

    // Subclass history listbox to handle Del and Esc keys
    m_originalListBoxProc = (WNDPROC)SetWindowLongPtrA(m_hHistoryCombo, GWLP_WNDPROC, (LONG_PTR)ListBoxSubclassProc);
    SetWindowLongPtrA(m_hHistoryCombo, GWLP_USERDATA, (LONG_PTR)this);

    // Set monospaced font for controls
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

    if (hFont)
    {
        SendMessage(m_hResultEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(m_hHistoryCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    // Get menu handle
    m_hMenu = ::GetMenu(m_hWnd);
    
    // Apply saved menu visibility (MNU flag): when MNU is set, menu should be hidden
    ::SetMenu(m_hWnd, (m_options & MNU) ? NULL : m_hMenu);
    m_menuVisible = ((m_options & MNU) == 0);
    DrawMenuBar(m_hWnd);

    // Update menu states
    UpdateMenuChecks();
    
    // Apply saved "always on top" (TOP) option at startup
    SetWindowPos(m_hWnd, (m_options & TOP) ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // Set window opacity
    SetWindowOpacity(m_opacity);
    
    // Create expression input LAST so it draws on top
    int inputHeight = GetControlHeight();
    sprintf_s(debugMsg, "OnCreate: Input height=%d (GetControlHeight=%d)", inputHeight, GetControlHeight());
    DebugLog(debugMsg);
    
    m_hExpressionEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT", 
        "",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        0, 0, // No margins
        ScaleDPI(WINDOW_MIN_WIDTH - 30), // Leave space for button, no margins
        inputHeight, // Dynamic height based on font size
        m_hWnd, 
        (HMENU)IDC_EXPRESSION_EDIT, 
        m_hInst, 
        nullptr);

    if (!m_hExpressionEdit)
    {
        MessageBoxA(m_hWnd, "Failed to create expression edit control", "Error", MB_OK);
        return;
    }

    // Subclass expression edit to handle Enter key
    m_originalEditProc = (WNDPROC)SetWindowLongPtrA(m_hExpressionEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
    SetWindowLongPtrA(m_hExpressionEdit, GWLP_USERDATA, (LONG_PTR)this);

    // Set font for expression edit too
    if (hFont)
    {
        SendMessage(m_hExpressionEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    // Set focus to expression edit
    SetFocus(m_hExpressionEdit);
    
    // Set initial window title
    SetWindowTextA(m_hWnd, "Calculator - Initial");
    
    // Load saved expression if any and set cursor at end
    if (!m_currentExpression.empty())
    {
        SetWindowTextA(m_hExpressionEdit, m_currentExpression.c_str());
        // Set cursor to end of text without selection
        SendMessage(m_hExpressionEdit, EM_SETSEL, m_currentExpression.length(), m_currentExpression.length());
    }
    
    // Always update title to reflect current expression (even if empty)
    OnExpressionChanged();
    
    // Populate history combo (already in correct order - latest first)
    UpdateHistoryCombo();
    
    // Set fixed window size immediately at creation
    ResizeWindow();
        
// Debug logging for history control was removed from OnCreate because yPos/displayLines are
// only defined inside UpdateLayout(). Detailed logs for UpdateLayout are added in that
// function where those variables are in scope.
}

void WinApiCalc::OnCommand(WPARAM wParam)
{
    int wmId = LOWORD(wParam);
    int wmEvent = HIWORD(wParam);

    switch (wmId)
    {
    // Calculator menu items
    case ID_CALC_PASSTYLE:
        ToggleOption(PAS);
        break;
    case ID_CALC_CASESENSETIVE:
        ToggleOption(UPCASE);
        break;
    case ID_CALC_FORCEDFLOAT:
        ToggleOption(FFLOAT);
        break;
    case ID_CALC_ESCMINIMIZED:
        ToggleOption(MIN);
        break;
    case ID_CALC_ALWAYSONTOP:
        ToggleOption(TOP);
        break;
    case ID_CALC_OPACITY:
        // Insert "opacity(100)" text into expression field
        {
            char currentExpr[2048];
            GetWindowTextA(m_hExpressionEdit, currentExpr, sizeof(currentExpr));
            std::string newExpr = std::string(currentExpr) + "opacity(100)";
            SetWindowTextA(m_hExpressionEdit, newExpr.c_str());
            SetFocus(m_hExpressionEdit);
            // Position cursor at the end
            SendMessage(m_hExpressionEdit, EM_SETSEL, newExpr.length(), newExpr.length());
        }
        break;
    case ID_CALC_EXIT:
        DestroyWindow(m_hWnd);
        break;

    // Format menu items
    case ID_FORMAT_SCIENTIFIC:
        ToggleOption(SCI);
        break;
    case ID_FORMAT_NORMALIZED:
        ToggleOption(NRM);
        break;
    case ID_FORMAT_FRACTION:
        ToggleOption(FRC);
        break;
    case ID_FORMAT_COMPUTING:
        ToggleOption(CMP);
        break;
    case ID_FORMAT_INTEGER:
        ToggleOption(IGR);
        break;
    case ID_FORMAT_UNSIGNED:
        ToggleOption(UNS);
        break;
    case ID_FORMAT_HEX:
        ToggleOption(HEX);
        break;
    case ID_FORMAT_OCTAL:
        ToggleOption(OCT);
        break;
    case ID_FORMAT_BINARY:
        ToggleOption(FBIN);
        break;
    case ID_FORMAT_CHAR:
        ToggleOption(CHR);
        break;
    case ID_FORMAT_WIDECHAR:
        ToggleOption(WCH);
        break;
    case ID_FORMAT_DATETIME:
        ToggleOption(DAT);
        break;
    case ID_FORMAT_UNIXTIME:
        ToggleOption(UTM);
        break;
    case ID_FORMAT_DEGREESE:
        ToggleOption(DEG);
        break;
    case ID_FORMAT_STRING:
        ToggleOption(STR);
        break;
    case ID_FORMAT_INCH:
        ToggleOption(FRI);
        break;
    case ID_FORMAT_AUTO:
        ToggleOption(AUT);
        break;
    case ID_FORMAT_ALL:
        // Set all format options (not toggle)
        m_options |= (SCI | NRM | FRC | CMP | IGR | UNS | HEX | OCT | FBIN | CHR | WCH | DAT | UTM | DEG | STR | FRI | AUT);
        UpdateMenuChecks();
        EvaluateExpression();
        break;

    // Binary width menu items
    case ID_BINARYWIDTH_8:
        SetBinaryWidth(8);
        break;
    case ID_BINARYWIDTH_16:
        SetBinaryWidth(16);
        break;
    case ID_BINARYWIDTH_24:
        SetBinaryWidth(24);
        break;
    case ID_BINARYWIDTH_32:
        SetBinaryWidth(32);
        break;
    case ID_BINARYWIDTH_48:
        SetBinaryWidth(48);
        break;
    case ID_BINARYWIDTH_64:
        SetBinaryWidth(64);
        break;

    // Hot keys
    case IDM_HELP_CONTENTS:
        ShowHelp();
        break;
    case IDM_CTRL_HOME:
        OnCtrlHome();
        break;
    case IDM_CTRL_R:
        OnCtrlR();
        break;
    case IDM_CTRL_S:
        OnCtrlS();
        break;
    case IDM_CTRL_I:
        OnCtrlI();
        break;
    case IDM_CTRL_LBRACKET:
        OnCtrlLBracket();
        break;
    case IDM_CTRL_RBRACKET:
        OnCtrlRBracket();
        break;
    case IDM_CTRL_PLUS:
        OnCtrlPlus();
        break;
    case IDM_CTRL_MINUS:
        OnCtrlMinus();
        break;

    // Control notifications
    case IDC_EXPRESSION_EDIT:
        if (wmEvent == EN_CHANGE)
        {
            OnExpressionChanged();
        }
        break;
    case IDC_HISTORY_BUTTON:
        {
            // Toggle history combo visibility
            BOOL isVisible = IsWindowVisible(m_hHistoryCombo);
            ShowWindow(m_hHistoryCombo, isVisible ? SW_HIDE : SW_SHOW);
            if (!isVisible)
            {
                // Set focus to history listbox and select first item
                SetFocus(m_hHistoryCombo);
                if (SendMessage(m_hHistoryCombo, LB_GETCOUNT, 0, 0) > 0)
                {
                    SendMessage(m_hHistoryCombo, LB_SETCURSEL, 0, 0); // Select first item
                }
                // Hide result field when history is visible
                //ShowWindow(m_hResultEdit, SW_HIDE);
            }
            else
            {
                // Show result field when history is hidden
                ShowWindow(m_hResultEdit, SW_SHOW);
                SetFocus(m_hExpressionEdit);
            }
            ResizeWindow(); // Изменяем размер окна при переключении истории
            UpdateLayout();
        }
        break;
    case IDC_HISTORY_COMBO:
        if (wmEvent == LBN_SELCHANGE)
        {
            // Just handle selection change - don't close history yet
            // History will close on Enter key or double-click
        }
        else if (wmEvent == LBN_DBLCLK)
        {
            int sel = (int)SendMessage(m_hHistoryCombo, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR && sel >= 0 && sel < (int)m_history.size())
            {
                LoadHistoryItem(sel);
                ShowWindow(m_hHistoryCombo, SW_HIDE); // Закрываем историю
                ShowWindow(m_hResultEdit, SW_SHOW); // Show result field back
                SetFocus(m_hExpressionEdit); // Возвращаем фокус
                EvaluateExpression(); // Сразу пересчитываем выражение
                ResizeWindow(); // Изменяем размер окна при закрытии истории
                UpdateLayout(); // Update layout to properly show result field
            }
        }
        break;
    case ID_CALC_VIEWVARIABLES:
        ShowVariablesDialog();
        break;
    default:
        DefWindowProc(m_hWnd, WM_COMMAND, wParam, 0);
    }
}
void WinApiCalc::OnSize(int width, int height) 
{
    if (m_hExpressionEdit && m_hResultEdit && m_hHistoryCombo)
    {
        UpdateLayout();
    }
}

void WinApiCalc::OnMove(int x, int y)
{
    RECT rect;
    if (GetWindowRect(m_hWnd, &rect)) {
        m_windowX = rect.left;
        m_windowY = rect.top;
        SaveSettings();
    }
}

void WinApiCalc::OnExpressionChanged()
{
    // Get current expression using ANSI
    char buffer[2048];
    int textLength = GetWindowTextA(m_hExpressionEdit, buffer, sizeof(buffer));
    m_currentExpression = buffer;
    
    // Update window title with current expression
    std::string title;
    if (m_currentExpression.empty())
    {
        title = "Calc";
    }
    else
    {
        title = "Calc: " + m_currentExpression;
    }
    
    // Limit title length to avoid too long titles
    if (title.length() > 100)
    {
        title = title.substr(0, 97) + "...";
    }
    
    SetWindowTextA(m_hWnd, title.c_str());
    
    // Evaluate expression as user types
    EvaluateExpression();
}

void WinApiCalc::OnEnterPressed()
{
    // Add current expression to history
    if (!m_currentExpression.empty())
    {
        AddToHistory(m_currentExpression);
        
        // Get numeric result and put it in expression field as %.16Lg format
        if (!m_pCalculator)
        {
            return;
        }
        
        try
        {
            char exprBuf[2048];
            memset(exprBuf, 0, sizeof(exprBuf));
            strncpy_s(exprBuf, m_currentExpression.c_str(), sizeof(exprBuf) - 1);
            exprBuf[sizeof(exprBuf) - 1] = '\0';
            
            m_pCalculator->syntax(m_options);
            
            int64_t iVal = 0;
            long double imVal = 0.0;
            long double fVal = m_pCalculator->evaluate(exprBuf, &iVal, &imVal);

            // Check for errors
            char* error = m_pCalculator->error();
            if (error && strlen(error) > 0)
            {
                return; // Don't replace on error
            }
            
            // Format result as %.16Lg as specified in SOW
            char resultStr[64];
            if (imVal == 0) sprintf_s(resultStr, "%.16Lg", fVal);
            else sprintf_s(resultStr, "%.16Lg%+.16Lg%c", fVal, imVal, m_pCalculator->Ichar());

            // Put result in expression field
            SetWindowTextA(m_hExpressionEdit, resultStr);
        }
        catch (...)
        {
            // Don't replace expression on error
        }
    }
}

void WinApiCalc::EvaluateExpression()
{
    if (!m_pCalculator)
    {
        SetWindowTextA(m_hResultEdit, "Calculator engine not initialized");
        return;
    }
    
    try {
        // Prepare expression buffer
        char exprBuf[2048];
        memset(exprBuf, 0, sizeof(exprBuf));
        strncpy_s(exprBuf, m_currentExpression.c_str(), sizeof(exprBuf) - 1);
        exprBuf[sizeof(exprBuf) - 1] = '\0';

        // Set calculator options
        m_pCalculator->syntax(m_options);

        // Evaluate expression
        int64_t iVal = 0;
		long double imVal = 0.0;
        long double fVal = m_pCalculator->evaluate(exprBuf, &iVal, &imVal);

       
        // Get syntax flags from calculator
        int scfg = m_pCalculator->issyntax();
        
        // Format output using format_out
        char strings[20][80];
        memset(strings, 0, sizeof(strings));
        
        int n = 0;
        try {
            // Call format_out
            n = m_pCalculator->format_out(m_options,  m_binWidth, 0, 
                          fVal, imVal, iVal, exprBuf, strings);
            // Safety check - ensure n is within bounds
            if (n < 0) n = 0;
            if (n > 20) n = 20;
        }
        catch (...)
        {
            // If format_out crashes, provide fallback
            SetWindowTextA(m_hResultEdit, "Error: Number too large to display");
            m_resultLines = 1;
            ResizeWindow();
            return;
        }
        
        // Build result text
        std::string result;
        int lineCount = 0;
        for (int i = 0; i < n && i < 20; i++)
        {
            if (strings[i][0] != '\0') // Skip empty strings
            {
                // Check string length to prevent buffer overflow issues
                size_t len = strnlen(strings[i], 79); // Max 79 chars per string
                if (len > 0)
                {
                    if (!result.empty())
                        result += "\r\n";
                        
                    // Safely append string with length limit
                    result.append(strings[i], len);
                    lineCount++;
                    
                    // Prevent result from becoming too large
                    if (result.length() > 4000) // Reasonable limit
                    {
                        result += "\r\n... (output truncated)";
                        lineCount++;
                        break;
                    }
                }
            }
        }
        
        // If no output from format_out, show simple result
        if (n == 0 || result.empty())
        {
            char fallback[256];
            sprintf_s(fallback, "= %Lg", fVal);
            result = fallback;
            lineCount = 1;
        }
        
        // Update result lines count for window sizing
        m_resultLines = lineCount > 0 ? lineCount : 1;
        if (m_resultLines > 20) m_resultLines = 20; // Increased limit for better display
        
        SetWindowTextA(m_hResultEdit, result.c_str());
        
        // Resize window to fit content
        ResizeWindow();
    }
    catch (...)
    {
        // Защита от краша при некорректных символах
        SetWindowTextA(m_hResultEdit, "Syntax error: invalid characters");
        m_resultLines = 1;
        ResizeWindow();
    }
}
void WinApiCalc::OnPaint() 
{ 
    PAINTSTRUCT ps; 
    HDC hdc = BeginPaint(m_hWnd, &ps); 
    EndPaint(m_hWnd, &ps); 
}

void WinApiCalc::OnDestroy() 
{
    // Get current expression for saving
    if (m_hExpressionEdit)
    {
        char buffer[2048];
        GetWindowTextA(m_hExpressionEdit, buffer, sizeof(buffer));
        m_currentExpression = buffer;
    }
    
    SaveSettings();
    SaveHistory();
}


void WinApiCalc::WrapExpressionWith(const char* prefix, const char* suffix)
{
    if (!m_hExpressionEdit) return;
    
    // Получаем текущее выражение
    char buffer[2048];
    GetWindowTextA(m_hExpressionEdit, buffer, sizeof(buffer));
    
    // Создаем новое выражение с префиксом и суффиксом
    std::string newExpression = prefix;
    newExpression += buffer;
    newExpression += suffix;
    
    // Устанавливаем новое выражение
    SetWindowTextA(m_hExpressionEdit, newExpression.c_str());
    
    // Ставим курсор в конец
    SendMessageA(m_hExpressionEdit, EM_SETSEL, newExpression.length(), newExpression.length());
    
    // Обновляем выражение и пересчитываем
    OnExpressionChanged();
}

void WinApiCalc::OnKeyDown(WPARAM key) 
{
    bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    
    // Отладочное сообщение
    if (key == 'R' && ctrlPressed)
    {
        MessageBoxA(m_hWnd, "Ctrl+R detected!", "Debug", MB_OK);
    }
    
    if (key == VK_F1)
    {
        // F1 - открыть помощь
        MessageBoxA(m_hWnd, "F1 detected!", "Debug", MB_OK);
        ShowHelp();
    }
    else if (key == VK_HOME && ctrlPressed)
    {
        // Ctrl+Home - убрать прозрачность и поместить в 100:100
        MessageBoxA(m_hWnd, "Ctrl+Home detected!", "Debug", MB_OK);
        SetWindowPos(m_hWnd, nullptr, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        // m_opacity = 255; // Прозрачность отключена
        // SetLayeredWindowAttributes(m_hWnd, 0, m_opacity, LWA_ALPHA);
        UpdateMenuChecks(); // Обновим отображение в меню
    }
    else if (ctrlPressed)
    {
        switch (key)
        {
        case 'R': // Ctrl+R - root2(...)
            WrapExpressionWith("root2(", ")");
            break;
        case 'S': // Ctrl+S - (...)^2
            WrapExpressionWith("(", ")^2");
            break;
        case 'I': // Ctrl+I - 1/(...)
            WrapExpressionWith("1/(", ")");
            break;
        case VK_OEM_4: // Ctrl+[ - (...)
            WrapExpressionWith("(", ")");
            break;
        case VK_OEM_6: // Ctrl+] - (...)  
            WrapExpressionWith("(", ")");
            break;
        }
    }
    else if (key == VK_RETURN)
    {
        OnEnterPressed();
    }
    else if (key == VK_ESCAPE)
    {
        if (m_options & MIN) // Если включен "Esc minimized"
        {
            ShowWindow(m_hWnd, SW_MINIMIZE);
        }
        else
        {
            // Закрыть приложение
            PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        }
    }
}

void WinApiCalc::OnLButtonDown(int x, int y)
{
    // Check if history window is visible
    if (IsWindowVisible(m_hHistoryCombo))
    {
        // Get history window rect in client coordinates
        RECT historyRect;
        GetWindowRect(m_hHistoryCombo, &historyRect);
        ScreenToClient(m_hWnd, (POINT*)&historyRect.left);
        ScreenToClient(m_hWnd, (POINT*)&historyRect.right);
        
        // Get history button rect in client coordinates  
        RECT buttonRect;
        GetWindowRect(m_hHistoryButton, &buttonRect);
        ScreenToClient(m_hWnd, (POINT*)&buttonRect.left);
        ScreenToClient(m_hWnd, (POINT*)&buttonRect.right);
        
        POINT clickPoint = {x, y};
        
        if (!PtInRect(&historyRect, clickPoint) && !PtInRect(&buttonRect, clickPoint))
        {
            // Close history window
            ShowWindow(m_hHistoryCombo, SW_HIDE);
            ShowWindow(m_hResultEdit, SW_SHOW);
            SetFocus(m_hExpressionEdit);
            UpdateLayout();
        }
    }
}

void WinApiCalc::UpdateMenuChecks()
{
    if (!m_hMenu) return;

    // Update checkable menu items
    CheckMenuItem(m_hMenu, ID_CALC_PASSTYLE, (m_options & PAS) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_CALC_CASESENSETIVE, (m_options & UPCASE) ? MF_UNCHECKED : MF_CHECKED);  // Инверсия: UPCASE=case insensitive
    CheckMenuItem(m_hMenu, ID_CALC_FORCEDFLOAT, (m_options & FFLOAT) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_CALC_ESCMINIMIZED, (m_options & MIN) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_CALC_ALWAYSONTOP, (m_options & TOP) ? MF_CHECKED : MF_UNCHECKED);
    // Menu visibility is controlled via the pseudo-function menu(0) -> MenuFunction()
    // and persisted in m_options (MNU). There is no separate menu item for this.

    // Format menu items
    CheckMenuItem(m_hMenu, ID_FORMAT_SCIENTIFIC, (m_options & SCI) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_NORMALIZED, (m_options & NRM) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_FRACTION, (m_options & FRC) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_COMPUTING, (m_options & CMP) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_INTEGER, (m_options & IGR) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_UNSIGNED, (m_options & UNS) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_HEX, (m_options & HEX) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_OCTAL, (m_options & OCT) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_BINARY, (m_options & FBIN) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_CHAR, (m_options & CHR) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_WIDECHAR, (m_options & WCH) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_DATETIME, (m_options & DAT) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_UNIXTIME, (m_options & UTM) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_DEGREESE, (m_options & DEG) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_STRING, (m_options & STR) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_INCH, (m_options & FRI) ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(m_hMenu, ID_FORMAT_AUTO, (m_options & AUT) ? MF_CHECKED : MF_UNCHECKED);

    // Binary width radio buttons
    int radioIndex;
    switch (m_binWidth) {
        case 8: radioIndex = 0; break;
        case 16: radioIndex = 1; break;
        case 24: radioIndex = 2; break;
        case 32: radioIndex = 3; break;
        case 48: radioIndex = 4; break;
        case 64: radioIndex = 5; break;
        default: radioIndex = 5; break; // default to 64
    }
    CheckMenuRadioItem(m_hMenu, ID_BINARYWIDTH_8, ID_BINARYWIDTH_64, 
        ID_BINARYWIDTH_8 + radioIndex, MF_BYCOMMAND);
}

void WinApiCalc::ToggleOption(int flag)
{
    m_options ^= flag;
    // Special handling for some UI-related flags must be applied before layout updates
    if (flag == TOP) {
        // Apply always-on-top immediately
        SetWindowPos(m_hWnd, (m_options & TOP) ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else if (flag == MNU) {
        // Show or hide menu and persist the option BEFORE resizing so AdjustWindowRect
        // uses correct m_menuVisible value.
        SetMenuVisibilityOption((m_options & MNU) ? false : true);
    }

    UpdateMenuChecks();
    EvaluateExpression();
    ResizeWindow(); // Пересчитываем размер при изменении опций
}

void WinApiCalc::SetBinaryWidth(int width)
{
    m_binWidth = width;
    UpdateMenuChecks();
    EvaluateExpression(); // Пересчитываем результат при изменении ширины
    ResizeWindow(); // Пересчитываем размер при изменении ширины bin
}

void WinApiCalc::UpdateLayout()
{
    DebugLog("=== UpdateLayout() called ===");
    
    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    
    int width = clientRect.right - clientRect.left;
    int controlWidth = width; // No margins
    
    // Expression edit (with space for button)
    if (m_hExpressionEdit)
    {
        int inputHeight = GetControlHeight(); // Safe to use always
        char debugMsg[200];
        sprintf_s(debugMsg, "UpdateLayout: Expression edit - inputHeight=%d", inputHeight);
        DebugLog(debugMsg);
        
        SetWindowPos(m_hExpressionEdit, nullptr,
            0, 0, // No margins
            controlWidth - ScaleDPI(30), inputHeight,
            SWP_NOZORDER);
    }
    
    // History button
    if (m_hHistoryButton)
    {
        int buttonHeight = GetControlHeight(); // Safe to use always
        
        // Scale button width based on font size when font is ready
        int buttonWidth = ScaleDPI(23); // Default width
        if (m_fontSize != 0) {
            // Scale button width based on font size for better proportions
            int fontSizeAbs = abs(m_fontSize);
            buttonWidth = max(ScaleDPI(15), ScaleDPI(23 * fontSizeAbs / 12)); // Scale relative to 12px default
            if (buttonWidth > ScaleDPI(40)) buttonWidth = ScaleDPI(40); // Max width limit
        }
        
        SetWindowPos(m_hHistoryButton, nullptr,
            width - buttonWidth - 2, 0, // Adjust position for scaled width
            buttonWidth, buttonHeight,
            SWP_NOZORDER);
    }
    
    BOOL historyVisible = IsWindowVisible(m_hHistoryCombo);
    
    // Result edit - show only if history is not visible
    if (m_hResultEdit)
    {
        if (!historyVisible)
        {
            // Position and size result edit (history hidden)
            int rowHeight = GetControlHeight();
            int baseFontSize = 16;
            int currentFontSize = abs(m_fontSize);
            if (m_fontSize == 0) currentFontSize = baseFontSize;
            int gap = ScaleDPI(max(2, (currentFontSize * 4) / baseFontSize));
            int yPos = rowHeight + gap;
            int padding = ScaleDPI(4);
            int resultHeight = rowHeight * m_resultLines + padding;

            SetWindowPos(m_hResultEdit, nullptr,
                0, yPos,
                controlWidth, resultHeight,
                SWP_NOZORDER | SWP_SHOWWINDOW);

            // Ensure history is hidden when not in use
            if (IsWindowVisible(m_hHistoryCombo))
                ShowWindow(m_hHistoryCombo, SW_HIDE);

// (end of not-history-visible branch)

            // Measure and store actual client height of result edit to avoid mismatch
            RECT wr={0}, cr={0};
            if (m_hResultEdit) {
                GetWindowRect(m_hResultEdit, &wr);
                GetClientRect(m_hResultEdit, &cr);
                m_lastResultClientHeight = cr.bottom - cr.top;
                // Compute internal padding = measured client height - (rowHeight * reportedLines)
                int reportedLinesLocal = (int)SendMessageA(m_hResultEdit, EM_GETLINECOUNT, 0, 0);
                if (reportedLinesLocal <= 0) reportedLinesLocal = m_resultLines;
                int idealHeight = GetControlHeight() * reportedLinesLocal;
                m_resultEditInternalPadding = m_lastResultClientHeight - idealHeight;
                if (m_resultEditInternalPadding < 0) m_resultEditInternalPadding = 0;
                // horizontal padding = control width - client width
                int winW = wr.right - wr.left;
                int cliW = cr.right - cr.left;
                m_resultEditInternalHorzPadding = max(0, winW - cliW);
            }
            // Log the freshly measured client height for debugging
            {
                char mlog[128];
                sprintf_s(mlog, sizeof(mlog), "UpdateLayout: measured m_lastResultClientHeight=%d", m_lastResultClientHeight);
                DebugLog(mlog);
            }

#if ENABLE_DEBUG_LOG
            // Log result edit metrics: window/client rect and reported line count
            {
                RECT wr={0}, cr={0};
                if (m_hResultEdit) {
                    GetWindowRect(m_hResultEdit, &wr);
                    GetClientRect(m_hResultEdit, &cr);
                }
                int reportedLines = 0;
                if (m_hResultEdit) reportedLines = (int)SendMessageA(m_hResultEdit, EM_GETLINECOUNT, 0, 0);
                char rdbg[256];
                sprintf_s(rdbg, sizeof(rdbg), "UpdateLayout(DEBUG): ResultEdit WindowRect=(%d,%d)-(%d,%d) ClientRect=(%d,%d)-(%d,%d) yPos=%d resultHeight=%d reportedLines=%d m_resultLines=%d", 
                    wr.left, wr.top, wr.right, wr.bottom, cr.left, cr.top, cr.right, cr.bottom, yPos, resultHeight, reportedLines, m_resultLines);
                DebugLog(rdbg);
            }
#endif
        }
        else
        {
            // Hide result field when history is visible
            //ShowWindow(m_hResultEdit, SW_HIDE);
            // Keep result field visible; history popup will be shown on top.
            // Avoid hiding m_hResultEdit to prevent clearing the client area.

        }
    }
    
    // History listbox - show in place of result field when visible
    if (m_hHistoryCombo && historyVisible)
    {
        // Compute popup size: min 5 visible lines, cap to avoid overly tall popup
        int rowHeight = GetControlHeight();
        int baseFontSize = 16;
        int currentFontSize = abs(m_fontSize);
        if (m_fontSize == 0) currentFontSize = baseFontSize;
        int gap = ScaleDPI(max(2, (currentFontSize * 4) / baseFontSize));
        int yPos = rowHeight + gap;

        const int minVisibleLines = 5;
        const int maxVisibleLines = 7; // cap for popup height
        int requestedLines = max(m_resultLines, minVisibleLines);
        int visibleLines = min(requestedLines, maxVisibleLines);
        int padding = ScaleDPI(4);
        int historyHeight = rowHeight * visibleLines + padding;

        // Position the popup in screen coords and show it
        POINT pt = { 0, yPos };
        ClientToScreen(m_hWnd, &pt);
        int popupWidth = controlWidth;
        SetWindowPos(m_hHistoryCombo, HWND_TOP,
            pt.x, pt.y,
            popupWidth, historyHeight,
            SWP_SHOWWINDOW);

        // Ensure history has focus and selection
        SetFocus(m_hHistoryCombo);
        if (SendMessage(m_hHistoryCombo, LB_GETCURSEL, 0, 0) == LB_ERR)
            SendMessage(m_hHistoryCombo, LB_SETCURSEL, 0, 0);
    }        
#if ENABLE_DEBUG_LOG
        // Log history control metrics for debugging (inside correct scope where yPos/displayLines are defined)
        {
            RECT wr={0}, cr={0};
            if (m_hHistoryCombo) {
                GetWindowRect(m_hHistoryCombo, &wr);
                GetClientRect(m_hHistoryCombo, &cr);
            }
            int itemCount = 0;
            if (m_hHistoryCombo) itemCount = (int)SendMessageA(m_hHistoryCombo, LB_GETCOUNT, 0, 0);
            char hdbg[256];
            sprintf_s(hdbg, sizeof(hdbg), "UpdateLayout(DEBUG): HistoryCombo WindowRect=(%d,%d)-(%d,%d) ClientRect=(%d,%d)-(%d,%d) yPos=%d historyHeight=%d displayLines=%d itemCount=%d", 
                wr.left, wr.top, wr.right, wr.bottom, cr.left, cr.top, cr.right, cr.bottom, yPos, historyHeight, displayLines, itemCount);
            DebugLog(hdbg);
        }
#endif
    
}

int WinApiCalc::GetCharWidth()
{
    HDC hdc = GetDC(m_hWnd);
    if (!hdc) return 8; // fallback
    
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    TEXTMETRICA tm;
    GetTextMetricsA(hdc, &tm);
    int charWidth = tm.tmAveCharWidth;
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(m_hWnd, hdc);
    
    return charWidth;
}

int WinApiCalc::GetCharHeight()
{
    HDC hdc = GetDC(m_hWnd);
    if (!hdc) return 16; // fallback
    
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    TEXTMETRICA tm;
    GetTextMetricsA(hdc, &tm);
    int charHeight = tm.tmHeight;
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(m_hWnd, hdc);
    
    return charHeight;
}

int WinApiCalc::GetControlHeight()
{
    // Calculate height based on font size + minimal padding for borders
    int charHeight = GetCharHeight();
    
    // Reduced padding for tighter appearance - just enough for borders
    int padding = 6; // Minimal padding for visibility (was 8)
    if (abs(m_fontSize) <= 10) { // Only very small fonts need extra padding
        padding = 8; 
    }
    
    // Add padding for top+bottom borders 
    int height = charHeight + padding;
    
    // Reduced minimum height for tighter appearance
    int minHeight = 20; // Minimum 20px height
    return max(height, minHeight);
}

int WinApiCalc::CalculateOptimalWidth(int charCount)
{
    int charWidth = GetCharWidth();
    
    // Для моноширинного шрифта протестируем реальную ширину строки из 70 символов
    HDC hdc = GetDC(m_hWnd);
    if (!hdc) return charWidth * charCount + 50; // fallback
    
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    // Тестовая строка из 70 символов (как в format_out)
    char testString[80];
    memset(testString, 'W', charCount); // 'W' - один из самых широких символов
    testString[charCount] = '\0';
    
    SIZE textSize;
    GetTextExtentPoint32A(hdc, testString, charCount, &textSize);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(m_hWnd, hdc);
    
    // Добавляем запас: реальная ширина + кнопка истории + отступы + скроллбар
    // + внутренние отступы Edit контрола (примерно 6-8 пикселей с каждой стороны)
    // Учитываем внутренние отступы Edit контрола (примерно 2 символа)
    
    // For large fonts, add extra margin to prevent text wrapping
    int extraMargin = 0;
    if (m_fontSize < 0 && m_fontSize <= -20) // Large pixel fonts
    {
        // Progressive scaling for very large fonts
        if (m_fontSize <= -25)
            extraMargin = abs(m_fontSize) * 5; // Much more space for very large fonts
        else if (m_fontSize <= -22) // Special handling for -23, -22 and down
            extraMargin = abs(m_fontSize) * 4 + 80; // Massive buffer for edge cases
        else
            extraMargin = abs(m_fontSize) * 3; // Standard extra space
    }
    
    // Add much larger buffer to prevent single character wrapping for large fonts
    int wrapBuffer = (m_fontSize <= -22) ? 60 : 15; // Much bigger buffer for large fonts
    
    return textSize.cx + 25 - (charWidth * 2) + extraMargin + wrapBuffer; // кнопка минус внутренние отступы плюс доп. отступ плюс буфер против переноса
}

void WinApiCalc::UpdateFont()
{
    // Удаляем старый шрифт если он есть
    HFONT hCurrentFont = (HFONT)SendMessage(m_hExpressionEdit, WM_GETFONT, 0, 0);
    if (hCurrentFont) DeleteObject(hCurrentFont);
    
    // Создаем новый шрифт Courier New
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

    if (hFont)
    {
        SendMessage(m_hExpressionEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(m_hResultEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(m_hHistoryCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        // Update history button font too
        if (m_hHistoryButton)
        {
            SendMessage(m_hHistoryButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
    }
    
    // Пересчитываем размер окна с новым шрифтом
    ResizeWindow();
}

int WinApiCalc::CalculateContentBasedWidth()
{
    if (!m_pCalculator) return CalculateOptimalWidth(76); // fallback
    
    // Создаем DC и настраиваем шрифт (как TBitmap в VCL)
    HDC hdc = GetDC(m_hWnd);
    if (!hdc) return CalculateOptimalWidth(76);
    
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
   
    int maxPixelWidth = 0;
    
    {
        //  измеряем тестовую строку
        char fallbackStr[80];
        memset(fallbackStr, 'W', 76);
        fallbackStr[76] = '\0';
        
        SIZE textSize;
        if (GetTextExtentPoint32A(hdc, fallbackStr, 76, &textSize))
            maxPixelWidth = textSize.cx;
    }
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(m_hWnd, hdc);

    // Если ничего не получили, используем fallback
    if (maxPixelWidth == 0)
        return CalculateOptimalWidth(76);
    
    // Добавляем места для кнопки истории (25px) + отступы + запас на рамку
    // + внутренние отступы Edit контрола (примерно 6-8 пикселей с каждой стороны)
    
    // Base calculation with 6% margin for measurement inaccuracies
    int baseWidth = maxPixelWidth + (maxPixelWidth * 6 / 100) + 50;
    
    // Add scaling factor for large fonts to prevent text wrapping
    if (m_fontSize < 0 && m_fontSize <= -20) // Large pixel fonts
    {
        // Use minimal percentage-based scaling for large fonts
        double scaleFactor = 1.0;
        if (m_fontSize <= -25)
            scaleFactor = 1.03; // 3% extra width for very large fonts
        else if (m_fontSize <= -22) // Special handling for -23, -22 and down
            scaleFactor = 1.02; // 2% extra width for problematic sizes
        else
            scaleFactor = 1.01; // 1% extra width for standard large fonts
        
        baseWidth = (int)(baseWidth * scaleFactor);
    }
    
    return baseWidth;
}

int WinApiCalc::CalculateCurrentResultWidth()
{
    if (!m_hResultEdit) return CalculateOptimalWidth(76);
    
    // Получаем текущий текст из поля результата
    int textLen = GetWindowTextLengthA(m_hResultEdit);
    if (textLen == 0) return CalculateOptimalWidth(76);
    
    char* buffer = new char[textLen + 1];
    GetWindowTextA(m_hResultEdit, buffer, textLen + 1);
    
    // Создаем DC и настраиваем шрифт
    HDC hdc = GetDC(m_hWnd);
    if (!hdc) 
    {
        delete[] buffer;
        return CalculateOptimalWidth(76);
    }
    
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    int maxPixelWidth = 0;
    
    // Разбиваем текст на строки и измеряем каждую
    char* context = nullptr;
    char* line = strtok_s(buffer, "\r\n", &context);
    while (line)
    {
        SIZE textSize;
        int len = (int)strlen(line);
        if (GetTextExtentPoint32A(hdc, line, len, &textSize))
        {
            if (textSize.cx > maxPixelWidth)
                maxPixelWidth = textSize.cx;
        }
        line = strtok_s(nullptr, "\r\n", &context);
    }
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(m_hWnd, hdc);
    delete[] buffer;
    
    // Если ничего не получили, используем fallback
    if (maxPixelWidth == 0)
        return CalculateOptimalWidth(76);
    
    // Добавляем места для кнопки истории (25px) + отступы + запас на рамку  
    // + внутренние отступы Edit контрола (примерно 6-8 пикселей с каждой стороны)
    
    // For large fonts, add proportional margin like in CalculateOptimalWidth
    int baseWidth = maxPixelWidth + 25 - 16; // Basic calculation: button minus internal padding
    
    // Add scaling factor for large fonts to prevent text wrapping
    if (m_fontSize < 0 && m_fontSize <= -20) // Large pixel fonts
    {
        // Use minimal percentage-based scaling for large fonts
        double scaleFactor = 1.0;
        if (m_fontSize <= -25)
            scaleFactor = 1.03; // 3% extra width for very large fonts
        else if (m_fontSize <= -22) // Special handling for -23, -22 and down
            scaleFactor = 1.02; // 2% extra width for problematic sizes
        else
            scaleFactor = 1.01; // 1% extra width for standard large fonts
        
        baseWidth = (int)(baseWidth * scaleFactor);
    }
    
    return baseWidth;
}

void WinApiCalc::OnCtrlR() 
{
    WrapExpressionWith("root2(", ")");
}

void WinApiCalc::OnCtrlS() 
{
    // Проверяем PAS style для выбора синтаксиса возведения в степень
    if (m_options & PAS)
    {
        // PAS style включен - используем Pascal математический синтаксис ^
        WrapExpressionWith("(", ")^2");
    }
    else
    {
        // C-style синтаксис **
        WrapExpressionWith("(", ")**2");
    }
}

void WinApiCalc::OnCtrlI() 
{
    WrapExpressionWith("1/(", ")");
}

void WinApiCalc::OnCtrlLBracket() 
{
    WrapExpressionWith("(", ")");
}

void WinApiCalc::OnCtrlRBracket() 
{
    WrapExpressionWith("(", ")");
}

void WinApiCalc::OnCtrlHome() 
{
    // Home position
    SetWindowPos(m_hWnd, nullptr, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    // Reset font to default
    SetFontSize(-12);
    
    // Reset opacity to fully opaque (255 = 100%)
    SetWindowOpacity(255);
    
    UpdateMenuChecks();
}

void WinApiCalc::OnCtrlPlus() 
{
    // Increase font size in pixels (larger visual size, smaller absolute value for negative numbers)
    // -12 -> -11 (bigger visual font)
    if (m_fontSize < -8) // Don't go smaller than 8 pixels in absolute value
    {
        SetFontSize(m_fontSize + 1); // More positive = bigger font visually
    }
}

void WinApiCalc::OnCtrlMinus() 
{
    // Decrease font size in pixels (smaller visual size, larger absolute value for negative numbers)  
    // -12 -> -13 (smaller visual font)
    if (m_fontSize > -50) // Don't go larger than 50 pixels in absolute value
    {
        SetFontSize(m_fontSize - 1); // More negative = smaller font visually
    }
}

// Window procedure for Variables dialog
static LRESULT CALLBACK VariablesDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:  // RichEdit may use this instead
        {
            // Set white background only for our edit control
            if ((HWND)lParam == g_hVariablesEdit) {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, RGB(0, 0, 0)); // Black text
                SetBkColor(hdc, RGB(255, 255, 255)); // White background
                return (LRESULT)GetStockObject(WHITE_BRUSH);
            }
            break;
        }
    
    case WM_CLOSE:
        DestroyWindow(hWnd);  // Use DestroyWindow instead of EndDialog
        return 0;
    
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hWnd);  // Use DestroyWindow instead of EndDialog
            return 0;
        }
        break;
    
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            DestroyWindow(hWnd);  // Use DestroyWindow instead of EndDialog
            return 0;
        }
        break;
    
    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK) {
            DestroyWindow(hWnd);  // Use DestroyWindow instead of EndDialog
            return 0;
        }
        break;
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Static buffer for collecting variables
static char* g_varBuffer = nullptr;
static size_t g_varBufferLen = 0;

// Static callback function for varlist
static void AddVarToBuffer(char* varName, float__t value) {
    if (g_varBuffer && g_varBufferLen > 0) {
        char line[200];
        sprintf_s(line, "%s = %.15g\r\n", varName, (double)value);
        
        // Check if there's enough space
        if (strlen(g_varBuffer) + strlen(line) < g_varBufferLen - 1) {
            strcat_s(g_varBuffer, g_varBufferLen, line);
        }
    }
}

void WinApiCalc::ShowVariablesDialog() 
{
    // Register a temporary window class for the dialog
    static bool classRegistered = false;
    const char* className = "VariablesDialog";
    
    if (!classRegistered) {
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.lpfnWndProc = VariablesDialogProc;
        wc.hInstance = m_hInst;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = className;
        wc.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SMALL));
        wc.hIconSm = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SMALL));
        
        if (RegisterClassExA(&wc)) {
            classRegistered = true;
        } else {
            return; // Failed to register class
        }
    }
    
    // Create modal dialog for viewing variables
    RECT rect;
    GetWindowRect(m_hWnd, &rect);
    
    // Create content text buffer (4KB should be enough for variable list)
    char textBuffer[4096] = "";
    int variableCount = 0;
    int maxlen = 0;
    // Get variable list and maxlen from calculator engine
    if (m_pCalculator) {
        variableCount = m_pCalculator->varlist(textBuffer, sizeof(textBuffer) - 1, &maxlen);
        if (variableCount == 0) {
            strcpy_s(textBuffer, "No variables available.");
            variableCount = 1;
            maxlen = (int)strlen(textBuffer);
        }
    } else {
        strcpy_s(textBuffer, "Calculator engine not available.");
        variableCount = 1;
        maxlen = (int)strlen(textBuffer);
    }

    // Calculate width in pixels for maxlen characters in current font
    HFONT hFontMeasure = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    HDC hdc = GetDC(m_hWnd);
    HFONT hOldFont = hFontMeasure ? (HFONT)SelectObject(hdc, hFontMeasure) : nullptr;
    SIZE sz = {0,0};
    // Формируем строку из maxlen символов 'X' (ширина максимальная для моноширинного)
    char testStr[256];
    int testLen = (maxlen < 255) ? maxlen : 255;
    for (int i = 0; i < testLen; ++i) testStr[i] = 'X';
    testStr[testLen] = '\0';
    GetTextExtentPoint32A(hdc, testStr, testLen, &sz);
    int dialogWidth = sz.cx + 50; // 50px for scrollbar and padding
    if (hOldFont) SelectObject(hdc, hOldFont);
    if (hFontMeasure) DeleteObject(hFontMeasure);
    ReleaseDC(m_hWnd, hdc);

    int charHeight = GetCharHeight();
    int displayLines = max(8, min(variableCount, 15));
    int dialogHeight = charHeight * displayLines + 50;
    
    // Create modal window at calculator's position (0:0 relative)
    HWND hDialog = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        className,
        "Variables",
        WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
        rect.left, rect.top,  // Position at calculator's top-left (0:0)
        dialogWidth, dialogHeight,
        m_hWnd,     // Parent window
        nullptr,    // No menu
        m_hInst,
        nullptr
    );
    
    if (!hDialog) return;
    
    // Always create edit control with vertical scrollbar, limit height to max 15 lines
    int maxLines = 15;
    int editLines = (variableCount > maxLines) ? maxLines : displayLines;
    // Добавим +18px к ширине edit-контрола для скроллера
    // Edit control шириной на 6px меньше окна, сдвинут на 3px вправо
    int editX = 3;
    int editW = dialogWidth - 20;
    HWND hMemoEdit = CreateWindowExA(
        0, // No extra styles
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER,
        editX, 0,
        editW, (charHeight * editLines) + 8,
        hDialog,
        nullptr,
        m_hInst,
        nullptr
    );
    
    if (!hMemoEdit) {
        DestroyWindow(hDialog);
        return;
    }
    
    // Store edit handle for color processing
    g_hVariablesEdit = hMemoEdit;
    
    // Set the font to match calculator's font (new object for edit control)
    HFONT hFont = CreateFontA(
        ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    if (hFont) {
        SendMessage(hMemoEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    
    // Set text in the memo control
    SetWindowTextA(hMemoEdit, textBuffer);
    
    // Show dialog modally using DialogBox-style approach
    EnableWindow(m_hWnd, FALSE);
    SetFocus(hDialog);
    
    // Simple message loop for modal behavior
    MSG msg;
    bool dialogActive = true;
    
    while (dialogActive && IsWindow(hDialog)) {
        BOOL bRet = GetMessage(&msg, nullptr, 0, 0);
        if (bRet == 0 || bRet == -1) break; // WM_QUIT or error
        
        // Check if our dialog was destroyed
        if (!IsWindow(hDialog)) {
            dialogActive = false;
            break;
        }
        
        if (msg.message == WM_QUIT) {
            PostQuitMessage((int)msg.wParam);
            dialogActive = false;
            break;
        }
        
        // Handle messages for dialog and its children
        if (msg.hwnd == hDialog || IsChild(hDialog, msg.hwnd)) {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                DestroyWindow(hDialog);
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (msg.hwnd == m_hWnd && msg.message == WM_CLOSE) {
            // Main window close - exit dialog
            DestroyWindow(hDialog);
            break;
        }
        else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // Clean up
    g_hVariablesEdit = nullptr;  // Clear static variable
    EnableWindow(m_hWnd, TRUE);
    SetFocus(m_hWnd);
    if (IsWindow(hDialog)) {
        DestroyWindow(hDialog);
    }
    if (hFont) DeleteObject(hFont);
}
void WinApiCalc::LoadHistory() 
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\WinApiCalc", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        m_history.clear();
        
        // Load history count first
        DWORD historyCount = 0;
        DWORD size = sizeof(historyCount);
        if (RegQueryValueExA(hKey, "HistoryCount", nullptr, nullptr, (LPBYTE)&historyCount, &size) == ERROR_SUCCESS)
        {
            // Load history items in DIRECT order - History0=newest, History1=older, etc.
            // Registry already stores in correct order: History0=newest
            // Vector needs: [0]=newest ... [N]=oldest
            for (int i = 0; i < (int)historyCount && i < (int)MAX_HISTORY; i++)
            {
                char szValueName[256];
                sprintf_s(szValueName, "History%d", i);
                
                char szData[1024];
                DWORD dwDataSize = sizeof(szData);
                if (RegQueryValueExA(hKey, szValueName, nullptr, nullptr, (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS)
                {
                    m_history.push_back(std::string(szData));
                }
            }
        }
        
        RegCloseKey(hKey);
    }
    
    UpdateHistoryCombo();
}

void WinApiCalc::SaveHistory() 
{
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\WinApiCalc", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
    {
        // Save history count
        DWORD historyCount = (DWORD)m_history.size();
        RegSetValueExA(hKey, "HistoryCount", 0, REG_DWORD, (LPBYTE)&historyCount, sizeof(historyCount));
        
        // Save history items
        for (size_t i = 0; i < m_history.size(); ++i)
        {
            char szValueName[32];
            sprintf_s(szValueName, "History%zu", i);
            RegSetValueExA(hKey, szValueName, 0, REG_SZ, (LPBYTE)m_history[i].c_str(), (DWORD)(m_history[i].length() + 1));
        }
        
        RegCloseKey(hKey);
    }
    // Apply UI flags loaded from registry
    // Ensure window handle may not be created yet; caller should call SetMenu/SetWindowPos after window creation if needed.
    if (m_hWnd) {
        SetWindowPos(m_hWnd, (m_options & TOP) ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetMenu(m_hWnd, (m_options & MNU) ? NULL : m_hMenu);
        DrawMenuBar(m_hWnd);
    }
}

void WinApiCalc::AddToHistory(const std::string& expression) 
{
    if (expression.empty()) return;
    
    // Check if expression consists only of whitespace
    bool onlyWhitespace = true;
    for (char c : expression)
    {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
        {
            onlyWhitespace = false;
            break;
        }
    }
    if (onlyWhitespace) return;
    
    // Check if expression already exists in history
    for (const auto& item : m_history)
    {
        if (item == expression)
        {
            return; // Do nothing if already exists
        }
    }
    
    // Shift all items by one position (simple array behavior)
    if (m_history.size() < MAX_HISTORY)
    {
        // If less than 100 items, just insert at beginning
        m_history.insert(m_history.begin(), expression);
    }
    else
    {
        // If 100 items, shift all items: last gets removed, others move by 1
        for (int i = MAX_HISTORY - 1; i > 0; i--)
        {
            m_history[i] = m_history[i-1];
        }
        m_history[0] = expression; // Put new item at position 0
    }
    
    UpdateHistoryCombo();
    // Убираем SaveHistory() - будем сохранять только при выходе
}

void WinApiCalc::LoadHistoryItem(int index) 
{
    // Проверяем валидность индекса
    if (index < 0 || index >= (int)m_history.size() || m_history.empty())
    {
        return;
    }
    
    // Direct mapping: listbox index = vector index (0 = newest item)
    const std::string& expression = m_history[index];
    
    // Проверяем, что выражение не пустое
    if (expression.empty())
    {
        return;
    }
    
    // Устанавливаем выражение в поле ввода
    SetWindowTextA(m_hExpressionEdit, expression.c_str());
    
    // Обновляем текущее выражение
    m_currentExpression = expression;
    
    // Set cursor to end of text without selection
    SendMessage(m_hExpressionEdit, EM_SETSEL, expression.length(), expression.length());
}

void WinApiCalc::DeleteSelectedHistoryItem()
{
    if (!m_hHistoryCombo) return;
    
    int sel = (int)SendMessage(m_hHistoryCombo, LB_GETCURSEL, 0, 0);
    if (sel != LB_ERR && sel >= 0 && sel < (int)m_history.size())
    {
        // Direct mapping: listbox index = vector index
        // Remove from vector - this automatically "lifts up" the list
        m_history.erase(m_history.begin() + sel);
        
        // Update listbox completely
        UpdateHistoryCombo();
        
        // If history is empty, close history window
        if (m_history.empty())
        {
            ShowWindow(m_hHistoryCombo, SW_HIDE);
            ShowWindow(m_hResultEdit, SW_SHOW);
            SetFocus(m_hExpressionEdit);
            UpdateLayout();
            return;
        }
        
        // Set selection to same position if possible, otherwise select previous item
        int newSel = sel;
        if (newSel >= (int)m_history.size() && !m_history.empty())
        {
            newSel = (int)m_history.size() - 1; // Select last item
        }
        
        // Set selection in listbox
        if (newSel >= 0 && newSel < (int)m_history.size())
        {
            SendMessage(m_hHistoryCombo, LB_SETCURSEL, newSel, 0);
        }
        else if (!m_history.empty())
        {
            // If calculation above failed, just select first item
            SendMessage(m_hHistoryCombo, LB_SETCURSEL, 0, 0);
        }
        
        // Ensure focus stays on history listbox
        SetFocus(m_hHistoryCombo);
        
        // Убираем SaveHistory() - будем сохранять только при выходе
    }
}

void WinApiCalc::LoadSettings() 
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\WinApiCalc", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwOptions = (FFLOAT + SCF + NRM + CMP + IGR + UNS + HEX + CHR + WCH + OCT + fBIN + DAT + DEG + STR + ALL + MNU + FRC + FRI);
        DWORD dwSize = sizeof(dwOptions);
		dwOptions = 0x0bcf7f37; // Default options if not found
        if (RegQueryValueExA(hKey, "Options", nullptr, nullptr, (LPBYTE)&dwOptions, &dwSize) == ERROR_SUCCESS)
        {
            m_options = dwOptions;
        }
        
        DWORD dwBinWidth = 32;
        dwSize = sizeof(dwBinWidth);
        if (RegQueryValueExA(hKey, "BinaryWidth", nullptr, nullptr, (LPBYTE)&dwBinWidth, &dwSize) == ERROR_SUCCESS)
        {
            m_binWidth = dwBinWidth;
        }
        
        DWORD dwFontSize = 14;
        dwSize = sizeof(dwFontSize);
        if (RegQueryValueExA(hKey, "FontSize", nullptr, nullptr, (LPBYTE)&dwFontSize, &dwSize) == ERROR_SUCCESS)
        {
            m_fontSize = dwFontSize;
        }
        
        DWORD dwOpacity = 200;
        dwSize = sizeof(dwOpacity);
        if (RegQueryValueExA(hKey, "Opacity", nullptr, nullptr, (LPBYTE)&dwOpacity, &dwSize) == ERROR_SUCCESS)
        {
            m_opacity = dwOpacity;
        }
        
        DWORD dwWindowX = 100;
        dwSize = sizeof(dwWindowX);
        if (RegQueryValueExA(hKey, "WindowX", nullptr, nullptr, (LPBYTE)&dwWindowX, &dwSize) == ERROR_SUCCESS)
        {
            m_windowX = dwWindowX;
        }
        
        DWORD dwWindowY = 100;
        dwSize = sizeof(dwWindowY);
        if (RegQueryValueExA(hKey, "WindowY", nullptr, nullptr, (LPBYTE)&dwWindowY, &dwSize) == ERROR_SUCCESS)
        {
            m_windowY = dwWindowY;
        }
        
        // Load current expression
        char exprBuffer[2048];
        DWORD exprSize = sizeof(exprBuffer);
        if (RegQueryValueExA(hKey, "CurrentExpression", nullptr, nullptr, (LPBYTE)exprBuffer, &exprSize) == ERROR_SUCCESS)
        {
            m_currentExpression = std::string(exprBuffer);
        }
        
        RegCloseKey(hKey);
    }
}

void WinApiCalc::SaveSettings() 
{
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\WinApiCalc", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
    {
        RegSetValueExA(hKey, "Options", 0, REG_DWORD, (LPBYTE)&m_options, sizeof(m_options));
        RegSetValueExA(hKey, "BinaryWidth", 0, REG_DWORD, (LPBYTE)&m_binWidth, sizeof(m_binWidth));
        RegSetValueExA(hKey, "FontSize", 0, REG_DWORD, (LPBYTE)&m_fontSize, sizeof(m_fontSize));
        RegSetValueExA(hKey, "Opacity", 0, REG_DWORD, (LPBYTE)&m_opacity, sizeof(m_opacity));
        RegSetValueExA(hKey, "WindowX", 0, REG_DWORD, (LPBYTE)&m_windowX, sizeof(m_windowX));
        RegSetValueExA(hKey, "WindowY", 0, REG_DWORD, (LPBYTE)&m_windowY, sizeof(m_windowY));
        
        // Save current expression (always, even if empty)
        RegSetValueExA(hKey, "CurrentExpression", 0, REG_SZ, (LPBYTE)m_currentExpression.c_str(), (DWORD)(m_currentExpression.length() + 1));
        
        RegCloseKey(hKey);
    }
}
void WinApiCalc::InitializeDPI() { HDC hdc = GetDC(nullptr); if (hdc) { m_dpiX = GetDeviceCaps(hdc, LOGPIXELSX); m_dpiY = GetDeviceCaps(hdc, LOGPIXELSY); ReleaseDC(nullptr, hdc); }}
int WinApiCalc::ScaleDPI(int value) { return MulDiv(value, m_dpiX, 96); }

void WinApiCalc::Initialize(HWND hwnd)
{
    m_hWnd = hwnd;
    m_hInst = GetModuleHandleA(nullptr);  // Get application instance
    LoadSettings();  // Load settings BEFORE OnCreate so m_currentExpression is available
    LoadHistory();
    OnCreate();
    UpdateMenuChecks();
    UpdateLayout();
}

void WinApiCalc::UpdateHistoryCombo()
{
    if (!m_hHistoryCombo) return;
    
    // Clear listbox
    SendMessageA(m_hHistoryCombo, LB_RESETCONTENT, 0, 0);
    
    // Add history items in direct order: vector[0] = newest = shows first in listbox
    // Since vector is already ordered with newest at index 0, just add them sequentially
    for (size_t i = 0; i < m_history.size(); ++i)
    {
        SendMessageA(m_hHistoryCombo, LB_ADDSTRING, 0, (LPARAM)m_history[i].c_str());
    }
}

void WinApiCalc::ResizeWindow() 
{
    if (!m_hWnd) return; // Защита от вызова до инициализации окна
    
    // Рассчитываем ширину на основе текущего результата, если он есть, иначе на основе настроек
    int clientWidth;
    try
    {
        if (m_hResultEdit && GetWindowTextLengthA(m_hResultEdit) > 0)
        {
            clientWidth = CalculateCurrentResultWidth(); // Используем реальный результат
        }
        else
        {
            clientWidth = CalculateContentBasedWidth(); // Используем тестовые данные
        }
    }
    catch (...)
    {
        // Fallback на минимальную ширину в случае ошибки
        clientWidth = ScaleDPI(WINDOW_MIN_WIDTH);
    }
    
    // Use control-based row height so ResizeWindow and UpdateLayout agree on
    // how tall each result row is (avoids mismatch between GetCharHeight and control height).
    int rowHeight = GetControlHeight();
    int inputHeight = rowHeight; // Use safe GetControlHeight
    
    // Проверяем, показывается ли история - для истории нужно минимум 5 строк
    BOOL historyVisible = m_hHistoryCombo && IsWindowVisible(m_hHistoryCombo);
    int displayLines;
    
    if (historyVisible) {
        displayLines = max(m_resultLines, 5); // Минимум 5 строк для истории
    } else {
        displayLines = m_resultLines;
    }
    
    int resultHeight = rowHeight * displayLines;
    
    // Add adaptive gap between input and result fields based on font size
    int baseFontSize = 16; // Default font size
    int currentFontSize = abs(m_fontSize);
    if (m_fontSize == 0) currentFontSize = baseFontSize;
    
    // Scale gap with font size: smaller gap for smaller fonts, larger for bigger
    int gap = ScaleDPI(max(2, (currentFontSize * 4) / baseFontSize)); // 2-6px range
    
    // Адаптивный запас: уменьшённый и более предсказуемый padding для
    // малых чисел строк, чтобы не оставлять лишнюю пустую строку в результате.
    // Для больших результатов оставляем расширенный запас для защиты от обрезания.
    // Use a small, scaled padding instead of large multi-row padding to avoid
    // visible blank lines under the last result line. Scrollbars handle overflow.
    int padding = ScaleDPI(4);
    // Two-pass resize strategy:
    // 1) Apply the desired client width with a temporary height so controls can
    //    reflow and we can measure the actual result edit client height (m_lastResultClientHeight).
    // 2) Recompute final client height using the measured control height and apply final window size.

    // Temporary client height: keep current client height if available, otherwise minimal
    RECT curCr = {0};
    int tempClientHeight = inputHeight + gap + rowHeight; // minimal reasonable temp height
    if (m_hWnd) {
        RECT cur; GetClientRect(m_hWnd, &cur); int ch = cur.bottom - cur.top; if (ch > 0) tempClientHeight = ch;
    }

    RECT tempRect = { 0, 0, clientWidth, tempClientHeight };
    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    style |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    // Apply temp size to allow UpdateLayout to measure control client size with the correct width
    AdjustWindowRect(&tempRect, style, m_menuVisible ? TRUE : FALSE);
    SetWindowPos(m_hWnd, nullptr, 0, 0, tempRect.right - tempRect.left, tempRect.bottom - tempRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Force layout to recalculate and measure the result edit client height
    UpdateLayout();

    // Debug: log whether we have a measured result client height after UpdateLayout
    {
        char mdbg[256];
        sprintf_s(mdbg, sizeof(mdbg), "ResizeWindow: after temp UpdateLayout m_lastResultClientHeight=%d (displayLines=%d)", m_lastResultClientHeight, displayLines);
        DebugLog(mdbg);
    }

    // Now compute final clientHeight using measured m_lastResultClientHeight if we have it
    // Compute desired result client height from the edit control's reported
    // line count (EM_GETLINECOUNT). This accounts for wrapped lines so the
    // visible lines match the height we allocate.
    int reportedLines = displayLines;
    if (m_hResultEdit) {
        reportedLines = (int)SendMessageA(m_hResultEdit, EM_GETLINECOUNT, 0, 0);
        if (reportedLines <= 0) reportedLines = displayLines;
    }

    // Adjust computed height by subtracting the measured internal padding so the
    // visible text region aligns exactly to the requested number of rows.
    // As a final attempt, measure the actual text height using DrawText wrapping
    // to the computed client width. This is the most reliable way to get the
    // rendered height that will be used inside the Edit control.
    std::string resultText;
    if (m_hResultEdit) {
        int len = GetWindowTextLengthA(m_hResultEdit);
        resultText.resize(len + 1);
        GetWindowTextA(m_hResultEdit, &resultText[0], len + 1);
        resultText.resize(len);
    }

    int measuredTextHeight = 0;
    if (!resultText.empty()) {
        int wrapWidth = clientWidth - m_resultEditInternalHorzPadding;
        if (wrapWidth < 20) wrapWidth = clientWidth; // fallback
        char dbgwrap[128]; sprintf_s(dbgwrap, sizeof(dbgwrap), "ResizeWindow: wrapWidth=%d clientWidth=%d horzPad=%d", wrapWidth, clientWidth, m_resultEditInternalHorzPadding); DebugLog(dbgwrap);
        measuredTextHeight = MeasureTextHeightForWidth(resultText, wrapWidth);
    }

    int computedResultClientHeight = measuredTextHeight > 0 ? measuredTextHeight : (rowHeight * reportedLines - m_resultEditInternalPadding + ScaleDPI(2));
    if (computedResultClientHeight < rowHeight) computedResultClientHeight = rowHeight; // at least one row
    // Log measured vs computed for diagnostics
    {
        char dbgmeas[256];
        sprintf_s(dbgmeas, sizeof(dbgmeas), "ResizeWindow: measured=%d computed=%d displayLines=%d rowHeight=%d", m_lastResultClientHeight, computedResultClientHeight, displayLines, rowHeight);
        DebugLog(dbgmeas);
    }

    int clientHeight = inputHeight + gap + computedResultClientHeight;
    
    char debugMsg[200];
    sprintf_s(debugMsg, "ResizeWindow: inputHeight=%d, gap=%d, resultHeight=%d, padding=%d, total=%d", 
              inputHeight, gap, resultHeight, padding, clientHeight);
    DebugLog(debugMsg);
    // Detailed debug: log computed parameters and menu state
#if ENABLE_DEBUG_LOG
    {
        char dbg2[512];
        RECT rcClient = {0,0,clientWidth,clientHeight};
        sprintf_s(dbg2, sizeof(dbg2), "ResizeWindow(DEBUG): clientWidth=%d, clientHeight=%d, rowHeight=%d, inputHeight=%d, displayLines=%d, gap=%d, padding=%d, menuVisible=%d, hMenu=%p", 
            clientWidth, clientHeight, rowHeight, inputHeight, displayLines, gap, padding, (int)(m_menuVisible ? 1 : 0), (void*)m_hMenu);
        DebugLog(dbg2);
    }
#endif
    
    RECT rect = { 0, 0, clientWidth, clientHeight };
    
    // Убедимся, что стиль окна правильный (без изменения размеров)
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX); // Убираем возможность ресайза
    style |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    SetWindowLong(m_hWnd, GWL_STYLE, style);
    
    // Adjust for menu presence so the non-client area (menu bar) is accounted for.
    // If the menu is visible, pass TRUE; otherwise FALSE.
    AdjustWindowRect(&rect, style, m_menuVisible ? TRUE : FALSE);
#if ENABLE_DEBUG_LOG
    {
        char dbg3[256];
        sprintf_s(dbg3, sizeof(dbg3), "ResizeWindow(DEBUG): adjusted window size (w,h)=(%d,%d)", rect.right - rect.left, rect.bottom - rect.top);
        DebugLog(dbg3);
    }
#endif
    
    SetWindowPos(m_hWnd, nullptr, 0, 0, 
        rect.right - rect.left, rect.bottom - rect.top, 
        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
#if ENABLE_DEBUG_LOG
    {
        RECT wr={0}; RECT cr={0};
        GetWindowRect(m_hWnd, &wr);
        GetClientRect(m_hWnd, &cr);
        char dbg4[256];
        sprintf_s(dbg4, sizeof(dbg4), "ResizeWindow(DEBUG): final WindowRect=(%d,%d)-(%d,%d) size=(%d,%d) ClientRect=(%d,%d)-(%d,%d)",
            wr.left, wr.top, wr.right, wr.bottom, wr.right-wr.left, wr.bottom-wr.top, cr.left, cr.top, cr.right, cr.bottom);
        DebugLog(dbg4);
    }
#endif
    
    UpdateLayout();
}

void WinApiCalc::ShowHelp() 
{
    // Сначала пытаемся открыть CHM файл
    char helpPath[MAX_PATH];
    
    // Получаем путь к исполняемому файлу
    if (GetModuleFileNameA(nullptr, helpPath, MAX_PATH) != 0)
    {
        // Заменяем имя exe на chm
        char* lastSlash = strrchr(helpPath, '\\');
        if (lastSlash)
        {
            strcpy_s(lastSlash + 1, MAX_PATH - (lastSlash + 1 - helpPath), "fcalc.chm");
            
            // Пробуем открыть CHM файл
            HWND helpWindow = HtmlHelpA(m_hWnd, helpPath, HH_DISPLAY_TOPIC, 0);
            if (helpWindow != nullptr)
            {
                // CHM файл открылся успешно
                return;
            }
        }
    }
    
    // Если CHM файл не удалось открыть, показываем MessageBox
    MessageBoxA(m_hWnd, 
        "Calculator Help\n\n"
        "Enter mathematical expressions in the input field.\n"
        "Use Calc menu to change format options.\n\n"
        "Hot keys:\n"
        "F1 - Help\n"
        "Ctrl+Home - Move window to (100,100)\n"
        "Ctrl+R - Wrap with root2()\n"
        "Ctrl+S - Wrap with ()^2\n"
        "Ctrl+I - Wrap with 1/()\n"
        "Ctrl+[ or Ctrl+] - Wrap with ()\n"
        "Enter - Evaluate expression\n"
        "Esc - Minimize (if enabled)",
        "Calculator Help", MB_OK | MB_ICONINFORMATION);
}

void WinApiCalc::SetWindowOpacity(int opacity) 
{
    if (!m_hWnd) return;
    
    // Store the opacity value
    m_opacity = opacity;
    
    // Set window layered style if not already set
    LONG exStyle = GetWindowLongA(m_hWnd, GWL_EXSTYLE);
    if (!(exStyle & WS_EX_LAYERED))
    {
        SetWindowLongA(m_hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }
    
    // Set the actual transparency
    SetLayeredWindowAttributes(m_hWnd, 0, (BYTE)opacity, LWA_ALPHA);
    
    // Save settings immediately
    SaveSettings();
}

void WinApiCalc::SetFontSize(int size) 
{
    // Validate font size (prevent extreme sizes)
    if (size == 0) return;
    
    // For negative pixel sizes (our normal case)
    if (size < 0) {
        if (size < -72) size = -72; // Max size limit
        if (size > -6) size = -6;   // Min size limit (6px minimum, reduced from 8px)
    }
    // For positive point sizes
    else {
        if (size < 4) size = 4;     // Min 4pt (reduced from 6pt)
        if (size > 72) size = 72;   // Max 72pt
    }
    
    m_fontSize = size;
    
    // Use existing UpdateFont method which handles font creation properly
    UpdateFont();
    
    // For large fonts (especially negative pixel values), recalculate window width
    if (size < 0 && size <= -18) // Lowered threshold for earlier resize
    {
        // Force window resize to accommodate larger font
        ResizeWindow();
    }
}

// Pseudo-functions implementation - статические C-функции
static long double MenuFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady() || !g_pCalcInstance->GetWindowMenu())
    {
        return 0.0; // Silently fail if UI not ready
    }
    
    int show = (int)x;
    if (show != 0)
    {
        // Show menu and persist MNU cleared
        g_pCalcInstance->SetMenuVisibilityOption(true);
    }
    else
    {
        // Hide menu and persist MNU set
        g_pCalcInstance->SetMenuVisibilityOption(false);
    }
    DrawMenuBar(g_pCalcInstance->GetWindow());
    g_pCalcInstance->ResizeWindow();
    
    return x;
}

static long double HelpFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady())
    {
        return 0.0; // Silently fail if UI not ready
    }
    
    g_pCalcInstance->ShowHelp();
    return x;
}

static long double OpacityFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady())
    {
        return x; // Return the value but don't execute
    }
    
    int opacity = (int)x;
    
    // Clamp opacity to valid range: 10% to 100%
    if (opacity < 10) opacity = 10;
    if (opacity > 100) opacity = 100;
    
    // Convert percentage to alpha value (255 = 100%)
    int alpha = (opacity * 255) / 100;
    
    g_pCalcInstance->SetWindowOpacity(alpha);
    return x;
}

static long double BinwideFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady())
    {
        return x; // Return the value but don't execute
    }
    
    int width = (int)x;
    // Validate binary width (8, 16, 24, 32, 48, 64)
    if (width == 8 || width == 16 || width == 24 || width == 32 || width == 48 || width == 64)
    {
        g_pCalcInstance->SetBinaryWidth(width);
    }
    return x;
}

static long double HomeFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady() || !g_pCalcInstance->GetWindow())
    {
        return x; // Return the value but don't execute
    }
    
    SetWindowPos(g_pCalcInstance->GetWindow(), nullptr, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    g_pCalcInstance->UpdateMenuChecks();
    return x;
}

static long double FontFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady())
    {
        return x; // Return the value but don't execute
    }
    
    int size = (int)x;
    
    // font(0) = return to default size
    if (size == 0)
    {
        size = -12; // Default font size in pixels (negative value)
    }
    
    g_pCalcInstance->SetFontSize(size);
    return x;
}

static long double VarsFunction(long double x) 
{
    // Safety check: ensure UI is fully initialized
    if (!g_pCalcInstance || !g_pCalcInstance->IsUIReady() || !g_pCalcInstance->GetCalculator())
    {
        return x; // Return the value but don't execute
    }
    
    if ((int)x) g_pCalcInstance->ShowVariablesDialog();
    return x;
}


// Message handler for about box.
INT_PTR CALLBACK WinApiCalc::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void WinApiCalc::SetMenuVisibilityOption(bool visible)
{
    if (visible)
    {
        m_options &= ~MNU; // clear MNU -> menu visible
        SetMenu(m_hWnd, m_hMenu);
        m_menuVisible = true;
    }
    else
    {
        m_options |= MNU; // set MNU -> menu hidden
        SetMenu(m_hWnd, NULL);
        m_menuVisible = false;
    }
    DrawMenuBar(m_hWnd);
    // Force immediate non-client recalculation so ResizeWindow uses correct metrics
    SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    // After the non-client area changed, force layout update so we can measure the
    // result control's client height (m_lastResultClientHeight) before resizing.
    // This addresses a timing mismatch where ResizeWindow() computed sizes
    // before UpdateLayout() had measured the actual control height.
    UpdateLayout();
    ResizeWindow();
    SaveSettings();
}

// Measure the rendered height of `text` when wrapped to `width` using the
// same font as the controls. Returns height in pixels.
int WinApiCalc::MeasureTextHeightForWidth(const std::string &text, int width)
{
    if (width <= 0) return GetControlHeight();

    HDC hdc = GetDC(m_hWnd);
    if (!hdc) return GetControlHeight();

    // Use the window's result edit font if available
    HFONT hFont = nullptr;
    if (m_hResultEdit) hFont = (HFONT)SendMessage(m_hResultEdit, WM_GETFONT, 0, 0);
    if (!hFont) {
        // Create matching font if none is applied
        hFont = CreateFontA(
            ScaleDPI(m_fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    }

    HFONT hOld = (HFONT)SelectObject(hdc, hFont);

    RECT rc = {0,0,width, INT_MAX};
    // DT_CALCRECT requires right coordinate to be width for wrapping
    rc.right = width;
    DrawTextA(hdc, text.c_str(), (int)text.length(), &rc, DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);

    int height = rc.bottom - rc.top;

    SelectObject(hdc, hOld);
    if (!((HFONT)SendMessage(m_hResultEdit, WM_GETFONT, 0, 0))) {
        DeleteObject(hFont);
    }
    ReleaseDC(m_hWnd, hdc);

    // Add a tiny inset so text doesn't touch bottom edge
    return height + ScaleDPI(2);
}
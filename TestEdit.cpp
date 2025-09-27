#include <windows.h>
#include <commctrl.h>

// Test program to isolate edit control rendering issues
// Minimal window with just an edit control using same settings as main app

#define IDC_TEST_EDIT 1001
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 100
#define CONTROL_HEIGHT 25

HINSTANCE g_hInst;
HWND g_hEdit;
HFONT g_hFont;

// DPI scaling function (same as main app)
int ScaleDPI(int value)
{
    HDC hdc = GetDC(nullptr);
    if (hdc)
    {
        int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(nullptr, hdc);
        return (value * dpiX) / 96;
    }
    return value;
}

// Dynamic height calculation (same as main app)
int GetControlHeight()
{
    HDC hdc = GetDC(nullptr);
    if (hdc)
    {
        HFONT oldFont = (HFONT)SelectObject(hdc, g_hFont ? g_hFont : GetStockObject(DEFAULT_GUI_FONT));
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        SelectObject(hdc, oldFont);
        ReleaseDC(nullptr, hdc);
        
        int charHeight = tm.tmHeight;
        return charHeight + 8; // Add padding for borders
    }
    return ScaleDPI(CONTROL_HEIGHT);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            // Initialize Common Controls (same as main app)
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_WIN95_CLASSES;
            InitCommonControlsEx(&icex);

            // Create font first (same as main app)
            g_hFont = CreateFontA(
                ScaleDPI(14), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

            // Create edit control with same settings as main app
            g_hEdit = CreateWindowExA(
                WS_EX_CLIENTEDGE,           // Same extended style
                "EDIT", 
                "Test input field - type here",
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, // Same style
                10,                         // X position
                10,                         // Y position  
                ScaleDPI(WINDOW_WIDTH - 40), // Width with margins
                GetControlHeight(),         // Dynamic height
                hWnd, 
                (HMENU)IDC_TEST_EDIT, 
                g_hInst, 
                nullptr);

            if (g_hEdit && g_hFont)
            {
                SendMessage(g_hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            }

            SetFocus(g_hEdit);
        }
        break;

    case WM_CTLCOLOREDIT:
        // Same color handling as main app
        SetBkColor((HDC)wParam, RGB(255, 255, 255)); // White background
        SetTextColor((HDC)wParam, RGB(0, 0, 0));     // Black text
        return (LRESULT)GetStockObject(WHITE_BRUSH);

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Fill background with white
            RECT rect;
            GetClientRect(hWnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        if (g_hFont)
        {
            DeleteObject(g_hFont);
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    // Register window class
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TestEditWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    
    if (!RegisterClassA(&wc))
    {
        MessageBoxA(nullptr, "Window registration failed", "Error", MB_OK);
        return -1;
    }

    // Create window
    HWND hWnd = CreateWindowExA(
        0,
        "TestEditWindow",
        "Test Edit Control - Check text rendering",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        ScaleDPI(WINDOW_WIDTH), ScaleDPI(WINDOW_HEIGHT),
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        MessageBoxA(nullptr, "Window creation failed", "Error", MB_OK);
        return -1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
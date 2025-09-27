#include "pch.h"
#include "WinApiCalc.h"
#include "Resource.h"

// Window procedure for the main window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static WinApiCalc* pCalc = nullptr;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        pCalc = new WinApiCalc();
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCalc);
        pCalc->Initialize(hwnd);
        return 0;
    }

    case WM_COMMAND:
    {
        if (pCalc)
        {
            // Handle combo box selection
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_HISTORY_COMBO)
            {
                int selection = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                if (selection != CB_ERR)
                {
                    pCalc->LoadHistoryItem(selection);
                }
                return 0;
            }

            // Handle edit control changes
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_EXPRESSION_EDIT)
            {
                pCalc->OnExpressionChanged();
                return 0;
            }

            // Handle menu commands
            pCalc->OnCommand(LOWORD(wParam));
        }
        break;
    }

    case WM_KEYDOWN:
        if (pCalc)
        {
            pCalc->OnKeyDown(wParam);
            
            // Handle Delete key when history is visible
            if (wParam == VK_DELETE && IsWindowVisible(pCalc->GetHistoryCombo()))
            {
                pCalc->DeleteSelectedHistoryItem();
            }
        }
        break;

    case WM_SIZE:
        if (pCalc)
        {
            pCalc->UpdateLayout();
        }
        break;

    case WM_PAINT:
        if (pCalc)
        {
            pCalc->OnPaint();
        }
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:  // Also handle static controls (readonly edit)
        if (pCalc)
        {
            // Set white background and black text for all edit controls
            WORD ctrlId = GetDlgCtrlID((HWND)lParam);
            if (ctrlId == IDC_RESULT_EDIT || ctrlId == IDC_EXPRESSION_EDIT)
            {
                SetTextColor((HDC)wParam, RGB(0, 0, 0));    // Black text
                SetBkColor((HDC)wParam, RGB(255, 255, 255)); // White background
                return (LRESULT)GetStockObject(WHITE_BRUSH);
            }
        }
        break;

    case WM_DESTROY:
        if (pCalc)
        {
            pCalc->OnDestroy();
            delete pCalc;
            pCalc = nullptr;
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Alternative application entry point (not used currently)
int APIENTRY WinMainAlternative(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Register the window class
    WNDCLASSEXA wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_WINAPICALC));
    wcex.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEA(IDC_WINAPICALC);
    wcex.lpszClassName = "WinApiCalcClass";
    wcex.hIconSm = LoadIconA(wcex.hInstance, MAKEINTRESOURCEA(IDI_SMALL));

    if (!RegisterClassExA(&wcex))
    {
        MessageBoxA(nullptr, "Window class registration failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Create the main window
    HWND hWnd = CreateWindowA(
        "WinApiCalcClass",                 // window class name
        "Calc",                            // window caption - ANSI only
        WS_OVERLAPPEDWINDOW,               // window style
        CW_USEDEFAULT, CW_USEDEFAULT,      // initial position
        500, 400,                          // initial size
        nullptr,                           // parent window
        nullptr,                           // window menu
        hInstance,                         // program instance handle
        nullptr                            // creation parameters
    );

    if (hWnd == nullptr)
    {
        MessageBoxA(nullptr, "Window creation failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Load accelerators
    HACCEL hAccelTable = LoadAcceleratorsA(hInstance, MAKEINTRESOURCEA(IDC_WINAPICALC));

    // Show and update the window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
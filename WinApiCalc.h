#pragma once

#include "Resource.h"
#include "framework.h"
#include "scalc.h"
#include "sfmts.h"

// Forward declarations
class calculator;

// Constants  
const int WINDOW_MIN_WIDTH = 533;  // Reduced from 800 to ~533 (1.5x smaller) 
const int WINDOW_MIN_HEIGHT = 200;
const int CONTROL_HEIGHT = 25; // Increased from 20 to 25 for proper text rendering
const int CONTROL_MARGIN = 0;  // No margins as per requirements
const int HISTORY_HEIGHT = 100;
const int MAX_HISTORY = 100;

// Expression calculator options (from SOW)
#define PAS     (1<<0)   // Pascal assignment style
#define SCI     (1<<1)   // Scientific format
#define UPCASE  (1<<2)   // Case insensitive
#define UTMP    (1<<3)   // Using $n for temp
#define FFLOAT  (1<<4)   // Forced float
#define DEG     (1<<5)   // Degrees format
#define SCF     (1<<6)   // Scientific format found
#define ENG     (1<<7)   // Engineering format found
#define STR     (1<<8)   // String format found
#define HEX     (1<<9)   // Hex format found
#define OCT     (1<<10)  // Octal format found
#define FBIN    (1<<11)  // Binary format found
#define DAT     (1<<12)  // Date time format found
#define CHR     (1<<13)  // Char format found
#define WCH     (1<<14)  // WChar format found
#define ESC     (1<<15)  // Escape format found
#define CMP     (1<<16)  // Computing format found
#define NRM     (1<<17)  // Normalized output
#define IGR     (1<<18)  // Integer output
#define UNS     (1<<19)  // Unsigned output
#define ALL     (1<<20)  // All outputs
#define MIN     (1<<21)  // Esc minimized feature
#define MNU     (1<<22)  // Show/hide menu feature
#define UTM     (1<<23)  // Unix time
#define FRC     (1<<24)  // Fraction output
#define FRI     (1<<25)  // Fraction inch output
#define AUT     (1<<26)  // Auto output

// Main application class
class WinApiCalc
{
private:
    HINSTANCE m_hInst;
    HWND m_hWnd;
    HWND m_hExpressionEdit;
    HWND m_hResultEdit;
    HWND m_hHistoryCombo;
    HWND m_hHistoryButton;
    HMENU m_hMenu;
    
    calculator* m_pCalculator;
    
    // Calculator state
    int m_options;
    int m_binWidth;
    int m_fontSize;
    int m_opacity;
    bool m_menuVisible;
    bool m_uiReady;         // Flag to indicate UI is fully initialized
    int m_windowX;          // Позиция окна X
    int m_windowY;          // Позиция окна Y
    
    std::vector<std::string> m_history;
    std::string m_currentExpression;
    
    // UI state
    int m_dpiX;
    int m_dpiY;
    int m_resultLines;
    
    // UI resources
    HBRUSH m_hWhiteBrush;
    WNDPROC m_originalEditProc;
    WNDPROC m_originalResultEditProc;
    WNDPROC m_originalListBoxProc;

public:
    WinApiCalc();
    ~WinApiCalc();

    // Initialization
    BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
    ATOM RegisterClass(HINSTANCE hInstance);
    void Initialize(HWND hwnd);
    
    // Public accessors
    HWND GetMainWindow() const { return m_hWnd; }

    // Window procedures
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ResultEditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListBoxSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Message handlers
    void OnCreate();
    void OnSize(int width, int height);
    void OnMove(int x, int y);
    void OnCommand(WPARAM wParam);
    void OnPaint();
    void OnDestroy();
    void OnKeyDown(WPARAM key);
    void OnLButtonDown(int x, int y);

    // Calculator operations
    void EvaluateExpression();
    void OnExpressionChanged();
    void OnEnterPressed();
    
    // Menu operations
    void UpdateMenuChecks();
    void OnMenuCommand(int command);
    void ToggleOption(int flag);
    void SetBinaryWidth(int width);
    
    // Hot key operations
    void OnCtrlR();    // Root2(...)
    void OnCtrlS();    // (...)^2
    void OnCtrlI();    // 1/(...)
    void OnCtrlLBracket(); // (...)
    void OnCtrlRBracket(); // (...)
    void OnCtrlHome(); // Home position and opacity
    void OnCtrlPlus(); // Increase font size by 1px
    void OnCtrlMinus(); // Decrease font size by 1px
    
    // Helper functions for hotkeys
    void OnHelp();
    void WrapExpressionWith(const char* prefix, const char* suffix);
    
    // Variables dialog
    void ShowVariablesDialog();
   
    // History management
    void LoadHistory();
    void SaveHistory();
    void AddToHistory(const std::string& expression);
    void LoadHistoryItem(int index);
    void UpdateHistoryCombo();
    void DeleteSelectedHistoryItem();
    
    // Settings persistence
    void LoadSettings();
    void SaveSettings();

    // Layout management
    void InitializeDPI();
    int ScaleDPI(int value);
    void UpdateLayout();
    void ResizeWindow();
    int GetCharWidth();
    int GetCharHeight();
    int GetControlHeight(); // Calculate proper height for edit controls
    int CalculateOptimalWidth(int charCount = 70);
    int CalculateContentBasedWidth(); // Новая функция для расчета на основе содержимого
    int CalculateCurrentResultWidth(); // Измерение ширины текущего результата
    void UpdateFont();

    // Utility
    void ShowHelp();
    void SetWindowOpacity(int opacity);
    void SetFontSize(int size);
    
    // Accessors
    HWND GetWindow() const { return m_hWnd; }
    HWND GetHistoryCombo() const { return m_hHistoryCombo; }
    calculator* GetCalculator() const { return m_pCalculator; }
    HMENU GetWindowMenu() const { return m_hMenu; }
    bool IsMenuVisible() const { return m_menuVisible; }
    void SetMenuVisible(bool visible) { m_menuVisible = visible; }
    bool IsUIReady() const { return m_uiReady; }
};
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
const int WM_DELAYED_CLEAR_HISTORY = WM_APP + 100;

// Main application class
class WinApiCalc
{
private:
    HINSTANCE m_hInst;
    HWND m_hWnd;
    HWND m_hExpressionEdit;
    HWND m_hResultEdit;
    HWND m_hComboBox;
    HWND m_hHelpWindow;   // HWND of the CHM help window (nullptr if closed)
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
    bool m_isUpdatingHistory; // Flag to suppress notifications during history updates
    bool m_suppressInteractive; // Flag to suppress interactive features (like color popup) during startup
    bool m_isWine;          // Flag indicating if running under Wine
    
    std::vector<std::string> m_history;
    std::string m_currentExpression;

    // Deferred color state
    uint32_t m_pendingColor;
    bool m_hasPendingColor;
    bool m_isColorWindowOpen;

    // UI state
    int m_dpiX;
    int m_dpiY;
    int m_resultLines;
    int m_lastResultClientHeight; // measured client height of result edit control
    int m_resultEditInternalPadding; // cached internal top/bottom padding inside result edit
    int m_resultEditInternalHorzPadding; // cached left+right internal padding inside result edit
    int m_lastComboHeight; // cached height of the combobox when closed
    
    // UI resources
    HBRUSH m_hWhiteBrush;
    WNDPROC m_originalEditProc;
    WNDPROC m_originalResultEditProc;
    WNDPROC m_originalComboBoxProc;

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
    static LRESULT CALLBACK ComboBoxSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ColorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // Color window
    static void ShowColorWindow(uint32_t color);
    void SetPendingColor(uint32_t color);
    void ProcessPendingColor();

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
    void WrapExpressionWith(const char* prefix, const char* suffix);
    
    // Variables dialog
    void ShowVariablesDialog();
   
    // History management
    void LoadHistory();
    void SaveHistory();
    void AddToHistory(const std::string& expression);
    void LoadHistoryItem(int index);
    void PopulateHistoryCombo();
    void ClearHistoryCombo();
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
    int MeasureTextHeightForWidth(const std::string &text, int width);
    bool IsWine() const { return m_isWine; }

    // Utility
    void ShowHelp();
    void SetWindowOpacity(int opacity);
    void SetFontSize(int size);
    
    // Accessors
    HWND GetWindow() const { return m_hWnd; }
    HWND GetHistoryCombo() const { return m_hComboBox; }
    calculator* GetCalculator() const { return m_pCalculator; }
    HMENU GetWindowMenu() const { return m_hMenu; }
    bool IsMenuVisible() const { return m_menuVisible; }
    void SetMenuVisible(bool visible) { m_menuVisible = visible; }
    // Toggle MNU option and persist
    void SetMenuVisibilityOption(bool visible);
    bool IsUIReady() const { return m_uiReady; }
    bool IsInteractiveSuppressed() const { return m_suppressInteractive; }
};
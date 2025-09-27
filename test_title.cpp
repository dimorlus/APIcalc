#include <windows.h>

int main() {
    // Создаем простое окно для теста
    HWND hwnd = CreateWindowA("STATIC", "Test Title ABC", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 400, 200, NULL, NULL, GetModuleHandle(NULL), NULL);
    
    if (hwnd) {
        MessageBoxA(NULL, "Window created. Check title!", "Info", MB_OK);
        
        // Меняем заголовок
        SetWindowTextA(hwnd, "Changed Title XYZ");
        MessageBoxA(NULL, "Title changed. Check again!", "Info", MB_OK);
        
        DestroyWindow(hwnd);
    }
    return 0;
}
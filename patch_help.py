import sys

with open('WinApiCalc.cpp', 'rb') as f:
    data = f.read()

# Work with bytes to avoid encoding issues
content = data.decode('utf-8-sig')

# Fix 1: In ShowHelp() - save HWND of CHM window
# Exact text from file: 2 spaces + // CHM file opened successfully\r\n + 7 spaces + return;
old1 = '  // CHM file opened successfully\r\n       return;'
new1 = '  // CHM file opened successfully - save handle so Esc can close it\r\n        m_hHelpWindow = helpWindow;\r\n       return;'

if old1 in content:
    content = content.replace(old1, new1, 1)
    print('Fix1: OK')
else:
    print('Fix1: NOT FOUND')
    idx = content.find('CHM file opened')
    print('Context:', repr(content[idx-3:idx+60]))
    sys.exit(1)

# Fix 2: In OnKeyDown() - handle Esc to close CHM first
# Find VK_ESCAPE block and replace 'if (m_options & MIN)' with additional CHM check
old2 = '    if (m_options & MIN) // If "Esc minimized" is enabled\r\n     {\r\n      ShowWindow (m_hWnd, SW_MINIMIZE);\r\n     }\r\n    else\r\n     {\r\n      // Close the application\r\n      PostMessage (m_hWnd, WM_CLOSE, 0, 0);\r\n     }\r\n   }\r\n}'
new2 = '    // First, check if the CHM help window is open - close it\r\n    if (m_hHelpWindow && IsWindow (m_hHelpWindow))\r\n     {\r\n      HtmlHelpA (nullptr, nullptr, HH_CLOSE_ALL, 0);\r\n      m_hHelpWindow = nullptr;\r\n     }\r\n    else if (m_options & MIN) // If "Esc minimized" is enabled\r\n     {\r\n      ShowWindow (m_hWnd, SW_MINIMIZE);\r\n     }\r\n    else\r\n     {\r\n      // Close the application\r\n      PostMessage (m_hWnd, WM_CLOSE, 0, 0);\r\n     }\r\n   }\r\n}'

if old2 in content:
    content = content.replace(old2, new2, 1)
    print('Fix2: OK')
else:
    print('Fix2: NOT FOUND')
    idx = content.find('VK_ESCAPE')
    while idx != -1:
        snippet = content[idx-2:idx+300]
        if 'MIN' in snippet and 'SW_MINIMIZE' in snippet:
            print('Context:', repr(snippet))
            break
        idx = content.find('VK_ESCAPE', idx+1)
    sys.exit(2)

with open('WinApiCalc.cpp', 'wb') as f:
    f.write(content.encode('utf-8'))

print('Done - both fixes applied successfully')

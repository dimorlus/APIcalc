@echo off
echo win32bit float64
ls -s --block-size=1 ".\ccalc\bin\Win32\Release\ccalc.exe" ".\Release\fcalc.exe"

echo.
echo STD win64bit float64
ls -s --block-size=1 ".\ccalc\bin\x64\Release\ccalc.exe" ".\bin64\Release\std\fcalc.exe"

echo.
echo GCC win64bit float128
ls -s --block-size=1 ".\gcc_release\ccalc.exe" ".\gcc_release\fcalc.exe"

echo.
echo DLL based win64bit float128
ls -s --block-size=1 ".\gcc_release\calclib.dll" ".\ccalc\bin\x64\Release\dll\ccalc.exe" ".\bin64\Release\dll\fcalc.exe"

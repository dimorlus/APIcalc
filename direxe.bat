@echo off
echo win32bit float64
dir .\ccalc\bin\Win32\Release\ccalc.exe
dir .\Release\fcalc.exe

echo STD win64bit float64
dir .\ccalc\bin\x64\Release\ccalc.exe
dir .\bin64\Release\std\fcalc.exe

echo GCC win64bit float128
dir .\gcc_release\ccalc.exe 
dir .\gcc_release\fcalc.exe 

echo DLL based win64bit float128 
dir .\gcc_release\calclib.dll 
dir .\ccalc\bin\x64\Release\dll\ccalc.exe 
dir .\bin64\Release\dll\fcalc.exe 

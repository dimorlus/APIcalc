@echo off
echo ### STD win32 float64 (fcalc_setup32.exe)
for %%I in (".\ccalc\bin\Win32\Release\ccalc.exe") do echo %%~nxI - %%~zI bytes^<br^>
for %%I in (".\Release\fcalc.exe") do echo %%~nxI - %%~zI bytes

echo.
echo ### STD win64 float64 (fcalc_setup.exe, fcalc_w10+64_setup.exe, fcalc_w10+64_std_dll_setup.exe) 
for %%I in (".\ccalc\bin\x64\Release\ccalc.exe") do echo %%~nxI - %%~zI bytes^<br^>
for %%I in (".\bin64\Release\std\fcalc.exe") do echo %%~nxI - %%~zI bytes

echo.
echo ### GCC win64 float128 (fcalc_w10+64_std_dll_setup.exe)
for %%I in (".\gcc_release\ccalc.exe") do echo %%~nxI - %%~zI bytes^<br^>
for %%I in (".\gcc_release\fcalc.exe") do echo %%~nxI - %%~zI bytes

echo.
echo ### DLL based win64 float128 (fcalc_w10+64_std_dll_setup.exe)
for %%I in (".\gcc_release\calclib.dll") do echo %%~nxI - %%~zI bytes^<br^>
for %%I in (".\ccalc\bin\x64\Release\dll\ccalc.exe") do echo %%~nxI - %%~zI bytes^<br^>
for %%I in (".\bin64\Release\dll\fcalc.exe") do echo %%~nxI - %%~zI bytes

echo.
echo ### BCB based win32 float80 (fcalc_bcb_setup.exe)
for %%I in (".\BCB\ccalc.exe") do echo %%~nxI - %%~zI bytes^<br^>
for %%I in (".\BCB\fcalc.exe") do echo %%~nxI - %%~zI bytes

REM === GCC: calclib.dll with 128-bit floats ===
del /Q gcc_release\*.*
c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe --makefile=dll_make.mak
copy /Y gcc_release\calclib.dll bin64\Release\dll\calclib.dll

REM === Import library from .def (no MSVC compilation of calclib) ===
for /d %%D in ("c:\Program Files\Microsoft Visual Studio\18\Professional\VC\Tools\MSVC\*") do set "MSVC_DIR=%%D"
"%MSVC_DIR%\bin\Hostx86\x64\lib.exe" /def:calclib.def /machine:x64 /out:calclib.lib

REM === DLL-based EXEs (link against calclib.lib) ===
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build

echo "Compile setup packages"
pause
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" fcalc_w10+64_std_dll.iss
echo "Run setup"
pause
.\Setup\fcalc_w10+64_std_dll_setup.exe

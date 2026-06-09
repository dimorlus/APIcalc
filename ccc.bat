REM === MSVC: standalone + CLI (x64) ===
"c:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_std.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
"c:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build

REM === MSVC: standalone + CLI (x86) ===
"c:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_std.vcxproj /p:Configuration=Release /p:Platform=x86 /t:Rebuild
"c:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc.vcxproj /p:Configuration=Release /p:Platform=x86 /t:Build

REM === Import library from .def (no MSVC compilation of calclib) ===
for /d %%D in ("c:\Program Files\Microsoft Visual Studio\18\Professional\VC\Tools\MSVC\*") do set "MSVC_DIR=%%D"
"%MSVC_DIR%\bin\Hostx86\x64\lib.exe" /def:calclib.def /machine:x64 /out:calclib.lib

REM === DLL-based EXEs (link against calclib.lib) ===
"c:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build
"c:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build

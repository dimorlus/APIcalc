"utils\vcntr.exe" -csubver.cfg all -d -q
pause
type ver.h
pause
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_std.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_std.vcxproj /p:Configuration=Release /p:Platform=x86 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc.vcxproj /p:Configuration=Release /p:Platform=x86 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calclib.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
copy /Y x64\Release\calclib.lib .\
::del /Q gcc_release\*.*
::c:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe makefilemake_dll.mak 2>nul
c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe --makefile=dll_make.mak
copy /Y gcc_release\calclib.dll bin64\Release\dll\calclib.dll
cd BCB
"C:\Program Files (x86)\Borland\CBuilder6\Bin\make.exe" -fccalc.mak -B
"C:\Program Files (x86)\Borland\CBuilder6\Bin\make.exe" -ffcalc.mak -B
cd ..
echo "Compile setup packages"
pause
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" fcalc_w10+64_std_dll.iss
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" fcalc.iss
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" fcalc_w10+64.iss
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" fcalc5.iss
cd BCB
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" fcalc.iss
cd..
echo "Run setup"
pause
.\Setup\fcalc_w10+64_std_dll_setup.exe
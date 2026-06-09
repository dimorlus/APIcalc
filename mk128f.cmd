"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calclib.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
copy /Y x64\Release\calclib.lib .\
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" ccalc\ccalc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" calc_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild

del /Q gcc_release\*.*
c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe --makefile=dll_make.mak
copy /Y gcc_release\calclib.dll bin64\Release\dll\calclib.dll
echo "Compile setup packages"
pause
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" fcalc_w10+64_std_dll.iss
echo "Run setup"
pause
.\Setup\fcalc_w10+64_std_dll_setup.exe
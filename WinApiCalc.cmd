"c:\Espressif\utils\vcntr.exe" -csubver.cfg all -d -q
"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" WinApiCalc.vcxproj /p:Configuration=Release /p:Platform=x64
pause
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" fcalc.iss
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" fcalc5.iss

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

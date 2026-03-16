del /Q gcc_release\*.*
del /Q gcc_release80\*.*

c:\Qt\Tools\mingw1120_64\bin\mingw32-make.exe  >mk.txt 2>&1
c:\Qt\Tools\mingw810_32\bin\mingw32-make.exe 80bit  >mk80.txt 2>&1
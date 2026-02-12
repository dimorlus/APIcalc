@echo off
if "%1"=="" (
    echo Usage: extract_version commit_hash
    exit /b 1
)

set HASH=%1
set OUT=..\ver_%HASH%

mkdir %OUT%

git show %HASH%:scalc.cpp > %OUT%\scalc.cpp
git show %HASH%:scalc.h   > %OUT%\scalc.h
git show %HASH%:sfmts.cpp > %OUT%\sfmts.cpp
git show %HASH%:sfmts.h   > %OUT%\sfmts.h
git show %HASH%:sfunc.cpp > %OUT%\sfunc.cpp
git show %HASH%:sfunc.h   > %OUT%\sfunc.h
git show %HASH%:WinApiCalc.cpp   > %OUT%\WinApiCalc.cpp

echo Done.

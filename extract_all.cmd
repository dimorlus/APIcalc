@echo off
setlocal EnableDelayedExpansion

set FILES=scalc.cpp scalc.h sfmts.cpp sfmts.h sfunc.cpp sfunc.h WinApiCalc.cpp ver.h
set OUTROOT=..\versions_dump

if not exist "%OUTROOT%" mkdir "%OUTROOT%"

echo.
echo Extracting versions...
echo.

for /f %%H in ('git rev-list --reverse --abbrev-commit HEAD') do (

    for /f %%D in ('git show -s --format^=%%cd --date^=short %%H') do (
        set CDATE=%%D
    )

    for /f %%T in ('git show -s --format^=%%ct %%H') do (
        set CTS=%%T
    )

    set DIRNAME=!CDATE!_!CTS!
    set OUTDIR=%OUTROOT%\!DIRNAME!

    echo Creating !DIRNAME!
    mkdir "!OUTDIR!"

    for %%F in (%FILES%) do (
        git show %%H:%%F > "!OUTDIR!\%%F" 2>nul
    )
)

echo.
echo Done.
pause

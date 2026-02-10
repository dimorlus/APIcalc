@echo off
setlocal EnableDelayedExpansion

set VER_FILE=ver.h
set VER=

for /f "tokens=3" %%A in ('findstr "_ver_" %VER_FILE%') do (
    set VER=%%A
)

:: убираем возможную запятую / точку с запятой
set VER=%VER:;=%

if "%VER%"=="" (
    echo ERROR: version not found
    exit /b 1
)

echo %VER%
endlocal

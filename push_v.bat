@echo off
setlocal EnableDelayedExpansion

git rev-parse --is-inside-work-tree >nul 2>&1 || (
    echo ERROR: not a git repository
    exit /b 1
)

:: Проверка изменений
git status --porcelain > status.tmp
for %%A in (status.tmp) do set HAS_CHANGES=1
del status.tmp

if not defined HAS_CHANGES (
    echo No changes to commit
    exit /b 0
)

:: Версия
for /f %%V in ('getver.bat') do set VER=%%V
set MSG=%date% %time%  v%VER%

:: Показываем, ЧТО будет закоммичено
echo ================================
git status --short
echo ================================
echo Commit message:
echo %MSG%
echo.

:: Подтверждение
choice /m "Commit and push these changes?"
if errorlevel 2 (
    echo Aborted.
    exit /b 0
)

git add -A
git commit -m "%MSG%" || exit /b 1
git push || exit /b 1

echo Done.
endlocal

@echo off
setlocal EnableDelayedExpansion

:: --- Проверка, что мы в git-репозитории ---
git rev-parse --is-inside-work-tree >nul 2>&1
if errorlevel 1 (
    echo ERROR: not a git repository
    exit /b 1
)

:: --- Проверка, есть ли изменения ---
git status --porcelain > status.tmp
for %%A in (status.tmp) do set HAS_CHANGES=1
del status.tmp

if not defined HAS_CHANGES (
    echo No changes to commit
    exit /b 0
)

:: --- Получаем версию ---
for /f %%V in ('getver.bat') do set VER=%%V

:: --- Формируем сообщение коммита ---
set MSG=%date% %time%  v%VER%

echo Commit message:
echo %MSG%
echo.

:: --- Добавляем ВСЕ изменения ---
git add -A

:: --- Коммит ---
git commit -m "%MSG%"
if errorlevel 1 (
    echo ERROR: commit failed
    exit /b 1
)

:: --- Пуш ---
git push
if errorlevel 1 (
    echo ERROR: push failed
    exit /b 1
)

echo Done.
endlocal

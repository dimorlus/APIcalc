for /f %%V in ('getver.bat') do set VER=%%V

gh release view v%VER% >nul 2>&1
if not errorlevel 1 (
  gh release delete v%VER% --yes
)

gh release create v%VER% ^
  Setup\fcalc_setup.exe ^
  Setup\fcalc_bcb_setup.exe ^
  Setup\fcalc_setup32.exe ^
  Setup\fcalc_w10+64_setup.exe ^
  Setup\fcalc_w10+64_std_dll_setup.exe ^
  --title "APIcalc %VER%" ^
  --notes "Windows installers. Version available via 'version' command."

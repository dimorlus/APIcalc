for /f %%V in ('getver.bat') do set VER=%%V

gh release create v%VER% ^
  Setup\fcalc_setup.exe ^
  Setup\fcalc_setup32.exe ^
  Setup\fcalc_w10+64_setup.exe ^
  --title "APIcalc %VER%" ^
  --notes "Windows installers. Version available via 'version' command."

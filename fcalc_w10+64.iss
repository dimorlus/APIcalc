; -- TesterAll.iss --

[Setup]
AppName=fcalc
AppVerName=fcalc (Scientific formula calculator) 1.0 files
VersionInfoVersion=1.0.0.0
AppContact=http://dorlov.no-ip.com
DefaultDirName={commonpf64}\FC
DefaultGroupName=fcalc
UninstallDisplayIcon={app}\fcalc.exe
LicenseFile=fclic.txt
Compression=lzma/ultra
SolidCompression=yes
OutputDir=Setup
OutputBaseFilename=fcalc_w10+64_setup
CreateUninstallRegKey=yes
PrivilegesRequired=admin

[Types]
Name: "custom"; Description: "custom"; Flags:iscustom

[Components]
; Note: The following line does nothing other than provide a visual cue
; to the user that the program files are installed no matter what.
Name: "program"; Description: "Fcalc formula calculator"; Types: custom; Flags: fixed


[Files]
Source: ".\x64\Release\fcalc.exe"; DestDir: "{app}";Components: program; Flags:ignoreversion
Source: "fcalc.chm"; DestDir: "{app}";Components: program; Flags:ignoreversion

[Registry]
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"CurrentExpression"; ValueData:"help(1)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History0"; ValueData:"L:=130u;c:=2.2n;f:=1/(2*pi*sqrt(l*c))"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History1"; ValueData:"font(14)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History2"; ValueData:"now(tz)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History3"; ValueData:"opacity(100)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History4"; ValueData:"menu(0)"; Components:program; Flags: createvalueifdoesntexist

Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"WindowY"; ValueData:"100"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"WindowX"; ValueData:"100"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"Opacity"; ValueData:"$ff"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"Options"; ValueData:"$853a17"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"HistoryCount"; ValueData:"$5"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"FontSize"; ValueData:"$0e"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: dword; Valuename:"BinaryWidth"; ValueData:"$40"; Components:program; Flags: createvalueifdoesntexist


[UninstallDelete]
Type: dirifempty; Name: "{app}"


[Icons]
Name: "{group}\fcalc"; Filename: "{app}\fcalc.exe"; WorkingDir: "{app}";Components: program
Name: "{group}\Uninstall"; Filename: "{app}\unins000.exe"
Name: "{userdesktop}\fcalc"; Filename: "{app}\fcalc.exe"; WorkingDir: "{app}";Components: program

[Run]
Filename: "{app}\fcalc.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent unchecked

; -- fcalc.iss --

[Setup]
AppName=fcalc
AppVerName=fcalc (Scientific formula calculator) 1.0 files
VersionInfoVersion=1.0.0.0
AppContact=http://dorlov.no-ip.com
DefaultDirName={pf}\FC
DefaultGroupName=fcalc
UninstallDisplayIcon={app}\fcalc.exe
LicenseFile=fclic.txt
Compression=lzma/ultra
SolidCompression=yes
OutputDir=Setup
OutputBaseFilename=fcalc_setup32
CreateUninstallRegKey=yes
PrivilegesRequired=admin
;UsedUserAreasWarning=no
;ArchitecturesAllowed=x86 x64
;ArchitecturesInstallIn64BitMode=x64
;ArchitecturesAllowed=x86

[Types]
Name: "custom"; Description: "custom"; Flags: iscustom

[Components]
Name: "program"; Description: "Fcalc formula calculator"; Types: custom; Flags: fixed

[Files]
; 32-bit version for x86 systems  
Source: ".\Release\fcalc.exe"; DestDir: "{app}"; Components: program; Flags: ignoreversion; Check: not Is64BitInstallMode
; Help file for both architectures
Source: "fcalc.chm"; DestDir: "{app}"; Components: program; Flags: ignoreversion

; Visual C++ Redistributables
Source: ".\Redistr\VC_redist.x86.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: not Is64BitInstallMode

[Registry]
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"CurrentExpression"; ValueData:"help(1)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History0"; ValueData:"L:=130u;c:=2.2n;f:=1/(2*pi*sqrt(l*c))"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History1"; ValueData:"font(14)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History2"; ValueData:"now(tz)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History3"; ValueData:"opacity(100)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History4"; ValueData:"menu(0)"; Components:program; Flags: createvalueifdoesntexist
Root:HKCU; Subkey: "Software\WinApiCalc"; Valuetype: string; Valuename:"History5"; ValueData:"version"; Components:program; Flags: createvalueifdoesntexist

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
Name: "{group}\fcalc"; Filename: "{app}\fcalc.exe"; WorkingDir: "{app}"; Components: program
Name: "{group}\Uninstall"; Filename: "{app}\unins000.exe"
Name: "{userdesktop}\fcalc"; Filename: "{app}\fcalc.exe"; WorkingDir: "{app}"; Components: program

[Run]
; Install VC++ Redistributable silently before main installation
Filename: "{tmp}\VC_redist.x86.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Redistributable (x86)..."; Check: not Is64BitInstallMode and VCRedistNeedsInstall; Flags: waituntilterminated

; Launch application after installation
Filename: "{app}\fcalc.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent unchecked

[Code]
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
begin
  // Check VC++ 2015-2022 Redistributable
  begin
    Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE,
      'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\X86', 'Version', Version);
    if Result then
      Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE,
        'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86', 'Version', Version);
  end;
end;
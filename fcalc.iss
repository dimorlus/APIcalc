; -- fcalc.iss --

[Setup]
AppName=fcalc
AppVerName=fcalc (Scientific formula calculator) 2.048 files
VersionInfoVersion=1.0.0.0
AppContact=http://dorlov.no-ip.com
DefaultDirName={autopf}\FC
DefaultGroupName=fcalc
UninstallDisplayIcon={app}\fcalc.exe
LicenseFile=fclic.txt
Compression=lzma/ultra
SolidCompression=yes
OutputDir=Setup
OutputBaseFilename=fcalc_setup
CreateUninstallRegKey=yes
PrivilegesRequired=admin
UsedUserAreasWarning=no
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64

[Types]
Name: "custom"; Description: "custom"; Flags: iscustom

[Components]
Name: "program"; Description: "Fcalc formula calculator"; Types: custom; Flags: fixed
Name: "ccalc"; Description:"CLI Calculator "; Types: custom


[Files]
; 64-bit version for x64 systems
Source: ".\x64\Release\fcalc.exe"; DestDir: "{app}"; Components: program; Flags: ignoreversion; Check: Is64BitInstallMode
Source: ".\ccalc\bin\x64\Release\ccalc.exe"; DestDir: "{app}"; Components: ccalc; Flags: ignoreversion; Check: Is64BitInstallMode
; 32-bit version for x86 systems  
Source: ".\Release\fcalc.exe"; DestDir: "{app}"; Components: program; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: ".\ccalc\bin\Win32\Release\ccalc.exe"; DestDir: "{app}"; Components: ccalc; Flags: ignoreversion; Check: not Is64BitInstallMode
; Help file for both architectures
Source: "fcalc.chm"; DestDir: "{app}"; Components: program; Flags: ignoreversion
Source: "ccalc\ccalc.cfg"; DestDir: "{app}"; Components: ccalc; Flags: ignoreversion

; Visual C++ Redistributables
Source: ".\Redistr\VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: Is64BitInstallMode
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
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Redistributable (x64)..."; Check: Is64BitInstallMode and VCRedistNeedsInstall; Flags: waituntilterminated
Filename: "{tmp}\VC_redist.x86.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Redistributable (x86)..."; Check: not Is64BitInstallMode and VCRedistNeedsInstall; Flags: waituntilterminated

; Launch application after installation
Filename: "{app}\fcalc.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent unchecked

[Code]
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
begin
  // Check VC++ 2015-2022 Redistributable
  if Is64BitInstallMode then
  begin
    Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE, 
      'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\X64', 'Version', Version);
    if Result then
      Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE,
        'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version);
  end
  else
  begin
    Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE,
      'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\X86', 'Version', Version);
    if Result then
      Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE,
        'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86', 'Version', Version);
  end;
end;

const
  HWND_BROADCAST = $ffff;
  WM_SETTINGCHANGE = $001A;
  SMTO_ABORTIFHUNG = 2;

// Notify system about environment changes
procedure RefreshEnvironment;
var
  S: string;
  MsgResult: DWORD;
begin
  S := 'Environment';
  SendTextMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
    PAnsiChar(S), SMTO_ABORTIFHUNG, 5000, MsgResult);
end;

// Add directory to PATH if not already present
procedure AddDirToPath(const DirPath: String);
var
  Path: String;
  RegKey: Integer;
  PathUpper: String;
  DirPathUpper: String;
begin
  // Use HKEY_CURRENT_USER for user-level installation
  RegKey := HKEY_CURRENT_USER;
  
  // Get current PATH
  if RegQueryStringValue(RegKey, 'Environment', 'Path', Path) then
  begin
    PathUpper := Uppercase(Path);
    DirPathUpper := Uppercase(DirPath);
    
    // Check if directory is already in PATH (case-insensitive)
    // Check for "dir;" or ";dir;" or ";dir" or just "dir" (if PATH has only one entry)
    if (Pos(DirPathUpper + ';', PathUpper) = 0) and
       (Pos(';' + DirPathUpper + ';', PathUpper) = 0) and
       (Pos(';' + DirPathUpper, PathUpper) = 0) and
       (PathUpper <> DirPathUpper) then
    begin
      // Add directory to PATH
      if Length(Path) > 0 then
      begin
        if Path[Length(Path)] <> ';' then
          Path := Path + ';';
      end;
      Path := Path + DirPath;
      
      // Write back to registry
      if RegWriteStringValue(RegKey, 'Environment', 'Path', Path) then
      begin
        Log('Successfully added to PATH: ' + DirPath);
        RefreshEnvironment;
      end
      else
        Log('Failed to write PATH to registry');
    end
    else
      Log('Directory already in PATH: ' + DirPath);
  end
  else
  begin
    // PATH doesn't exist, create it
    if RegWriteStringValue(RegKey, 'Environment', 'Path', DirPath) then
    begin
      Log('Created new PATH with: ' + DirPath);
      RefreshEnvironment;
    end
    else
      Log('Failed to create PATH in registry');
  end;
end;

// Remove directory from PATH
procedure RemoveDirFromPath(const DirPath: String);
var
  Path: String;
  P: Integer;
  NewPath: String;
  RegKey: Integer;
  PathUpper: String;
  DirPathUpper: String;
begin
  RegKey := HKEY_CURRENT_USER;
  
  if RegQueryStringValue(RegKey, 'Environment', 'Path', Path) then
  begin
    PathUpper := Uppercase(Path);
    DirPathUpper := Uppercase(DirPath);
    NewPath := Path;
    
    // Try to find and remove "DirPath;" 
    P := Pos(Uppercase(DirPath) + ';', PathUpper);
    if P > 0 then
    begin
      Delete(NewPath, P, Length(DirPath) + 1);
    end
    else
    begin
      // Try ";DirPath"
      P := Pos(';' + Uppercase(DirPath), PathUpper);
      if P > 0 then
      begin
        Delete(NewPath, P, Length(DirPath) + 1);
      end
      else
      begin
        // Try just "DirPath" (single entry)
        if PathUpper = DirPathUpper then
          NewPath := '';
      end;
    end;
    
    // Clean up double semicolons
    while Pos(';;', NewPath) > 0 do
      StringChangeEx(NewPath, ';;', ';', True);
    
    // Remove leading/trailing semicolons
    if (Length(NewPath) > 0) and (NewPath[1] = ';') then
      Delete(NewPath, 1, 1);
    if (Length(NewPath) > 0) and (NewPath[Length(NewPath)] = ';') then
      Delete(NewPath, Length(NewPath), 1);
    
    // Write back to registry
    if NewPath <> Path then
    begin
      if RegWriteStringValue(RegKey, 'Environment', 'Path', NewPath) then
      begin
        Log('Successfully removed from PATH: ' + DirPath);
        RefreshEnvironment;
      end
      else
        Log('Failed to write PATH to registry');
    end
    else
      Log('Directory not found in PATH: ' + DirPath);
  end;
end;

// Called after successful installation
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Add to PATH only if ccalc component is selected
    if WizardIsComponentSelected('ccalc') then
    begin
      Log('Adding to PATH...');
      AddDirToPath(ExpandConstant('{app}'));
    end
    else
      Log('ccalc component not selected, skipping PATH addition');
  end;
end;

// Called during uninstallation
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    Log('Removing from PATH...');
    RemoveDirFromPath(ExpandConstant('{app}'));
  end;
end;
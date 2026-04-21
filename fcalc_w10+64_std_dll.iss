; -- TesterAll.iss --
[Setup]
AppName=fcalc
AppVerName=fcalc (Scientific formula calculator) 2.184 files
VersionInfoVersion=2.184
AppContact=http://dorlov.no-ip.com
DefaultDirName={autopf}\FC
DefaultGroupName=fcalc
UninstallDisplayIcon={app}\fcalc.exe
LicenseFile=fclic.txt
Compression=lzma/ultra
SolidCompression=yes
OutputDir=Setup
OutputBaseFilename=fcalc_w10+64_std_dll_setup
CreateUninstallRegKey=yes
PrivilegesRequired=admin
UsedUserAreasWarning=no
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64

[Types]
Name: "std";    Description: "Standard (built-in calculator engine)"
Name: "dll";    Description: "DLL version (GCC engine, higher precision)"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "program";     Description: "fcalc formula calculator";         Types: std dll custom; Flags: fixed
Name: "program\std"; Description: "Standard version";                 Types: std;            Flags: exclusive
Name: "program\dll"; Description: "DLL version (higher precision)";   Types: dll;            Flags: exclusive
Name: "ccalc";       Description: "CLI Calculator (ccalc.exe)";       Types: custom
Name: "ccalc\std";   Description: "Standard CLI";                     Types: std;            Flags: exclusive
Name: "ccalc\dll";   Description: "DLL CLI (higher precision)";       Types: dll;            Flags: exclusive

[Files]
; GUI Standard version
Source: ".\bin64\Release\std\fcalc.exe";         DestDir: "{app}"; DestName: "fcalc.exe"; Components: program\std; Flags: ignoreversion
; GUI DLL version
Source: ".\bin64\Release\dll\fcalc.exe";         DestDir: "{app}"; DestName: "fcalc.exe"; Components: program\dll; Flags: ignoreversion
Source: ".\gcc_release\calclib.dll";             DestDir: "{app}";                        Components: program\dll; Flags: ignoreversion
; CLI Standard version
Source: ".\ccalc\bin\x64\Release\ccalc.exe";     DestDir: "{app}"; DestName: "ccalc.exe"; Components: ccalc\std;   Flags: ignoreversion
; CLI DLL version
Source: ".\ccalc\bin\x64\Release\dll\ccalc.exe"; DestDir: "{app}"; DestName: "ccalc.exe"; Components: ccalc\dll;   Flags: ignoreversion
Source: ".\gcc_release\calclib.dll";             DestDir: "{app}";                        Components: ccalc\dll;   Flags: ignoreversion skipifsourcedoesntexist

Source: ".\fcalc.chm"; DestDir: "{app}";Components: program; Flags:ignoreversion
Source: ".\ccalc\ccalc.cfg"; DestDir: "{app}"; Components: ccalc; Flags: ignoreversion onlyifdoesntexist uninsneveruninstall; Permissions: users-modify
Source: ".\consts.txt"; DestDir: "{app}"; Flags: ignoreversion uninsneveruninstall; Permissions: users-modify
Source: ".\help.txt"; DestDir: "{app}"; Flags: ignoreversion uninsneveruninstall; Permissions: users-modify
Source: ".\README.pdf"; DestDir: "{app}"; Flags: ignoreversion uninsneveruninstall; Permissions: users-modify
Source: ".\user.txt"; DestDir: "{app}"; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: users-modify

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
Name: "{group}\fcalc"; Filename: "{app}\fcalc.exe"; WorkingDir: "{app}";Components: program
Name: "{group}\Uninstall"; Filename: "{app}\unins000.exe"
Name: "{userdesktop}\fcalc"; Filename: "{app}\fcalc.exe"; WorkingDir: "{app}";Components: program
; Ярлык для редактирования констант
Name: "{group}\Edit Constants"; Filename: "notepad.exe"; Parameters: "{app}\consts.txt"; WorkingDir: "{app}"; IconFilename: "shell32.dll"; IconIndex: 69
Name: "{group}\Edit User Constants"; Filename: "notepad.exe"; Parameters: "{app}\user.txt"; WorkingDir: "{app}"; IconFilename: "shell32.dll"; IconIndex: 69

[Run]
Filename: "{app}\fcalc.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent unchecked

[Code]

// Sync ccalc subcomponent with the selected type (std/dll)
// Called when user changes component selection
procedure SyncCcalcWithType;
var
  CcalcChecked: Boolean;
begin
  CcalcChecked := WizardIsComponentSelected('ccalc\std') or
                  WizardIsComponentSelected('ccalc\dll');
  if WizardIsComponentSelected('program\std') then
  begin
    WizardSelectComponents('ccalc\dll');  // deselect dll
    if CcalcChecked then
      WizardSelectComponents('ccalc\std');
  end
  else if WizardIsComponentSelected('program\dll') then
  begin
    WizardSelectComponents('ccalc\std');  // deselect std
    if CcalcChecked then
      WizardSelectComponents('ccalc\dll');
  end;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectComponents then
    SyncCcalcWithType;
end;

// Add directory to PATH if not already present
procedure AddDirToPath(const DirPath: String);
var
  Path: String;
  RegKey: Integer;
  PathUpper: String;
  DirPathUpper: String;
begin
  RegKey := HKEY_CURRENT_USER;
  if RegQueryStringValue(RegKey, 'Environment', 'Path', Path) then
  begin
    PathUpper    := Uppercase(Path);
    DirPathUpper := Uppercase(DirPath);
    if (Pos(DirPathUpper + ';', PathUpper) = 0) and
       (Pos(';' + DirPathUpper + ';', PathUpper) = 0) and
       (Pos(';' + DirPathUpper, PathUpper) = 0) and
       (PathUpper <> DirPathUpper) then
    begin
      if Length(Path) > 0 then
        if Path[Length(Path)] <> ';' then
          Path := Path + ';';
      Path := Path + DirPath;
      if RegWriteStringValue(RegKey, 'Environment', 'Path', Path) then
        Log('Successfully added to PATH: ' + DirPath)
      else
        Log('Failed to write PATH to registry');
    end
    else
      Log('Directory already in PATH: ' + DirPath);
  end
  else
  begin
    if RegWriteStringValue(RegKey, 'Environment', 'Path', DirPath) then
      Log('Created new PATH with: ' + DirPath)
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
    PathUpper    := Uppercase(Path);
    DirPathUpper := Uppercase(DirPath);
    NewPath      := Path;
    P := Pos(Uppercase(DirPath) + ';', PathUpper);
    if P > 0 then
      Delete(NewPath, P, Length(DirPath) + 1)
    else
    begin
      P := Pos(';' + Uppercase(DirPath), PathUpper);
      if P > 0 then
        Delete(NewPath, P, Length(DirPath) + 1)
      else
        if PathUpper = DirPathUpper then
          NewPath := '';
    end;
    while Pos(';;', NewPath) > 0 do
      StringChangeEx(NewPath, ';;', ';', True);
    if (Length(NewPath) > 0) and (NewPath[1] = ';') then
      Delete(NewPath, 1, 1);
    if (Length(NewPath) > 0) and (NewPath[Length(NewPath)] = ';') then
      Delete(NewPath, Length(NewPath), 1);
    if NewPath <> Path then
    begin
      if RegWriteStringValue(RegKey, 'Environment', 'Path', NewPath) then
        Log('Successfully removed from PATH: ' + DirPath)
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
    if WizardIsComponentSelected('ccalc\std') or
       WizardIsComponentSelected('ccalc\dll') then
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

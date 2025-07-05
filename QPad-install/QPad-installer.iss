; ————————————————————————————————————————————————————————————————————————————————————————————————
; QPadInstaller.iss – Instalador con clone vs pull y acceso directo dinámico
; ————————————————————————————————————————————————————————————————————————————————————————————————

[Setup]
AppName=QPad
AppVersion=1.1.0
DefaultDirName={pf}\QPad
DefaultGroupName=QPad
OutputDir=.
OutputBaseFilename=QPadInstaller
Compression=lzma
SolidCompression=yes
SetupIconFile=QPadres\Ico\icon.ico

[Languages]
Name: "default"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "Git.zip"; DestDir: "{tmp}"; Flags: ignoreversion

[Registry]
Root: HKCU; Subkey: "Software\QPad"; ValueType: string; ValueName: "RepoDir"; ValueData: "{code:GetRepoDir}"; Flags: createvalueifdoesntexist uninsdeletekey

[Icons]
Name: "{group}\QPad"; Filename: "{app}\qpad\QPad.exe"; WorkingDir: "{app}\qpad"; IconFilename: "{app}\Ico\icoqpad.ico"; IconIndex: 0; Tasks: desktopicon
Name: "{group}\Desinstalar QPad"; Filename: "{uninstallexe}"

[Tasks]
Name: desktopicon; Description: "Crear acceso directo en el Escritorio"; GroupDescription: "Tareas opcionales:"; Flags: unchecked
Name: cleaninstall; Description: "Realizar instalación limpia (borrar carpeta previa)"; GroupDescription: "Tareas opcionales:"; Flags: unchecked

[UninstallDelete]
Type: filesandordirs; Name: "{reg:HKCU\\Software\\QPad,RepoDir}"

[Code]
var
  DirPage: TInputDirWizardPage;
  RepoDir: string;
  ResultCode: Integer;

function GetRepoDir(Value: string): string;
begin
  Result := RepoDir;
end;

procedure InitializeWizard();
begin
  DirPage := CreateInputDirPage(
    wpSelectDir,
    '¿Dónde quieres clonar el código de QPad?',
    'Selecciona la carpeta donde se guardará el repositorio:',
    'Dentro de ella se creará la carpeta "qpad".',
    False,
    ''
  );
  DirPage.Add('');
  DirPage.Values[0] := WizardForm.DirEdit.Text;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  GitDir, GitExe, UnzipCmd, ExePath: string;
begin
  if CurStep <> ssPostInstall then Exit;

  RepoDir := AddBackslash(DirPage.Values[0]) + 'qpad_repo';

  GitDir := ExpandConstant('{tmp}');
UnzipCmd := 'powershell -NoProfile -InputFormat None -Command "& { Add-Type -AssemblyName ''System.IO.Compression.FileSystem''; [IO.Compression.ZipFile]::ExtractToDirectory(''' + ExpandConstant('{tmp}') + '\\Git.zip'', ''' + GitDir + ''') }"';
  Exec('cmd.exe', '/k "' + UnzipCmd + ' & echo. & echo Presioná ENTER para salir... & pause >nul"', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);

  GitExe := GitDir + '\Git\Cmd\git.exe';
  if not FileExists(GitExe) then
  begin
    MsgBox('Error: git.exe no se encontró en:'#13#10 + GitExe, mbCriticalError, MB_OK);
    Exit;
  end;

  if IsTaskSelected('cleaninstall') then
  begin
    if DirExists(RepoDir) then
      DelTree(RepoDir, True, True, True);
  end;
  
  if DirExists(RepoDir + '\.git') then
    Exec('cmd.exe', '/k ""' + GitExe + '" -C "' + RepoDir + '" pull & echo. & echo Presioná ENTER para salir..."', '', SW_SHOW, ewWaitUntilTerminated, ResultCode)
  else
    DelTree(RepoDir, True, True, True);
    Exec('cmd.exe', '/k ""' + GitExe + '" clone https://github.com/Soyzian/qpad-raw "' + RepoDir + '" & echo. & echo Presioná ENTER para salir..."', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);

  ExePath := RepoDir + '\qpad\QPad.exe';
  if not FileExists(ExePath) then
  begin
    MsgBox('Error: no se encontró QPad.exe en:'#13#10 + ExePath, mbCriticalError, MB_OK);
    Exit;
  end;

  if FileExists(ExpandConstant('{userdesktop}\QPad.lnk')) then
    DeleteFile(ExpandConstant('{userdesktop}\QPad.lnk'));
  CreateShellLink(
    ExpandConstant('{userdesktop}\QPad.lnk'),
    '',
    ExePath,
    '',
    ExtractFileDir(ExePath),
    ExpandConstant('{app}\Ico\icoqpad.ico'),
    0,
    SW_SHOWNORMAL
  );

  MsgBox('¡QPad instalado correctamente!'#13#10 + 'Ejecutable: ' + ExePath, mbInformation, MB_OK);
end;

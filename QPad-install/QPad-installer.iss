; QPadInstaller.iss

[Setup]
AppName=QPad
AppVersion=1.0
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
; Solo empaquetamos Git.zip, el resto lo clona Git Portable
Source: "Git.zip"; DestDir: "{tmp}"; Flags: ignoreversion
Source: "QPadres\Ico\icoqpad.ico"; DestDir: "{app}\Ico"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; No definimos iconos estáticos aqui; se crearán dinámicamente en [Code]

[Code]
var
  RepoDir: string;
  ResultCode: Integer;

function GetUserClonePath(): string;
begin
  Result := '';
  if BrowseForFolder('Seleccionar carpeta de destino para QPad', Result, False) then
    Result := AddBackslash(Result) + 'qpad';
end;

procedure InitializeWizard();
begin
  RepoDir := GetUserClonePath();
  if RepoDir = '' then
    WizardForm.Close; // Usuario canceló
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  GitDir, GitExe, UnzipCmd, CmdScriptPath, ExePath: string;
  SL: TStringList;
begin
  if CurStep <> ssPostInstall then
    Exit;

  // 1) Descomprimir Git Portable
  GitDir := ExpandConstant('{tmp}');
  GitExe := GitDir + '\Git\Cmd\git.exe';
  MsgBox('Descomprimiendo Git Portable...', mbInformation, MB_OK);
  UnzipCmd := 'powershell -Command "Expand-Archive -Path ''' +
              ExpandConstant('{tmp}\Git.zip') +
              ''' -DestinationPath ''' + GitDir +
              ''' -Force"';
  Exec('cmd.exe', '/c ' + UnzipCmd, '', SW_SHOW, ewWaitUntilTerminated, ResultCode);

  // 2) Verificar git.exe
  if not FileExists(GitExe) then
  begin
    MsgBox('Error: git.exe no se encontró en:'#13#10 + GitExe,
           mbCriticalError, MB_OK);
    Exit;
  end;

  // 3) Crear script temporal para el clone
  CmdScriptPath := ExpandConstant('{tmp}\runclone.cmd');
  SL := TStringList.Create;
  try
    SL.Add('@echo off');
    SL.Add('echo Clonando QPad desde GitHub...');
    SL.Add('"' + GitExe + '" clone https://github.com/Soyzian/qpad-raw "' + RepoDir + '"');
    SL.Add('pause');
    SL.SaveToFile(CmdScriptPath);
  finally
    SL.Free;
  end;

  // 4) Ejecutar el script visible
  MsgBox('Se abrirá una ventana para clonar el repositorio. Esperá a que termine.',
         mbInformation, MB_OK);
  Exec('cmd.exe', '/c "' + CmdScriptPath + '"', '', SW_SHOWNORMAL,
       ewWaitUntilTerminated, ResultCode);

  // 5) Verificar QPad.exe
  ExePath := RepoDir + '\qpad\QPad.exe';
  if not FileExists(ExePath) then
  begin
    MsgBox('Error: no se encontró QPad.exe en:'#13#10 + ExePath,
           mbCriticalError, MB_OK);
    Exit;
  end;

  // 6) Crear acceso directo en Escritorio
  CreateShellLink(
    ExpandConstant('{userdesktop}\QPad.lnk'),
    '',
    ExePath,
    '',                               // Sin parámetros
    ExtractFileDir(ExePath),          // WorkingDir
    ExpandConstant('{app}\Ico\iconqpad.ico'), // Icono
    0,                                // IconIndex
    SW_SHOWNORMAL                     // ShowCmd
  );
  
  if not FileExists(ExpandConstant('{app}\QPadres\Ico\icoqpad.ico')) then
  MsgBox('¡Error: no existe el icono en {app}\QPadres\Ico\iconqpad.ico!', mbError, MB_OK);

  MsgBox('QPad se instaló correctamente en:'#13#10 + ExePath,
         mbInformation, MB_OK);
end;

; KeyboardToGamepad.iss
; Script de Inno Setup 6 para crear el instalador completo.
; Requiere: Inno Setup 6.2+, ViGEmBus MSI en installer/vigem/

#define MyAppName "KeyboardToGamepad"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "KeyboardToGamepad Contributors"
#define MyAppURL "https://github.com/tu-usuario/keyboard-to-gamepad"
#define MyAppExeName "KeyboardToGamepad.exe"

[Setup]
AppId={{7A2B3C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
OutputDir=..
OutputBaseFilename=Setup_{#MyAppName}_{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "autostart"; Description: "Iniciar con Windows"; GroupDescription: "Opciones de inicio"; Flags: unchecked

[Files]
; Ejecutable principal
Source: "..\build\bin\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Perfiles por defecto
Source: "..\config\default_profiles.json"; DestDir: "{commonappdata}\{#MyAppName}"; Flags: ignoreversion

; ViGEmBus MSI (descargar manualmente y colocar aquí)
Source: "vigem\ViGEmBusSetup_x64.msi"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: NeedsViGEmBus

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Auto-start opcional
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: "\"{app}\{#MyAppExeName}\""; Tasks: autostart

[Run]
; Instalar ViGEmBus si no está presente
Filename: "msiexec.exe"; Parameters: "/i ""{tmp}\ViGEmBusSetup_x64.msi"" /qn /norestart"; StatusMsg: "Instalando ViGEmBus..."; Check: NeedsViGEmBus

Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Desinstalador opt-in de ViGEmBus
Filename: "msiexec.exe"; Parameters: "/x {tmp}\ViGEmBusSetup_x64.msi /qn"; StatusMsg: "Desinstalando ViGEmBus..."; Check: UninstallViGEmBus

[Code]
function NeedsViGEmBus: Boolean;
begin
  Result := not RegKeyExists(HKLM, 'SYSTEM\CurrentControlSet\Services\ViGEmBus');
end;

function UninstallViGEmBus: Boolean;
begin
  Result := MsgBox('¿Desinstalar también ViGEmBus?\n\nOtros programas pueden depender de él. Es recomendable dejarlo instalado.', mbConfirmation, MB_YESNO) = IDYES;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    MsgBox('Gracias por usar KeyboardToGamepad.\n\nTus perfiles personales se conservaron en %APPDATA%\{#MyAppName}.', mbInformation, MB_OK);
  end;
end;

; NatLang Studio Easy portable distribution -> single installer EXE.
; Run ISCC.exe after the package payload was assembled by GitHub Actions.
#define AppName "NatLang Studio Easy"
#define AppVersion "0.5.1"
[Setup]
AppId={{944F015B-8BA9-4C05-928B-5AE8A926EC5E}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Gabriele Viola and NatLang contributors
AppPublisherURL=https://github.com/gabrieleviolait/natlang
DefaultDirName={localappdata}\Programs\NatLang Studio
DefaultGroupName=NatLang Studio
PrivilegesRequired=lowest
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\natlang-studio.exe
OutputDir=out
OutputBaseFilename=NatLang-Studio-Easy-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
LicenseFile=payload\LICENSE
[Files]
Source: "payload\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\NatLang Studio"; Filename: "{app}\natlang-studio.exe"
Name: "{autodesktop}\NatLang Studio"; Filename: "{app}\natlang-studio.exe"; Tasks: desktopicon
[Tasks]
Name: "desktopicon"; Description: "Crea collegamento sul desktop"; GroupDescription: "Collegamenti:"; Flags: unchecked
[Run]
Filename: "{app}\natlang-studio.exe"; Description: "Apri NatLang Studio"; Flags: nowait postinstall skipifsilent

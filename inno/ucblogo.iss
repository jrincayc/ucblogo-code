; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Berkeley Logo
AppVerName=Berkeley Logo 6.1
AppVersion=6.1
AppPublisher=University of California, Berkeley
AppPublisherURL=http://www.cs.berkeley.edu/~bh/logo.html
AppSupportURL=https://github.com/jrincayc/ucblogo-code/issues
AppUpdatesURL=https://github.com/jrincayc/ucblogo-code/releases
OutputBaseFilename=ucblogo61setup
OutputDir=.
DefaultDirName={autopf}\UCBLogo
DefaultGroupName=Berkeley Logo
DisableStartupPrompt=yes
DisableProgramGroupPage=yes
AllowNoIcons=yes
WindowStartMaximized=no
LicenseFile=..\LICENSE
PrivilegesRequiredOverridesAllowed=dialog commandline

[Components]
Name: "program"; Description: "Program Files"; Types: full compact custom; Flags: fixed
Name: "help"; Description: "Help Files"; Types: full compact custom
Name: "csls"; Description: "Programs from Computer Science Logo Style"; Types: full compact custom
Name: "pdf"; Description: "User Manual in PDF format"; Types: full custom
Name: "source"; Description: "Source Files"; Types: full custom

[Tasks]
Name: "programmenu"; Description: "Create a Program menu entry"; GroupDescription: "Shortcuts:"
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Shortcuts:"; MinVersion: 4,4
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Shortcuts:"; MinVersion: 4,4; Flags: unchecked


[Files]
Source: "C:\ucblogo\bin\ucblogo.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: program
Source: "C:\ucblogo\bin\*.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: program
Source: "C:\UCBLOGO\lib\logo\CSLS\*.*"; DestDir: "{app}\CSLS"; CopyMode: alwaysoverwrite; Components: csls
Source: "C:\UCBLOGO\lib\logo\HELPFILES\*.*"; DestDir: "{app}\HELPFILE"; CopyMode: alwaysoverwrite; Components: help
Source: "C:\UCBLOGO\lib\logo\LOGOLIB\*.*"; DestDir: "{app}\LOGOLIB"; CopyMode: alwaysoverwrite; Components: program
Source: "C:\UCBLOGO\lib\logo\LICENSE"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: program
Source: "C:\UCBLOGO\lib\logo\README.txt"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: program
Source: "C:\UCBLOGO\lib\logo\usermanual.pdf"; DestDir: "{app}\DOCS"; CopyMode: alwaysoverwrite; Components: pdf
Source: "C:\UCBLOGO\lib\logo\SOURCE\*.*"; DestDir: "{app}\SOURCE"; CopyMode: alwaysoverwrite; Components: source

[Icons]
Name: "{group}\Berkeley Logo"; Filename: "{app}\ucblogo.exe"; WorkingDir: "{app}"; Tasks: programmenu
Name: "{autodesktop}\Berkeley Logo"; Filename: "{app}\ucblogo.exe"; WorkingDir: "{app}"; MinVersion: 4,4; Tasks: desktopicon

[Registry]
Root: HKA; Subkey: "Software\UCB"; Flags: uninsdeletekeyifempty
Root: HKA; Subkey: "Software\UCB\UCBLogo"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\UCB\UCBLogo"; ValueType: string; ValueName: "LOGOLIB"; ValueData: "{app}\LOGOLIB"
Root: HKA; Subkey: "Software\UCB\UCBLogo"; ValueType: string; ValueName: "HELPFILE"; ValueData: "{app}\HELPFILE"
Root: HKA; Subkey: "Software\UCB\UCBLogo"; ValueType: string; ValueName: "CSLS"; ValueData: "{app}\CSLS"

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nThis installer was created with the freeware Inno Setup Compiler by Jordan Russell with portions by Martijn Laan.%nhttp://www.jrsoftware.org/isinfo.php
LicenseLabel=UCBLogo is free, and has NO WARRANTY.  (See sections 15 and 16 below.)%nThe other license provisions are only about distributing Logo to other people.
ComponentsDiskSpaceMBLabel=Current selection requires at least [kb] KB of disk space.

[Run]
Filename: "notepad.exe"; Parameters: "{app}\README"; Description: "View README file"; Flags: postinstall skipifsilent
Filename: "{app}\ucblogo.exe"; WorkingDir: "{app}"; Description: "Launch Berkeley Logo"; Flags: postinstall skipifsilent
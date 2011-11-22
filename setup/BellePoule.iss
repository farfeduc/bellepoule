; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#include "..\sources\version.h"

[Setup]
AppName=BellePoule
AppVerName=BellePoule version {#VERSION}.{#VERSION_REVISION}{#VERSION_MATURITY}
OutputBaseFilename=setup{#VERSION}_{#VERSION_REVISION}{#VERSION_MATURITY}
AppPublisher=betton.escrime
AppPublisherURL=http://betton.escrime.free.fr/
AppSupportURL=http://betton.escrime.free.fr/
AppUpdatesURL=http://betton.escrime.free.fr/
UsePreviousAppDir=no
DefaultDirName={code:GetInstallDir}\BellePoule
DefaultGroupName=BellePoule
AllowNoIcons=yes
LicenseFile=COPYING.txt
Compression=lzma
SolidCompression=yes
ChangesEnvironment=yes
ChangesAssociations=true
PrivilegesRequired=none
WizardImageFile=BellePoule.bmp
WizardSmallImageFile=BellePoule_small.bmp
SetupIconFile=setup_logo.ico

[Tasks]
Name: desktopicon; Description: "Cr�er un icone sur le bureau"; GroupDescription: "Icones de lancement :";
Name: quicklaunchicon; Description: "Cr�er une icone de lancement rapide dans la barre des t�ches"; GroupDescription: "Icones de lancement :";
;Name: downloadsources; Description: "R�cup�rer le code source"; GroupDescription: "Code source :"; Flags: unchecked;

[Registry]
Root: HKCU; Subkey: "Software\Classes\.cotcot"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\BellePoule"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\BellePoule\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\BellePoule.exe,0"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\BellePoule\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\BellePoule.bat"" ""%1"""; Flags: uninsdeletekey noerror

Root: HKLM; Subkey: "Software\Classes\.cotcot"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\BellePoule"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\BellePoule\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\BellePoule.exe,0"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\BellePoule\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\BellePoule.bat"" ""%1"""; Flags: uninsdeletekey noerror

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
;LicenseFile: "license-French.txt"

[Icons]
Name: "{userprograms}\BellePoule\BellePoule"; Filename: "{app}\BellePoule.bat"; WorkingDir: "{app}"; IconFilename: "{app}\BellePoule.exe"
Name: "{userprograms}\BellePoule\Uninstall BellePoule"; Filename: "{uninstallexe}"; WorkingDir: "{app}"
Name: "{commonprograms}\BellePoule\BellePoule"; Filename: "{app}\BellePoule.bat"; WorkingDir: "{app}"; IconFilename: "{app}\BellePoule.exe"
Name: "{commonprograms}\BellePoule\Uninstall BellePoule"; Filename: "{uninstallexe}"; WorkingDir: "{app}"
Name: "{userdesktop}\BellePoule"; Filename: "{app}\BellePoule.bat"; WorkingDir: "{app}"; Tasks: desktopicon; IconFilename: "{app}\BellePoule.exe"
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\BellePoule"; Filename: "{app}\BellePoule.bat"; WorkingDir: "{app}"; Tasks: quicklaunchicon; IconFilename: "{app}\BellePoule.exe"

[Dirs]
Name: "{app}\porting_layer"; Attribs: hidden
Name: "{app}\resources"; Attribs: hidden

[Files]
Source: "path_dependent_files\BellePoule.bat"; DestDir: "{app}"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\BellePoule.bat'), 'INSTALL_DIR', ExpandConstant('{app}'), 1)
Source: "..\bin\Release\BellePoule.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "gtkrc"; DestDir: "{app}\resources"; Flags: ignoreversion
Source: "path_dependent_files\gtk.immodules"; DestDir: "{app}\porting_layer\etc\gtk-2.0"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\porting_layer\etc\gtk-2.0\gtk.immodules'), 'INSTALL_DIR', ExpandConstant('{app}'), 0)
Source: "path_dependent_files\gdk-pixbuf.loaders"; DestDir: "{app}\porting_layer\etc\gtk-2.0"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\porting_layer\etc\gtk-2.0\gdk-pixbuf.loaders'), 'INSTALL_DIR', ExpandConstant('{app}'), 0)
Source: "path_dependent_files\pango.modules"; DestDir: "{app}\porting_layer\etc\pango"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\porting_layer\etc\pango\pango.modules'), 'INSTALL_DIR', ExpandConstant('{app}'), 0)

;Exemple de fichiers
Source: "..\Exemples\exemple.cotcot"; DestDir: "{app}\Exemples"; Flags: ignoreversion
Source: "..\Exemples\CLS_SHM.FFF"; DestDir: "{app}\Exemples"; Flags: ignoreversion
Source: "..\Exemples\CLS_EDM.FFF"; DestDir: "{app}\Exemples"; Flags: ignoreversion

;Documentation
Source: "..\resources\user_manual.pdf"; DestDir: "{app}\resources"; Flags: ignoreversion

;Traductions
Source: "..\resources\translations\fr\*"; DestDir: "{app}\resources\translations\fr"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\de\*"; DestDir: "{app}\resources\translations\de"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\nl\*"; DestDir: "{app}\resources\translations\nl"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\ru\*"; DestDir: "{app}\resources\translations\ru"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\ar\*"; DestDir: "{app}\resources\translations\ar"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\es\*"; DestDir: "{app}\resources\translations\es"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\it\*"; DestDir: "{app}\resources\translations\it"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\ko\*"; DestDir: "{app}\resources\translations\ko"; Flags: ignoreversion recursesubdirs

Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\de\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\de\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\de\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\de\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\de\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\de\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\de\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\de\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\es\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\es\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\es\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\es\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\es\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\es\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\es\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\es\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\it\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\it\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\it\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\it\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\it\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\it\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\it\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\it\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\atk10.mo"; DestDir: "{app}\porting_layer\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\glib20.mo"; DestDir: "{app}\porting_layer\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\porting_layer\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\porting_layer\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion

;Resources
Source: "..\resources\glade\*.png"; DestDir: "{app}\resources\glade\"; Flags: ignoreversion
Source: "..\resources\glade\*.jpg"; DestDir: "{app}\resources\glade\"; Flags: ignoreversion
Source: "..\resources\glade\*.glade"; DestDir: "{app}\resources\glade\"; Flags: ignoreversion
Source: "..\resources\countries\*"; DestDir: "{app}\resources\countries\"; Flags: ignoreversion recursesubdirs
Source: "..\resources\exe.ico"; DestDir: "{app}\resources\"; Flags: ignoreversion

Source: "..\resources\data\*"; DestDir: "{app}\resources\data\"; Flags: ignoreversion  recursesubdirs

;Sources
;Source: "..\BellePoule.cbp"; DestDir: "{app}"; Tasks: downloadsources; Flags: ignoreversion
;Source: "..\sources\*.hpp"; DestDir: "{app}\sources\"; Tasks: downloadsources; Flags: ignoreversion
;Source: "..\sources\*.h"; DestDir: "{app}\sources\"; Tasks: downloadsources; Flags: ignoreversion
;Source: "..\sources\*.cpp"; DestDir: "{app}\sources\"; Tasks: downloadsources; Flags: ignoreversion
;Source: "..\sources\resource.rc"; DestDir: "{app}\sources\"; Tasks: downloadsources; Flags: ignoreversion

; GTK+ dependencies
; DLL
Source: "C:\MinGW\bin\libcairo-2.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpangocairo-1.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\jpeg62.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libtiff3.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
;Source: "C:\MinGW\bin\libpng14-14.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpng12.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpng12-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgio-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\zlib1.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\intl.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libatk-1.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgdk-win32-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libglib-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgmodule-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgobject-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgthread-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgtk-win32-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpango-1.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpangoft2-1.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpangowin32-1.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libxml2.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\iconv.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgoocanvas-3.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgdk_pixbuf-2.0-0.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\bzip2.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\freetype6.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libcroco-0.6-3.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libfontconfig-1.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libfreetype-6.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgsf-1-114.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\librsvg-2-2.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion

Source: "C:\MinGW\bin\libcurl.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libssh2.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libeay32.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libidn-11.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\librtmp.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libssh2.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion
Source: "C:\MinGW\bin\libssl32.dll"; DestDir: "{app}\porting_layer\lib"; Flags: ignoreversion

; theme
Source: "C:\MinGW\etc\gtk-2.0\gtkrc"; DestDir: "{app}\porting_layer\etc\gtk-2.0"; Flags: ignoreversion
Source: "C:\MinGW\share\themes\Aurora\gtk-2.0\gtkrc"; DestDir: "{app}\porting_layer\share\themes\Aurora\gtk-2.0"; Flags: ignoreversion
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\engines\libaurora.dll"; DestDir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\engines\"; Flags: ignoreversion

; icons
Source: "C:\MinGW\share\icons\hicolor\index.theme"; DestDir: "{app}\porting_layer\share\icons\hicolor\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\16x16\apps\gnome-devel.png"; DestDir: "{app}\porting_layer\share\icons\hicolor\16x16\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\24x24\apps\gnome-devel.png"; DestDir: "{app}\porting_layer\share\icons\hicolor\24x24\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\32x32\apps\gnome-devel.png"; DestDir: "{app}\porting_layer\share\icons\hicolor\32x32\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\48x48\apps\gnome-devel.png"; DestDir: "{app}\porting_layer\share\icons\hicolor\48x48\apps\"; Flags: ignoreversion

;
Source: "C:\MinGW\etc\pango\pango.aliases"; Destdir: "{app}\porting_layer\etc\pango"

; loaders
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\loaders\*"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\loaders"

; optional: let the user make the app look more Windows-like
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\engines\libwimp.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\engines"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\engines\libpixmap.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\engines"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-am-et.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-cedilla.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-cyrillic-translit.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ime.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-inuktitut.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ipa.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-multipress.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-thai.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ti-er.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ti-et.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-viqr.dll"; Destdir: "{app}\porting_layer\lib\gtk-2.0\2.10.0\immodules"

[UninstallDelete]
Type: filesandordirs; Name: "{app}\porting_layer"

[Code]
//////////////////////////////////////////
function GetInstallDir(Param: String): String;
var pfdir, tmpdir: string;
begin
  pfdir  := ExpandConstant('{pf}');
  tmpdir := pfdir + '\MY_PROGRAM_TMP';

  if CreateDir(tmpdir) then
  begin
    RemoveDir(tmpdir);
    Result := pfdir;
  end
  else begin
    Result := ExpandConstant('{userdesktop}');
  end
end;

//////////////////////////////////////////
procedure UpdatePath (const file: String; const what: String; const by: String; const dos_separator: Integer);
var
  i     : Integer;
  lines : TArrayOfString;
begin

  begin
   if dos_separator = 0
    then StringChange(by, '\', '/');
  end;

  LoadStringsFromFile(file, lines);

  for i := 0 to GetArrayLength(lines)-1 do
    begin
      StringChange(lines[i], what, by);
    end;

  SaveStringsToFile(file, lines, False);
end;

[Run]
;Filename: "{app}\BellePoule.exe"; Description: "{cm:LaunchProgram,BellePoule}"; Flags: waituntilterminated postinstall skipifsilent






















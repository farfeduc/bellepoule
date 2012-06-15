; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#include "..\sources\common\version.h"

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
Name: desktopicon; Description: "Cr�er un icone sur le bureau"; GroupDescription: "Ic�nes de lancement :";
Name: quicklaunchicon; Description: "Cr�er une icone de lancement rapide dans la barre des t�ches"; GroupDescription: "Icones de lancement :";
;Name: downloadsources; Description: "R�cup�rer le code source"; GroupDescription: "Code source :"; Flags: unchecked;

[Registry]
Root: HKCU; Subkey: "Software\Classes\.cotcot"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\.fff"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\BellePoule"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\BellePoule\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\BellePoule.exe,0"; Flags: uninsdeletekey noerror
Root: HKCU; Subkey: "Software\Classes\BellePoule\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\BellePoule.exe"" ""%1"""; Flags: uninsdeletekey noerror

Root: HKLM; Subkey: "Software\Classes\.cotcot"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\.fff"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\BellePoule"; ValueType: string; ValueName: ""; ValueData: "BellePoule"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\BellePoule\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\BellePoule.exe,0"; Flags: uninsdeletekey noerror
Root: HKLM; Subkey: "Software\Classes\BellePoule\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\BellePoule.bin"" ""%1"""; Flags: uninsdeletekey noerror

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
;LicenseFile: "license-French.txt"

[Icons]
Name: "{userprograms}\BellePoule\BellePoule"; Filename: "{app}\bin\BellePoule.exe"; IconFilename: "{app}\bin\BellePoule.exe"
Name: "{userprograms}\BellePoule\Uninstall BellePoule"; Filename: "{uninstallexe}"
Name: "{commonprograms}\BellePoule\BellePoule"; Filename: "{app}\bin\BellePoule.exe"; IconFilename: "{app}\bin\BellePoule.exe"
Name: "{commonprograms}\BellePoule\Uninstall BellePoule"; Filename: "{uninstallexe}"
Name: "{userdesktop}\BellePoule"; Filename: "{app}\bin\BellePoule.exe"; Tasks: desktopicon; IconFilename: "{app}\bin\BellePoule.exe"
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\BellePoule"; Filename: "{app}\bin\BellePoule.exe"; Tasks: quicklaunchicon; IconFilename: "{app}\bin\BellePoule.exe"

[Files]
Source: "..\bin\Release\BellePoule.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\resources\gtkrc"; DestDir: "{app}\share\BellePoule\resources"; Flags: ignoreversion
Source: "path_dependent_files\gtk.immodules"; DestDir: "{app}\etc\gtk-2.0"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\etc\gtk-2.0\gtk.immodules'), 'INSTALL_DIR', ExpandConstant('{app}'), 0)
Source: "path_dependent_files\gdk-pixbuf.loaders"; DestDir: "{app}\etc\gtk-2.0"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\etc\gtk-2.0\gdk-pixbuf.loaders'), 'INSTALL_DIR', ExpandConstant('{app}'), 0)
Source: "path_dependent_files\pango.modules"; DestDir: "{app}\etc\pango"; Flags: ignoreversion; AfterInstall: UpdatePath(ExpandConstant('{app}\etc\pango\pango.modules'), 'INSTALL_DIR', ExpandConstant('{app}'), 0)

;Exemple de fichiers
Source: "..\Exemples\exemple.cotcot"; DestDir: "{app}\share\BellePoule\Exemples"; Flags: ignoreversion
Source: "..\Exemples\CLS_SHM.FFF"; DestDir: "{app}\share\BellePoule\Exemples"; Flags: ignoreversion
Source: "..\Exemples\CLS_EDM.FFF"; DestDir: "{app}\share\BellePoule\Exemples"; Flags: ignoreversion
Source: "..\Exemples\FFE\*"; DestDir: "{app}\share\BellePoule\Exemples\FFE"; Flags: ignoreversion

;Documentation
Source: "..\resources\user_manual.pdf"; DestDir: "{app}\share\BellePoule\resources"; Flags: ignoreversion

;Traductions
Source: "..\resources\translations\index.txt"; DestDir: "{app}\share\BellePoule\resources\translations"; Flags: ignoreversion

Source: "..\resources\translations\fr\*"; DestDir: "{app}\share\BellePoule\resources\translations\fr"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\de\*"; DestDir: "{app}\share\BellePoule\resources\translations\de"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\nl\*"; DestDir: "{app}\share\BellePoule\resources\translations\nl"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\ru\*"; DestDir: "{app}\share\BellePoule\resources\translations\ru"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\ar\*"; DestDir: "{app}\share\BellePoule\resources\translations\ar"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\es\*"; DestDir: "{app}\share\BellePoule\resources\translations\es"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\it\*"; DestDir: "{app}\share\BellePoule\resources\translations\it"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\ko\*"; DestDir: "{app}\share\BellePoule\resources\translations\ko"; Flags: ignoreversion recursesubdirs
Source: "..\resources\translations\pt_br\*"; DestDir: "{app}\share\BellePoule\resources\translations\pt_br"; Flags: ignoreversion recursesubdirs

Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\fr\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\fr\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\nl\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\nl\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\de\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\de\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\de\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\de\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\de\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\de\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\de\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\de\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ru\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\ru\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ar\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\ar\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\es\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\es\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\es\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\es\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\es\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\es\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\es\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\es\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\it\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\it\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\it\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\it\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\it\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\it\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\it\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\it\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\ko\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\ko\LC_MESSAGES"; Flags: ignoreversion

Source: "C:\MinGW\share\locale\pt\LC_MESSAGES\atk10.mo"; DestDir: "{app}\share\locale\pt\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\pt\LC_MESSAGES\glib20.mo"; DestDir: "{app}\share\locale\pt\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\pt\LC_MESSAGES\gtk20.mo"; DestDir: "{app}\share\locale\pt\LC_MESSAGES"; Flags: ignoreversion
Source: "C:\MinGW\share\locale\pt\LC_MESSAGES\gtk20-properties.mo"; DestDir: "{app}\share\locale\pt\LC_MESSAGES"; Flags: ignoreversion

;Resources
Source: "..\resources\glade\*.png"; DestDir: "{app}\share\BellePoule\resources\glade\"; Flags: ignoreversion
Source: "..\resources\glade\*.jpg"; DestDir: "{app}\share\BellePoule\resources\glade\"; Flags: ignoreversion
Source: "..\resources\glade\*.glade"; DestDir: "{app}\share\BellePoule\resources\glade\"; Flags: ignoreversion
Source: "..\resources\countries\*"; DestDir: "{app}\share\BellePoule\resources\countries\"; Flags: ignoreversion recursesubdirs
Source: "..\resources\exe.ico"; DestDir: "{app}\share\BellePoule\resources\"; Flags: ignoreversion

Source: "..\resources\data\*"; DestDir: "{app}\share\BellePoule\resources\data\"; Flags: ignoreversion  recursesubdirs

; GTK+ dependencies
; DLL
Source: "C:\MinGW\bin\libcairo-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpangocairo-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\jpeg62.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libtiff3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
;Source: "C:\MinGW\bin\libpng14-14.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpng12.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpng12-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgio-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\zlib1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\intl.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libatk-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgdk-win32-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libglib-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgmodule-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgobject-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgthread-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgtk-win32-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpango-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpangoft2-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libpangowin32-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\iconv.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgoocanvas-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgdk_pixbuf-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\bzip2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\freetype6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libcroco-0.6-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libfontconfig-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libfreetype-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgsf-1-114.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\librsvg-2-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

Source: "C:\MinGW\bin\libcurl.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libssh2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libeay32.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libidn-11.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\librtmp.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libssh2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libssl32.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libxml2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libmicrohttpd-10.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgnutls-28.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgmp.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libintl-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libiconv-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\pthreadGC2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgcrypt-11.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libgpg-error-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\MinGW\bin\libplibc-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

; theme
Source: "C:\MinGW\etc\gtk-2.0\gtkrc"; DestDir: "{app}\etc\gtk-2.0"; Flags: ignoreversion
Source: "C:\MinGW\share\themes\Aurora\gtk-2.0\gtkrc"; DestDir: "{app}\share\themes\Aurora\gtk-2.0"; Flags: ignoreversion
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\engines\libaurora.dll"; DestDir: "{app}\lib\gtk-2.0\2.10.0\engines\"; Flags: ignoreversion

; icons
Source: "C:\MinGW\share\icons\hicolor\index.theme"; DestDir: "{app}\share\icons\hicolor\"; Flags: ignoreversion

Source: "C:\MinGW\share\icons\hicolor\16x16\apps\gnome-devel.png"; DestDir: "{app}\share\icons\hicolor\16x16\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\24x24\apps\gnome-devel.png"; DestDir: "{app}\share\icons\hicolor\24x24\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\32x32\apps\gnome-devel.png"; DestDir: "{app}\share\icons\hicolor\32x32\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\48x48\apps\gnome-devel.png"; DestDir: "{app}\share\icons\hicolor\48x48\apps\"; Flags: ignoreversion

Source: "C:\MinGW\share\icons\hicolor\16x16\apps\preferences-desktop-theme.png"; DestDir: "{app}\share\icons\hicolor\16x16\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\24x24\apps\preferences-desktop-theme.png"; DestDir: "{app}\share\icons\hicolor\24x24\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\32x32\apps\preferences-desktop-theme.png"; DestDir: "{app}\share\icons\hicolor\32x32\apps\"; Flags: ignoreversion

Source: "C:\MinGW\share\icons\hicolor\16x16\apps\preferences-desktop-locale.png"; DestDir: "{app}\share\icons\hicolor\16x16\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\24x24\apps\preferences-desktop-locale.png"; DestDir: "{app}\share\icons\hicolor\24x24\apps\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\32x32\apps\preferences-desktop-locale.png"; DestDir: "{app}\share\icons\hicolor\32x32\apps\"; Flags: ignoreversion

Source: "C:\MinGW\share\icons\hicolor\16x16\mimetypes\x-office-spreadsheet.png"; DestDir: "{app}\share\icons\hicolor\16x16\mimetypes\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\24x24\mimetypes\x-office-spreadsheet.png"; DestDir: "{app}\share\icons\hicolor\24x24\mimetypes\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\32x32\mimetypes\x-office-spreadsheet.png"; DestDir: "{app}\share\icons\hicolor\32x32\mimetypes\"; Flags: ignoreversion

Source: "C:\MinGW\share\icons\hicolor\16x16\status\software-update-available.png"; DestDir: "{app}\share\icons\hicolor\16x16\status\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\24x24\status\software-update-available.png"; DestDir: "{app}\share\icons\hicolor\24x24\status\"; Flags: ignoreversion
Source: "C:\MinGW\share\icons\hicolor\32x32\status\software-update-available.png"; DestDir: "{app}\share\icons\hicolor\32x32\status\"; Flags: ignoreversion

;
Source: "C:\MinGW\etc\pango\pango.aliases"; Destdir: "{app}\etc\pango"

; loaders
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\loaders\*"; Destdir: "{app}\lib\gtk-2.0\2.10.0\loaders"

; optional: let the user make the app look more Windows-like
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\engines\libwimp.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\engines"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\engines\libpixmap.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\engines"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-am-et.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-cedilla.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-cyrillic-translit.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ime.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-inuktitut.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ipa.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-multipress.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-thai.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ti-er.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-ti-et.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"
Source: "C:\MinGW\lib\gtk-2.0\2.10.0\immodules\im-viqr.dll"; Destdir: "{app}\lib\gtk-2.0\2.10.0\immodules"

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




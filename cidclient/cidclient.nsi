; $Amigan: phoned/cidclient/cidclient.nsi,v 1.1 2005/06/23 02:29:18 dcp1990 Exp $
!define PRODUCT_NAME "CIDClient"
!define PRODUCT_VERSION "0.1"
!define PRODUCT_PUBLISHER "Dan Ponte"
!define PRODUCT_WEB_SITE "http://www.theamigan.net/cid.html"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\CIDClient.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

SetCompressor bzip2

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Language Selection Dialog Settings
;!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
;!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
;!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "COPYRIGHT.rtf"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "CIDClient"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\CIDClient.exe"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"


; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "cidcSetup.exe"
InstallDir "$PROGRAMFILES\CIDClient"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show


Function DownloadFiles3
NSISdl::download http://download.microsoft.com/download/vb60pro/Redist/sp5/WIN98Me/EN-US/vbrun60sp5.exe vbrun60sp5.exe
Pop $R0 ;Get the return value
StrCmp $R0 "success" +3
FunctionEnd

Function ConnectInternet
Push $R0

ClearErrors
Dialer::AttemptConnect
IfErrors noie3

Pop $R0
StrCmp $R0 "online" connected
MessageBox MB_OK|MB_ICONSTOP "Cannot connect to the internet."
Quit

noie3:

; IE3 not installed
MessageBox MB_OK|MB_ICONINFORMATION "Please connect to the internet now. Setup needs to download and install VB runtime libraries."

connected:
Pop $R0
FunctionEnd

Section "Main" SEC01
SectionIn RO
IfFileExists "$SYSDIR\msvbvm60.dll" NoErrorMsg ErrorHere
ErrorHere:
Call ConnectInternet
Call DownloadFiles3
Exec "$SYSDIR\vbrun60sp5.exe /Q"
NoErrorMsg:
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "CIDClient.exe"
  File "Ring.wav"
  File "COPYRIGHT.txt"
  File "faxmachine.ico"
  File "fax.bmp"
  File "CIDClient.exe.MANIFEST"

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\CIDClient.lnk" "$INSTDIR\CIDClient.exe"
  CreateShortCut "$SMPROGRAMS\Startup\CIDClient.lnk" "$INSTDIR\CIDClient.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Source" SEC02
  SetOutPath "$INSTDIR\src"
  File "CIDClient.vbp"
  File "Ring.wav"
  File "modMain.bas"
  File "frmSock.frx"
  File "frmSock.frm"
  File "frmPopup.frx"
  File "frmPopup.frm"
  File "faxmachine.ico"
  File "fax.bmp"
  File "CIDClient.vbw"
  File "sysTrayMod.bas"

; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\uninst.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\CIDClient.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\CIDClient.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Main program"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "VB6 source code"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\src\sysTrayMod.bas"
  Delete "$INSTDIR\src\CIDClient.vbw"
  Delete "$INSTDIR\src\fax.bmp"
  Delete "$INSTDIR\src\faxmachine.ico"
  Delete "$INSTDIR\src\frmPopup.frm"
  Delete "$INSTDIR\src\frmPopup.frx"
  Delete "$INSTDIR\src\frmSock.frm"
  Delete "$INSTDIR\src\frmSock.frx"
  Delete "$INSTDIR\src\modMain.bas"
  Delete "$INSTDIR\src\Ring.wav"
  Delete "$INSTDIR\src\CIDClient.vbp"
  Delete "$INSTDIR\CIDClient.exe.MANIFEST"
  Delete "$INSTDIR\fax.bmp"
  Delete "$INSTDIR\faxmachine.ico"
  Delete "$INSTDIR\COPYRIGHT.txt"
  Delete "$INSTDIR\Ring.wav"
  Delete "$INSTDIR\CIDClient.exe"

  Delete "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\Website.lnk"
  Delete "$STARTMENU\Startup.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\CIDClient.lnk"

  RMDir "$SMPROGRAMS\$ICONS_GROUP"
  RMDir "$INSTDIR\src"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

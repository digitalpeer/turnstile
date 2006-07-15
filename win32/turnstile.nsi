;  $Id: turnstile.nsi,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
;
;  Turnstile
;
;  Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

; The name of the installer
Name "Turnstile"

; The file to write
OutFile "turnstilesetup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Turnstile

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Turnstile" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Turnstile Version 0.0.1"

  SetDetailsPrint textonly
  DetailPrint "Installing Turnstile..."
  SetDetailsPrint listonly

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "bin\turnstile.exe"
  File "bin\wxmsw250.dll"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Turnstile "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Turnstile" "DisplayName" "Turnstile"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Turnstile" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Turnstile" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Turnstile" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetDetailsPrint textonly
  DetailPrint "Installing shortcuts..."
  SetDetailsPrint listonly


  CreateDirectory "$SMPROGRAMS\Turnstile"
  CreateShortCut "$SMPROGRAMS\Turnstile\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Turnstile\Turnstile.lnk" "$INSTDIR\turnstile.exe" "" "$INSTDIR\turnstile.exe" 0

  CreateShortCut "$DESKTOP\Turnstile.lnk" "$INSTDIR\turnstile.exe"
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  SetDetailsPrint textonly
  DetailPrint "Uninstalling turnstile..."
  SetDetailsPrint listonly
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Turnstile"
  DeleteRegKey HKLM SOFTWARE\Turnstile

  ; Remove files and uninstaller
  Delete $INSTDIR\turnstile.exe
  Delete $INSTDIR\wxmsw250.dll
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Turnstile\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Turnstile"
  RMDir "$INSTDIR"

SectionEnd

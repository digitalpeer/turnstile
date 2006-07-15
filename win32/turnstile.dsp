# Microsoft Developer Studio Project File - Name="turnstile" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=turnstile - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "turnstile.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "turnstile.mak" CFG="turnstile - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "turnstile - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "turnstile - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "turnstile - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin"
# PROP Intermediate_Dir "debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /GX /O1 /I "C:\wxWindows\lib\mswd\\" /I "C:\wxWindows\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib winmm.lib wsock32.lib wxmsw250.lib /nologo /subsystem:windows /machine:I386 /libpath:"C:\wxWindows\lib\\"

!ELSEIF  "$(CFG)" == "turnstile - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin"
# PROP Intermediate_Dir "debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /GX /Zi /Od /I "C:\wxWindows\lib\mswd\\" /I "C:\wxWindows\include\\" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib winmm.lib wsock32.lib wxmsw250d.lib /nologo /subsystem:windows /profile /debug /machine:I386 /libpath:"C:\wxWindows\lib\\"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "turnstile - Win32 Release"
# Name "turnstile - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=\turnstile\src\aboutdialog.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\app.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\buddy.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\buddytreectrl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\clientsocket.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\connection.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\connectionthread.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\crypto.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\debugframe.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\dhtutils.cpp
# End Source File
# Begin Source File

SOURCE=..\src\eraser.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\fileframe.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\files.cpp
# End Source File
# Begin Source File

SOURCE=..\src\filetransferthread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\findbuddyframe.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\finger.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\frameutils.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\hashtable.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\hostcache.cpp
# End Source File
# Begin Source File

SOURCE=..\src\imeventqueue.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\imframe.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\lookupresponse.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\mainframe.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\network.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\packet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\packetgen.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\prefframe.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\prefs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\profiledialog.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\server.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\serversocket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\socketbase.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\typeconv.cpp
# End Source File
# Begin Source File

SOURCE=\turnstile\src\utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=\turnstile\src\aboutdialog.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\app.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\buddy.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\buddytreectrl.h
# End Source File
# Begin Source File

SOURCE=..\src\clientsocket.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\config.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\connection.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\connectionthread.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\crypto.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\debugframe.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\dhtutils.h
# End Source File
# Begin Source File

SOURCE=..\src\eraser.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\fileframe.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\files.h
# End Source File
# Begin Source File

SOURCE=..\src\filetransferthread.h
# End Source File
# Begin Source File

SOURCE=..\src\findbuddyframe.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\finger.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\frameutils.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\hashtable.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\hostcache.h
# End Source File
# Begin Source File

SOURCE=..\src\imevent.h
# End Source File
# Begin Source File

SOURCE=..\src\imeventqueue.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\imframe.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\lookupresponse.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\mainframe.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\network.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\packet.h
# End Source File
# Begin Source File

SOURCE=..\src\packetgen.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\prefframe.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\prefs.h
# End Source File
# Begin Source File

SOURCE=..\src\profiledialog.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\server.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\serversocket.h
# End Source File
# Begin Source File

SOURCE=..\src\socketbase.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\typeconv.h
# End Source File
# Begin Source File

SOURCE=\turnstile\src\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\turnstile.rc
# End Source File
# End Group
# End Target
# End Project

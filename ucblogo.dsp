# Microsoft Developer Studio Project File - Name="ucblogo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ucblogo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ucblogo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ucblogo.mak" CFG="ucblogo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ucblogo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ucblogo - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ucblogo - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Od /I "C:\wxWidgets-2.8.8\include" /I "C:\wxWidgets-2.8.8\lib\vc_lib\mswd" /D WINVER=0x400 /D wxUSE_GUI=1 /D "__WXDEBUG__" /D "HAVE_WX" /D "__WXMSW__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wxmsw28d_core.lib wxmsw28d_html.lib wxbase28d.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"C:\wxWidgets-2.8.8\lib\vc_lib"

!ELSEIF  "$(CFG)" == "ucblogo - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "C:\wxWidgets-2.8.8\include" /I "C:\wxWidgets-2.8.8\lib\vc_lib\mswd" /D WINVER=0x400 /D wxUSE_GUI=1 /D "__WXDEBUG__" /D "HAVE_WX" /D "__WXMSW__" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxmsw28d_core.lib wxmsw28d_html.lib wxbase28d.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"C:\wxWidgets-2.8.8\lib\vc_lib"

!ENDIF 

# Begin Target

# Name "ucblogo - Win32 Release"
# Name "ucblogo - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\coms.c
# End Source File
# Begin Source File

SOURCE=.\error.c
# End Source File
# Begin Source File

SOURCE=.\eval.c
# End Source File
# Begin Source File

SOURCE=.\files.c
# End Source File
# Begin Source File

SOURCE=.\graphics.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\intern.c
# End Source File
# Begin Source File

SOURCE=.\libloc.c
# End Source File
# Begin Source File

SOURCE=.\lists.c
# End Source File
# Begin Source File

SOURCE=.\logodata.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\makehelp.c
# End Source File
# Begin Source File

SOURCE=.\math.c
# End Source File
# Begin Source File

SOURCE=.\mem.c
# End Source File
# Begin Source File

SOURCE=.\obj.c
# End Source File
# Begin Source File

SOURCE=.\paren.c
# End Source File
# Begin Source File

SOURCE=.\parse.c
# End Source File
# Begin Source File

SOURCE=.\print.c
# End Source File
# Begin Source File

SOURCE=.\svn.c
# End Source File
# Begin Source File

SOURCE=.\TextEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\wrksp.c
# End Source File
# Begin Source File

SOURCE=.\wxMain.cpp
# End Source File
# Begin Source File

SOURCE=.\wxterm.c
# End Source File
# Begin Source File

SOURCE=.\wxTerminal.cpp
# End Source File
# Begin Source File

SOURCE=.\wxTurtleGraphics.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\gpl_text.h
# End Source File
# Begin Source File

SOURCE=.\logo.h
# End Source File
# Begin Source File

SOURCE=.\LogoFrame.h
# End Source File
# Begin Source File

SOURCE=.\macterm.h
# End Source File
# Begin Source File

SOURCE=.\nographics.h
# End Source File
# Begin Source File

SOURCE=.\TextEditor.h
# End Source File
# Begin Source File

SOURCE=.\win32trm.h
# End Source File
# Begin Source File

SOURCE=.\wxGlobals.h
# End Source File
# Begin Source File

SOURCE=.\wxGraphics.h
# End Source File
# Begin Source File

SOURCE=.\wxMain.h
# End Source File
# Begin Source File

SOURCE=.\wxTerminal.h
# End Source File
# Begin Source File

SOURCE=.\wxTurtleGraphics.h
# End Source File
# Begin Source File

SOURCE=.\xgraphics.h
# End Source File
# Begin Source File

SOURCE=.\ztcterm.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

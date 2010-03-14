# Microsoft Developer Studio Project File - Name="gammu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=gammu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gammu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gammu.mak" CFG="gammu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gammu - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "gammu - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gammu - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "gammu - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "gammu - Win32 Release"
# Name "gammu - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\common\protocol\alcatel\alcabus.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\alcatel\alcatel.c
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\at\at.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\at\atgen.c
# End Source File
# Begin Source File

SOURCE=..\..\common\misc\cfg.c
# End Source File
# Begin Source File

SOURCE=..\..\common\misc\coding.c
# End Source File
# Begin Source File

SOURCE=..\depend\dct3.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\dct3func.c
# End Source File
# Begin Source File

SOURCE=..\depend\dct4.c
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\nokia\fbus2.c
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\nokia\fbusirda.c
# End Source File
# Begin Source File

SOURCE=..\gammu.c
# End Source File
# Begin Source File

SOURCE=.\gammu.rc
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmback.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmcal.c
# End Source File
# Begin Source File

SOURCE=..\..\common\gsmcomon.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmlogo.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmnet.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmpbk.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmring.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmsms.c
# End Source File
# Begin Source File

SOURCE=..\..\common\gsmstate.c
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmwap.c
# End Source File
# Begin Source File

SOURCE=..\..\common\device\irda\irda.c
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\nokia\mbus2.c
# End Source File
# Begin Source File

SOURCE=..\..\common\misc\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\n6110.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct4\n6510.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\n7110.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\n9210.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\nauto.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\nfunc.c
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\pfunc.c
# End Source File
# Begin Source File

SOURCE=..\smsd\s_files.c
# End Source File
# Begin Source File

SOURCE=..\smsd\smsdcore.c
# End Source File
# Begin Source File

SOURCE=..\sniff.c
# End Source File
# Begin Source File

SOURCE=..\..\common\device\bluetoth\unixblue.c
# End Source File
# Begin Source File

SOURCE=..\..\common\device\serial\win32.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\common\protocol\alcatel\alcabus.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\alcatel\alcatel.h
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\at\at.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\at\atgen.h
# End Source File
# Begin Source File

SOURCE=..\..\common\misc\cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\common\misc\coding.h
# End Source File
# Begin Source File

SOURCE=..\depend\dct3.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\dct3comm.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\dct3func.h
# End Source File
# Begin Source File

SOURCE=..\depend\dct4.h
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\nokia\fbus2.h
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\nokia\fbusirda.h
# End Source File
# Begin Source File

SOURCE=..\gammu.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmback.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmcal.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsmcomon.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmlogo.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmnet.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmpbk.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmprof.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmring.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmsms.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsmstate.h
# End Source File
# Begin Source File

SOURCE=..\..\common\service\gsmwap.h
# End Source File
# Begin Source File

SOURCE=..\..\common\device\irda\irda.h
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\nokia\mbus2.h
# End Source File
# Begin Source File

SOURCE=..\..\common\misc\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\n6110.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct4\n6510.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\n7110.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\dct3\n9210.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\ncommon.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\nokia\nfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\common\phone\pfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\common\protocol\protocol.h
# End Source File
# Begin Source File

SOURCE=..\smsd\s_files.h
# End Source File
# Begin Source File

SOURCE=..\smsd\smsdcore.h
# End Source File
# Begin Source File

SOURCE=..\sniff.h
# End Source File
# Begin Source File

SOURCE=..\..\common\device\bluetoth\unixblue.h
# End Source File
# Begin Source File

SOURCE=..\..\common\device\irda\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\common\device\serial\win32.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

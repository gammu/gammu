# Microsoft Developer Studio Project File - Name="GnokiiAPI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GnokiiAPI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GnokiiAPI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GnokiiAPI.mak" CFG="GnokiiAPI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GnokiiAPI - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GnokiiAPI - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GnokiiAPI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "USE_NLS" /D "SECURITY" /D "HAVE_TIMEOPS" /D "WIN32" /D "VC6" /D "__svr4__" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GNOKIIAPI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "USE_NLS" /D "SECURITY" /D "HAVE_TIMEOPS" /D "WIN32" /D "VC6" /D "__svr4__" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GNOKIIAPI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG"
# ADD RSC /l 0x404 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "GnokiiAPI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "USE_NLS" /D "SECURITY" /D "HAVE_TIMEOPS" /D "WIN32" /D "VC6" /D "__svr4__" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GNOKIIAPI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "USE_NLS" /D "SECURITY" /D "HAVE_TIMEOPS" /D "WIN32" /D "VC6" /D "__svr4__" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GNOKIIAPI_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG"
# ADD RSC /l 0x404 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GnokiiAPI - Win32 Release"
# Name "GnokiiAPI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\at.c"
# End Source File
# Begin Source File

SOURCE=".\cfgreader.c"
# End Source File
# Begin Source File

SOURCE=".\device.c"
# End Source File
# Begin Source File

SOURCE=".\fbus.c"
# End Source File
# Begin Source File

SOURCE=".\fbus3110.c"
# End Source File
# Begin Source File

SOURCE=".\fbusirda.c"
# End Source File
# Begin Source File

SOURCE=".\getopt.c"
# End Source File
# Begin Source File

SOURCE=".\gnokiiapi.c"
# End Source File
# Begin Source File

SOURCE=".\gnokiiapi.def"
# End Source File
# Begin Source File

SOURCE=".\gsm-api.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-bitmaps.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-calendar.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-coding.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-datetime.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-filetypes.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-networks.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-phonebook.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-ringtones.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-sms.c"
# End Source File
# Begin Source File

SOURCE=".\gsm-wap.c"
# End Source File
# Begin Source File

SOURCE=".\mbus.c"
# End Source File
# Begin Source File

SOURCE=".\midifile.c"
# End Source File
# Begin Source File

SOURCE=".\misc.c"
# End Source File
# Begin Source File

SOURCE=".\n3110.c"
# End Source File
# Begin Source File

SOURCE=".\n6110.c"
# End Source File
# Begin Source File

SOURCE=".\n7110.c"
# End Source File
# Begin Source File

SOURCE=".\newat.c"
# End Source File
# Begin Source File

SOURCE=".\rlp-common.c"
# End Source File
# Begin Source File

SOURCE=".\rlp-crc24.c"
# End Source File
# Begin Source File

SOURCE=".\sniff.c"
# End Source File
# Begin Source File

SOURCE=".\winserial.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\at.h"
# End Source File
# Begin Source File

SOURCE=.\cfgreader.h
# End Source File
# Begin Source File

SOURCE=.\fbusirda.h
# End Source File
# Begin Source File

SOURCE=.\getopt.h
# End Source File
# Begin Source File

SOURCE=.\gnokiiapi.h
# End Source File
# Begin Source File

SOURCE=".\gsm-api.h"
# End Source File
# Begin Source File

SOURCE=".\gsm-coding.h"
# End Source File
# Begin Source File

SOURCE=".\gsm-common.h"
# End Source File
# Begin Source File

SOURCE=".\gsm-datetime.h"
# End Source File
# Begin Source File

SOURCE=".\gsm-filetypes.h"
# End Source File
# Begin Source File

SOURCE=".\gsm-networks.h"
# End Source File
# Begin Source File

SOURCE=".\gsm-wap.h"
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=".\win32\misc_win32.h"
# End Source File
# Begin Source File

SOURCE=".\n3110.h"
# End Source File
# Begin Source File

SOURCE=".\n6110.h"
# End Source File
# Begin Source File

SOURCE=".\newat.h"
# End Source File
# Begin Source File

SOURCE=".\rlp-common.h"
# End Source File
# Begin Source File

SOURCE=".\rlp-crc24.h"
# End Source File
# Begin Source File

SOURCE=.\winserial.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Script1.rc
# End Source File
# End Group
# End Target
# End Project

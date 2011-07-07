# Microsoft Developer Studio Project File - Name="MTK" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MTK - Win32 Debug_Arima
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MTK.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MTK.mak" CFG="MTK - Win32 Debug_Arima"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MTK - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MTK - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MTK - Win32 Debug_PG1900" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MTK - Win32 Release_PG1900" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MTK - Win32 Debug_Arima" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MTK - Win32 Release_Arima" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Mobile/CAMagic Mobile 3.0/Phone/MTK", UWYCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MTK - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Release"
# PROP Intermediate_Dir "..\temp\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\Proj_inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\exe\release\MTK.dll" /libpath:"..\lib\Release" /libpath:"..\anwlib\lib\release"

!ELSEIF  "$(CFG)" == "MTK - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\debug"
# PROP Intermediate_Dir "..\temp\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\Proj_inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"..\exe\debug\MTK.dll" /pdbtype:sept /libpath:"..\lib\debug" /libpath:"..\anwlib\lib\debug"

!ELSEIF  "$(CFG)" == "MTK - Win32 Debug_PG1900"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MTK___Win32_Debug_PG1900"
# PROP BASE Intermediate_Dir "MTK___Win32_Debug_PG1900"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\debug"
# PROP Intermediate_Dir "..\..\temp\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\Proj_inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\Proj_inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_PG1900" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\debug\MTK.dll" /pdbtype:sept /libpath:"..\..\lib\debug"
# ADD LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\debug\MTK.dll" /pdbtype:sept /libpath:"..\..\lib\debug"

!ELSEIF  "$(CFG)" == "MTK - Win32 Release_PG1900"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MTK___Win32_Release_PG1900"
# PROP BASE Intermediate_Dir "MTK___Win32_Release_PG1900"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\Release"
# PROP Intermediate_Dir "..\..\temp\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\Proj_inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Proj_inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_PG1900" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\Release\MTK.dll" /libpath:"..\..\lib\Release"
# ADD LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\Release\MTK.dll" /libpath:"..\..\lib\Release"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "MTK - Win32 Debug_Arima"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MTK___Win32_Debug_Arima"
# PROP BASE Intermediate_Dir "MTK___Win32_Debug_Arima"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\debug"
# PROP Intermediate_Dir "..\..\temp\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\Proj_inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\Proj_inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ARIMA" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\debug\MTK.dll" /pdbtype:sept /libpath:"..\..\lib\debug"
# ADD LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"D:\A&W Project\Mingfa-Arima\code\Arima Driver\CAMagic Mobile\MTK.dll" /pdbtype:sept /libpath:"..\..\lib\debug"

!ELSEIF  "$(CFG)" == "MTK - Win32 Release_Arima"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MTK___Win32_Release_Arima"
# PROP BASE Intermediate_Dir "MTK___Win32_Release_Arima"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\Release"
# PROP Intermediate_Dir "..\..\temp\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\Proj_inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Proj_inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ARIMA" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\Release\MTK.dll" /libpath:"..\..\lib\Release"
# ADD LINK32 ObexGen.lib ATgen.lib common.lib shfolder.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\Release\MTK.dll" /libpath:"..\..\lib\Release"

!ENDIF 

# Begin Target

# Name "MTK - Win32 Release"
# Name "MTK - Win32 Debug"
# Name "MTK - Win32 Debug_PG1900"
# Name "MTK - Win32 Release_PG1900"
# Name "MTK - Win32 Debug_Arima"
# Name "MTK - Win32 Release_Arima"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MTK.cpp
# End Source File
# Begin Source File

SOURCE=.\MTK.def
# End Source File
# Begin Source File

SOURCE=.\MTK.rc
# End Source File
# Begin Source File

SOURCE=.\MTKCalendar.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKFile.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKInit.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKMMS.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKObex.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKPhonebook.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKSMS.cpp
# End Source File
# Begin Source File

SOURCE=.\MTKSync.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MTK.h
# End Source File
# Begin Source File

SOURCE=.\MTKfundef.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\MTK.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

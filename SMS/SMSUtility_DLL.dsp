# Microsoft Developer Studio Project File - Name="SMSUtility_DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SMSUtility_DLL - Win32 Debug_Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SMSUtility_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SMSUtility_DLL.mak" CFG="SMSUtility_DLL - Win32 Debug_Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SMSUtility_DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SMSUtility_DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SMSUtility_DLL - Win32 Debug_Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SMSUtility_DLL - Win32 Release_Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Mobile/CAMagic Mobile 3.0/Application/SMSUtility", ORZCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SMSUtility_DLL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../temp/Release"
# PROP Intermediate_Dir "../../temp/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\..\Proj_Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release_dll/SMSUtility.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 CommUI.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\release\SMSUtility.dll" /libpath:"..\anwlib\lib\Release" /libpath:"..\..\Proc_Lib" /libpath:"..\..\lib\Release"

!ELSEIF  "$(CFG)" == "SMSUtility_DLL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../temp/Debug"
# PROP Intermediate_Dir "../../temp/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\..\Proj_Inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug_dll/SMSUtility.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 CommUI.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\Debug\SMSUtility.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\..\Proc_Lib" /libpath:"..\..\lib\Debug"

!ELSEIF  "$(CFG)" == "SMSUtility_DLL - Win32 Debug_Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SMSUtility_DLL___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "SMSUtility_DLL___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../temp/Debug"
# PROP Intermediate_Dir "../temp/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\..\Proj_Inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\Proj_Inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_UNICODE" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug_dll/SMSUtility.bsc"
# ADD BSC32 /nologo /o"Debug_dll/SMSUtility.bsc"
LINK32=link.exe
# ADD BASE LINK32 CommUI.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\Debug\SMSUtility.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\..\Proc_Lib" /libpath:"..\..\lib\Debug"
# ADD LINK32 CommUIU.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\exe\Debug\SMSUtilityU.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\Proc_Lib" /libpath:"..\lib\Debug"

!ELSEIF  "$(CFG)" == "SMSUtility_DLL - Win32 Release_Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SMSUtility_DLL___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "SMSUtility_DLL___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../temp/Release"
# PROP Intermediate_Dir "../temp/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\..\Proj_Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\Proj_Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_UNICODE" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release_dll/SMSUtility.bsc"
# ADD BSC32 /nologo /o"Release_dll/SMSUtility.bsc"
LINK32=link.exe
# ADD BASE LINK32 CommUI.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\release\SMSUtility.dll" /libpath:"..\anwlib\lib\Release" /libpath:"..\..\Proc_Lib" /libpath:"..\..\lib\Release"
# ADD LINK32 CommUIU.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\exe\release\SMSUtilityU.dll" /libpath:"..\anwlib\lib\Release" /libpath:"..\Proc_Lib" /libpath:"..\lib\Release"

!ENDIF 

# Begin Target

# Name "SMSUtility_DLL - Win32 Release"
# Name "SMSUtility_DLL - Win32 Debug"
# Name "SMSUtility_DLL - Win32 Debug_Unicode"
# Name "SMSUtility_DLL - Win32 Release_Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ContactsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ContentEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\LeftView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\NewSMS.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenSMS.cpp
# End Source File
# Begin Source File

SOURCE=.\PhoneNumberEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\PrevView.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReportCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SkinHeaderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SMSUtility.def
# End Source File
# Begin Source File

SOURCE=.\SMSUtility.rc
# End Source File
# Begin Source File

SOURCE=.\SMSUtility_DLL.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\worker.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ContactsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ContentEdit.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\import_lib.h
# End Source File
# Begin Source File

SOURCE=.\LeftView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\NewSMS.h
# End Source File
# Begin Source File

SOURCE=.\OpenSMS.h
# End Source File
# Begin Source File

SOURCE=.\PhoneNumberEdit.h
# End Source File
# Begin Source File

SOURCE=.\PrevView.h
# End Source File
# Begin Source File

SOURCE=.\ProcessDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\ReportCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SkinHeaderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\SMSUtility.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\UIMessage.h
# End Source File
# Begin Source File

SOURCE=.\worker.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\res\camagic mobile icon.ico"
# End Source File
# Begin Source File

SOURCE=.\res\phone.ico
# End Source File
# Begin Source File

SOURCE=.\res\sim.ico
# End Source File
# Begin Source File

SOURCE=.\res\SMSUtility.rc2
# End Source File
# Begin Source File

SOURCE=.\res\tb_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tb_toolb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Group "Utility Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TabSplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\TabSplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\Toolbar\uifixtb.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbar\uifixtb.h
# End Source File
# Begin Source File

SOURCE=.\Toolbar\uiflatbar.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbar\uiflatbar.h
# End Source File
# Begin Source File

SOURCE=.\Toolbar\uimodulver.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbar\uimodulver.h
# End Source File
# End Group
# Begin Group "Print Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CPage.cpp
# End Source File
# Begin Source File

SOURCE=.\CPage.h
# End Source File
# Begin Source File

SOURCE=.\CPrinter.cpp
# End Source File
# Begin Source File

SOURCE=.\CPrinter.h
# End Source File
# Begin Source File

SOURCE=.\Dib.cpp
# End Source File
# Begin Source File

SOURCE=.\Dib.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

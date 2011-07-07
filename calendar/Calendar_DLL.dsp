# Microsoft Developer Studio Project File - Name="Calendar_DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Calendar_DLL - Win32 Debug_Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Calendar_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Calendar_DLL.mak" CFG="Calendar_DLL - Win32 Debug_Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Calendar_DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Calendar_DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Calendar_DLL - Win32 Debug_Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Calendar_DLL - Win32 Release_Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Mobile/CAMagic Mobile 3.0/Application/Calendar", ULZCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Calendar_DLL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\temp\Release\Calendar"
# PROP Intermediate_Dir "..\..\temp\Release\Calendar"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\..\Proj_Inc" /I "..\Proj_Inc\SYNC" /I "..\..\Proj_Inc\MSOT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 CommUI.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\release\Calendar.dll" /libpath:"..\anwlib\lib\release" /libpath:"..\..\lib\release"

!ELSEIF  "$(CFG)" == "Calendar_DLL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\temp\debug\Calendar"
# PROP Intermediate_Dir "..\..\temp\debug\Calendar"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\..\Proj_Inc" /I "..\Proj_Inc\SYNC" /I "..\..\Proj_Inc\MSOT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 CommUI.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\debug\Calendar.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\..\lib\debug"

!ELSEIF  "$(CFG)" == "Calendar_DLL - Win32 Debug_Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Calendar_DLL___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "Calendar_DLL___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\temp\debug\Calendar"
# PROP Intermediate_Dir "..\temp\debug\Calendar"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\..\Proj_Inc" /I "..\Proj_Inc\SYNC" /I "..\..\Proj_Inc\MSOT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\Proj_Inc" /I "..\Proj_Inc\SYNC" /I "..\Proj_Inc\MSOT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "_USRDLL" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 CommUI.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\exe\debug\Calendar.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\..\lib\debug"
# ADD LINK32 CommUIU.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\exe\debug\CalendarU.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\lib\debug"

!ELSEIF  "$(CFG)" == "Calendar_DLL - Win32 Release_Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Calendar_DLL___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "Calendar_DLL___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\temp\Release\Calendar"
# PROP Intermediate_Dir "..\temp\Release\Calendar"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\..\Proj_Inc" /I "..\Proj_Inc\SYNC" /I "..\..\Proj_Inc\MSOT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\Proj_Inc" /I "..\Proj_Inc\SYNC" /I "..\Proj_Inc\MSOT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_UNICODE" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 CommUI.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\release\Calendar.dll" /libpath:"..\anwlib\lib\release" /libpath:"..\..\lib\release"
# ADD LINK32 CommUIU.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\exe\release\CalendarU.dll" /libpath:"..\anwlib\lib\release" /libpath:"..\lib\release"

!ENDIF 

# Begin Target

# Name "Calendar_DLL - Win32 Release"
# Name "Calendar_DLL - Win32 Debug"
# Name "Calendar_DLL - Win32 Debug_Unicode"
# Name "Calendar_DLL - Win32 Release_Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Tool bar"

# PROP Default_Filter ""
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
# Begin Group "Schedule"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Schedule\cammschedulecalendar.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\cammschedulecalendar.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\CAMMScheduleDailyTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\CAMMScheduleDailyTable.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\CAMMScheduleDayView.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\CAMMScheduleDayView.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\CAMMScheduleEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\CAMMScheduleEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\cammschedulelist.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\cammschedulelist.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\ListPrint.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\ListPrint.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\PrintSchedule.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\PrintSchedule.h
# End Source File
# Begin Source File

SOURCE=.\Schedule\XCell.cpp
# End Source File
# Begin Source File

SOURCE=.\Schedule\XCell.h
# End Source File
# End Group
# Begin Group "Organizer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Organizer\OleDateTimeEx.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\OleDateTimeEx.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\OrganizeDeclaration.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\Schedule3MonthInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\Schedule3MonthInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleAllInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleAllInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleAllRepeatInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleAllRepeatInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleDayInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleDayInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleMonthInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleMonthInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleMultiInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleMultiInfo.h
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleTimeInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Organizer\ScheduleTimeInfo.h
# End Source File
# End Group
# Begin Group "MobileDevice"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MobileDevice\CAMMSync.cpp
# End Source File
# Begin Source File

SOURCE=.\MobileDevice\CAMMSync.h
# End Source File
# Begin Source File

SOURCE=.\MobileDevice\CAMMSyncInclude.cpp
# End Source File
# Begin Source File

SOURCE=.\MobileDevice\CAMMSyncInclude.h
# End Source File
# End Group
# Begin Group "UnicodeString"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UnicodeString\UnicodeString.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeString\UnicodeString.h
# End Source File
# End Group
# Begin Group "Logger"

# PROP Default_Filter ""
# End Group
# Begin Group "CIni_Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CIni_Src\Ini.cpp
# End Source File
# Begin Source File

SOURCE=.\CIni_Src\Ini.h
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\SkinHeaderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SkinHeaderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\include\SortClass.cpp
# End Source File
# Begin Source File

SOURCE=.\include\SortClass.h
# End Source File
# Begin Source File

SOURCE=.\include\TabButtonEx2.cpp
# End Source File
# Begin Source File

SOURCE=.\include\TabButtonEx2.h
# End Source File
# Begin Source File

SOURCE=.\include\TabSplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\include\TabSplitterWnd.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Calendar.def
# End Source File
# Begin Source File

SOURCE=.\Calendar.rc
# End Source File
# Begin Source File

SOURCE=.\Calendar_DLL.cpp
# End Source File
# Begin Source File

SOURCE=.\DialogBarEx.cpp
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\LeftView.cpp
# End Source File
# Begin Source File

SOURCE=.\loadString.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\Proj_inc\MSOT\MSOTDLL.cpp
# End Source File
# Begin Source File

SOURCE=.\myedit.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenrepeatQuestDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PrevView.cpp
# End Source File
# Begin Source File

SOURCE=.\processdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TopBarDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TopView.cpp
# End Source File
# Begin Source File

SOURCE=.\UserExitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewModePanel.cpp
# End Source File
# Begin Source File

SOURCE=.\worker.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Calendar.h
# End Source File
# Begin Source File

SOURCE=.\CommonDeclaration.h
# End Source File
# Begin Source File

SOURCE=.\DialogBarEx.h
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

SOURCE=.\loadString.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=..\..\Proj_inc\MSOT\MSOTDLL.h
# End Source File
# Begin Source File

SOURCE=.\myedit.h
# End Source File
# Begin Source File

SOURCE=.\OpenrepeatQuestDlg.h
# End Source File
# Begin Source File

SOURCE=.\PrevView.h
# End Source File
# Begin Source File

SOURCE=.\processdlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TopBarDlg.h
# End Source File
# Begin Source File

SOURCE=.\TopView.h
# End Source File
# Begin Source File

SOURCE=.\UserExitDlg.h
# End Source File
# Begin Source File

SOURCE=.\ViewModePanel.h
# End Source File
# Begin Source File

SOURCE=.\worker.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\birthday.ico
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Calendar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Calendar.ico
# End Source File
# Begin Source File

SOURCE=.\res\Calendar.rc2
# End Source File
# Begin Source File

SOURCE=.\res\clock.bmp
# End Source File
# Begin Source File

SOURCE=.\res\clock_gray.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ColumnHeaderEnd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ColumnHeaderSpan.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ColumnHeaderStart.bmp
# End Source File
# Begin Source File

SOURCE=.\res\gototoday_up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\memorandum.ico
# End Source File
# Begin Source File

SOURCE=.\res\memorandum_1.ico
# End Source File
# Begin Source File

SOURCE=.\res\metting.ico
# End Source File
# Begin Source File

SOURCE=.\res\phone.ico
# End Source File
# Begin Source File

SOURCE=.\res\PhoneBook.ico
# End Source File
# Begin Source File

SOURCE=.\res\repeat.bmp
# End Source File
# Begin Source File

SOURCE=.\res\repeat_gray.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sch_more.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sch_more_gray.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tb_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tb_toolb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar_.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

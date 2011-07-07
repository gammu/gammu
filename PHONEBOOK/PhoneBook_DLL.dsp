# Microsoft Developer Studio Project File - Name="PhoneBook_DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PhoneBook_DLL - Win32 Debug_Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PhoneBook_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PhoneBook_DLL.mak" CFG="PhoneBook_DLL - Win32 Debug_Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PhoneBook_DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PhoneBook_DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PhoneBook_DLL - Win32 Debug_Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PhoneBook_DLL - Win32 Release_Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PhoneBook_DLL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\temp\Release\Phonebook"
# PROP Intermediate_Dir "..\..\temp\Release\Phonebook"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\..\Proj_Inc" /I "..\..\Proj_Inc\MSOT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 CommUI.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\exe\Release\Phonebook.dll" /libpath:"..\anwlib\lib\Release" /libpath:"..\..\lib\Release"

!ELSEIF  "$(CFG)" == "PhoneBook_DLL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\temp\Debug\Phonebook"
# PROP Intermediate_Dir "..\..\temp\Debug\Phonebook"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\..\Proj_Inc" /I "..\..\Proj_Inc\MSOT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/PhoneBook.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 CommUI.lib /nologo /subsystem:windows /dll /pdb:"Debug/Phonebook.pdb" /debug /machine:I386 /out:"../../EXE/Debug/Phonebook.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\..\lib\debug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PhoneBook_DLL - Win32 Debug_Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PhoneBook_DLL___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "PhoneBook_DLL___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\temp\Debug\Phonebook"
# PROP Intermediate_Dir "..\temp\Debug\Phonebook"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\Proj_Inc" /I "..\Proj_Inc\sync" /I "..\Proj_Inc\MSOT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\anwlib\include" /I "..\Proj_Inc" /I "..\Proj_Inc\MSOT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_UNICODE" /D "_USRDLL" /D "DLL_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug/PhoneBook.bsc"
# ADD BSC32 /nologo /o"Debug/PhoneBook.bsc"
LINK32=link.exe
# ADD BASE LINK32 CommUI.lib /nologo /subsystem:windows /dll /pdb:"Debug/Phonebook.pdb" /debug /machine:I386 /out:"E:\Mobile Driver\exe\Debug\Phonebook.dll" /pdbtype:sept /libpath:"..\anwlib\lib" /libpath:"..\Bluetooth_Driver\Proc_Lib" /libpath:"..\lib\debug"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 CommUIU.lib shlwapi.lib /nologo /subsystem:windows /dll /pdb:"Debug/Phonebook.pdb" /debug /machine:I386 /out:"..\exe\debug\PhonebookU.dll" /pdbtype:sept /libpath:"..\anwlib\lib\Debug" /libpath:"..\lib\debug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PhoneBook_DLL - Win32 Release_Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PhoneBook_DLL___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "PhoneBook_DLL___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\temp\Release\Phonebook"
# PROP Intermediate_Dir "..\temp\Release\Phonebook"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\Proj_Inc" /I "..\Proj_Inc\sync" /I "..\Proj_Inc\MSOT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\anwlib\include" /I "..\Proj_Inc" /I "..\Proj_Inc\MSOT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_UNICODE" /D "_USRDLL" /D "DLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x404 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 CommUI.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\exe\Release\Phonebook.dll" /libpath:"..\anwlib\lib" /libpath:"..\Bluetooth_Driver\Proc_Lib" /libpath:"..\lib\Release"
# ADD LINK32 CommUIU.lib shlwapi.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\exe\release\PhonebookU.dll" /libpath:"..\anwlib\lib\Release" /libpath:"..\lib\Release"

!ENDIF 

# Begin Target

# Name "PhoneBook_DLL - Win32 Release"
# Name "PhoneBook_DLL - Win32 Debug"
# Name "PhoneBook_DLL - Win32 Debug_Unicode"
# Name "PhoneBook_DLL - Win32 Release_Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dlg\AddressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\contentedit.cpp
# End Source File
# Begin Source File

SOURCE=.\DriverWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\motoE2dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\motoEdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\Proj_inc\MSOT\MSOTDLL.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\newsms.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\NS4AddressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\OtherDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PhoneBook.def
# End Source File
# Begin Source File

SOURCE=.\PhoneBook_DLL.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\phonenumberedit.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\S4datadycdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\S4datadycdlgEx.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\Sagemdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\SamsungDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\SamsungZdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\Sharp90Xdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dlg\SharpT300Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dlg\AddressDlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\contentedit.h
# End Source File
# Begin Source File

SOURCE=.\DriverWrapper.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\import_lib.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\dlg\motoE2dlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\motoEdlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Proj_inc\MSOT\MSOTDLL.h
# End Source File
# Begin Source File

SOURCE=.\dlg\newsms.h
# End Source File
# Begin Source File

SOURCE=.\dlg\NS4AddressDlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\NS6AddressDlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\OtherDlg.h
# End Source File
# Begin Source File

SOURCE=.\PhoneBook.h
# End Source File
# Begin Source File

SOURCE=.\PhoneBook.rc
# End Source File
# Begin Source File

SOURCE=.\PhoneBookEntry.h
# End Source File
# Begin Source File

SOURCE=.\dlg\phonenumberedit.h
# End Source File
# Begin Source File

SOURCE=.\dlg\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\dlg\S4datadycdlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\S4datadycdlgEx.h
# End Source File
# Begin Source File

SOURCE=.\dlg\S6datadycdlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\Sagemdlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\SamsungDlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\SamsungZdlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\Sharp90Xdlg.h
# End Source File
# Begin Source File

SOURCE=.\dlg\SharpT300Dlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp1.bmp
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

SOURCE=.\res\idr_main.ico
# End Source File
# Begin Source File

SOURCE=.\res\phone.bmp
# End Source File
# Begin Source File

SOURCE=.\res\phone.ico
# End Source File
# Begin Source File

SOURCE=.\res\PhoneBook.ico
# End Source File
# Begin Source File

SOURCE=.\res\PhoneBook.rc2
# End Source File
# Begin Source File

SOURCE=.\res\PhoneBook_DLL.rc2
# End Source File
# Begin Source File

SOURCE=.\res\sim.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sim.ico
# End Source File
# Begin Source File

SOURCE=.\res\tb_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tb_toolb.bmp
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\CheckThumbListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CheckThumbListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\include\CSVHandle.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CSVHandle.h
# End Source File
# Begin Source File

SOURCE=.\include\myedit.cpp
# End Source File
# Begin Source File

SOURCE=.\include\myedit.h
# End Source File
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
# Begin Group "Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dlg\asusdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\asusdlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\basedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\basedlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\DataDycDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\DataDycDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\motodlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\motodlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\NS6AddressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\ProgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\ProgDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\S6datadycdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\SearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\SearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\siemenssdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\siemenssdlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\sonyericcsonskdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\sonyericcsonskdlg.h
# End Source File
# Begin Source File

SOURCE=.\Dlg\sonyericssonatdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\sonyerisontdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg\sonyerisontdlg.h
# End Source File
# End Group
# Begin Group "Data"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Data Structure\CardPanel.cpp"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\CardPanel.h"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\ListData.cpp"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\ListData.h"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\PhBoundary.cpp"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\PhBoundary.h"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\PhoneData.cpp"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\PhoneData.h"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\PhoneNumber.cpp"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\PhoneNumber.h"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\SupportEntry.cpp"
# End Source File
# Begin Source File

SOURCE=".\Data Structure\SupportEntry.h"
# End Source File
# End Group
# Begin Group "Panel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Panel\LeftView.cpp
# End Source File
# Begin Source File

SOURCE=.\Panel\LeftView.h
# End Source File
# Begin Source File

SOURCE=.\Panel\PrevView.cpp
# End Source File
# Begin Source File

SOURCE=.\Panel\PrevView.h
# End Source File
# Begin Source File

SOURCE=.\Panel\ViewModePanel.cpp
# End Source File
# Begin Source File

SOURCE=.\Panel\ViewModePanel.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

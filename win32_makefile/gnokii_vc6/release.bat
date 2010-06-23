@echo off
call ..\fcopy.bat make
echo Making
NMAKE /f "gnokii.mak" CFG="gnokii - Win32 Release" > log
echo Gnokii.exe is in the RELEASE directory
call ..\clean.bat make

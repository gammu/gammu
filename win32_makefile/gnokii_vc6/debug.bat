@echo off
call ..\fcopy.bat make
echo Making
NMAKE /f "gnokii.mak" CFG="gnokii - Win32 Debug" > log
echo Gnokii.exe is in the DEBUG directory
call ..\clean.bat make

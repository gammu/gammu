@echo off
if not "%1"=="make" goto info
echo Preparing files
start /wait ..\mversion.js make
copy ..\..\common\*.c *.c > nul
copy ..\..\common\protocol\*.c *.c > nul
copy ..\..\common\devices\*.c *.c > nul
copy ..\..\getopt\win32\*.c *.c > nul
copy ..\..\getopt\win32\*.h *.h > nul
copy ..\..\common\data\*.c *.c > nul
copy ..\..\common\files\*.c *.c > nul
copy ..\..\common\newmodules\*.c *.c > nul
copy ..\..\common\newmodules\sniff\*.c *.c > nul
copy ..\..\gnokii\gnokii.c gnokii.c > nul
copy ..\..\gnokii\gnokii.h gnokii.h > nul
copy ..\..\include\*.h *.h > nul
mkdir files > nul
copy ..\..\include\files\*.h files\*.h >nul
mkdir data > nul
copy ..\..\include\data\*.h data\*.h >nul
mkdir newmodules > nul
copy ..\..\include\newmodules\*.h newmodules\*.h >nul
cd newmodules > nul
mkdir sniff > nul
cd .. > nul
copy ..\..\include\newmodules\sniff\*.h newmodules\sniff\*.h >nul
mkdir protocol > nul
copy ..\..\include\protocol\*.h protocol\*.h >nul
mkdir devices > nul
copy ..\..\include\devices\*.h devices\*.h >nul
del unixserial.* > nul
goto end
:info
echo Used by bat in subdirs for copying source files
:end
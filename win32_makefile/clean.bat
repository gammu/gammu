@echo off
if not "%1"=="make" goto info
del *.c > nul
del *.h > nul
del protocol\*.h > nul
rmdir protocol > nul
del newmodules\sniff\*.h > nul
rmdir newmodules\sniff > nul
del newmodules\*.h > nul
rmdir newmodules > nul
del files\*.h > nul
rmdir files > nul
del data\*.h > nul
rmdir data > nul
del devices\*.h > nul
rmdir devices > nul
goto end
:info
echo Used by bat in subdirs for cleaning source files
:end
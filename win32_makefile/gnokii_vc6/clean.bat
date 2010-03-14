@echo off
call ..\clean.bat make
del release\*.pch > nul
rem del release\*.exe > nul
del release\*.idb > nul
del release\*.obj > nul
del release\*.pdb > nul
del release\*.ilk > nul
rmdir release > nul
del debug\*.pch > nul
rem del debug\*.exe > nul
del debug\*.idb > nul
del debug\*.obj > nul
del debug\*.pdb > nul
del debug\*.ilk > nul
rmdir debug > nul
del log > nul

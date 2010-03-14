@echo off
ren gnokiiapi.c gnokiiapi.1 > nul
ren gnokiiapi.h gnokiiapi.2 > nul
ren resource.h resource.1 > nul
call ..\clean.bat make
del release\*.pch > nul
del release\*.exe > nul
del release\*.idb > nul
del release\*.obj > nul
del release\*.pdb > nul
del release\*.ilk > nul
rmdir release > nul
del debug\*.pch > nul
del debug\*.exe > nul
del debug\*.idb > nul
del debug\*.obj > nul
del debug\*.pdb > nul
del debug\*.ilk > nul
rmdir debug > nul
ren gnokiiapi.1 gnokiiapi.c > nul
ren gnokiiapi.2 gnokiiapi.h > nul
ren resource.1 resource.h > nul
del log > nul
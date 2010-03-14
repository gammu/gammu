# Microsoft Developer Studio Generated NMAKE File, Based on gnokii.dsp
!IF "$(CFG)" == ""
CFG=gnokii - Win32 Debug
!MESSAGE No configuration specified. Defaulting to gnokii - Win32 Debug.
!ENDIF 

# here are some configuration
MODEL=\"5110\" 
PORT=\"COM2\"

!IF "$(CFG)" != "gnokii - Win32 Release" && "$(CFG)" != "gnokii - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gnokii.mak" CFG="gnokii - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gnokii - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "gnokii - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "gnokii - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\gnokii.exe"


CLEAN :
	-@erase "$(INTDIR)\cfgreader.obj"
	-@erase "$(INTDIR)\device.obj"
	-@erase "$(INTDIR)\at.obj"
	-@erase "$(INTDIR)\fbus.obj"
	-@erase "$(INTDIR)\fbus3110.obj"
	-@erase "$(INTDIR)\fbusirda.obj"
	-@erase "$(INTDIR)\mbus.obj"
	-@erase "$(INTDIR)\n7110.obj"
	-@erase "$(INTDIR)\newat.obj"
	-@erase "$(INTDIR)\n6110.obj"
	-@erase "$(INTDIR)\n3110.obj"
	-@erase "$(INTDIR)\sniff.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\gnokii.obj"
	-@erase "$(INTDIR)\gsm-api.obj"
	-@erase "$(INTDIR)\gsm-calendar.obj"
	-@erase "$(INTDIR)\gsm-coding.obj"
	-@erase "$(INTDIR)\gsm-datetime.obj"
	-@erase "$(INTDIR)\gsm-phonebook.obj"
	-@erase "$(INTDIR)\gsm-sms.obj"
	-@erase "$(INTDIR)\gsm-wap.obj"
	-@erase "$(INTDIR)\gsm-filetypes.obj"
	-@erase "$(INTDIR)\gsm-networks.obj"
	-@erase "$(INTDIR)\gsm-bitmaps.obj"
	-@erase "$(INTDIR)\gsm-ringtones.obj"
	-@erase "$(INTDIR)\rlp-common.obj"
	-@erase "$(INTDIR)\rlp-crc24.obj"
	-@erase "$(INTDIR)\midifile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\winserial.obj"
	-@erase "$(OUTDIR)\gnokii.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "USE_NLS" /D "SECURITY" /D "HAVE_TIMEOPS" /D "WIN32" /D "VC6" /D "__svr4__" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D MODEL=$(MODEL) /D PORT=$(PORT) /D VERSION=$(VERSION) $(DEBUG) /Fp"$(INTDIR)\gnokii.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gnokii.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\gnokii.pdb" /machine:I386 /out:"$(OUTDIR)\gnokii.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cfgreader.obj" \
	"$(INTDIR)\n6110.obj" \
	"$(INTDIR)\n3110.obj" \
	"$(INTDIR)\sniff.obj" \
	"$(INTDIR)\fbusirda.obj" \
	"$(INTDIR)\n7110.obj" \
	"$(INTDIR)\newat.obj" \
	"$(INTDIR)\mbus.obj" \
	"$(INTDIR)\fbus.obj" \
	"$(INTDIR)\fbus3110.obj" \
	"$(INTDIR)\at.obj" \
	"$(INTDIR)\device.obj" \
	"$(INTDIR)\gsm-sms.obj" \
	"$(INTDIR)\gsm-wap.obj" \
	"$(INTDIR)\gsm-calendar.obj" \
	"$(INTDIR)\gsm-phonebook.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\gnokii.obj" \
	"$(INTDIR)\gsm-api.obj" \
	"$(INTDIR)\gsm-coding.obj" \
	"$(INTDIR)\gsm-datetime.obj" \
	"$(INTDIR)\gsm-ringtones.obj" \
	"$(INTDIR)\gsm-bitmaps.obj" \
	"$(INTDIR)\gsm-networks.obj" \
	"$(INTDIR)\rlp-common.obj" \
	"$(INTDIR)\rlp-crc24.obj" \
	"$(INTDIR)\midifile.obj" \
	"$(INTDIR)\winserial.obj" \
	"$(INTDIR)\gsm-filetypes.obj" \

"$(OUTDIR)\gnokii.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gnokii - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\gnokii.exe"


CLEAN :
	-@erase "$(INTDIR)\cfgreader.obj"
	-@erase "$(INTDIR)\fbus.obj"
	-@erase "$(INTDIR)\fbus3110.obj"
	-@erase "$(INTDIR)\mbus.obj"
	-@erase "$(INTDIR)\at.obj"
	-@erase "$(INTDIR)\n7110.obj"
	-@erase "$(INTDIR)\n6110.obj"
	-@erase "$(INTDIR)\n3110.obj"
	-@erase "$(INTDIR)\sniff.obj"
	-@erase "$(INTDIR)\fbusirda.obj"
	-@erase "$(INTDIR)\newat.obj"
	-@erase "$(INTDIR)\device.obj"
	-@erase "$(INTDIR)\gsm-sms.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\gnokii.obj"
	-@erase "$(INTDIR)\gsm-api.obj"
	-@erase "$(INTDIR)\gsm-coding.obj"
	-@erase "$(INTDIR)\gsm-datetime.obj"
	-@erase "$(INTDIR)\gsm-wap.obj"
	-@erase "$(INTDIR)\gsm-calendar.obj"
	-@erase "$(INTDIR)\gsm-phonebook.obj"
	-@erase "$(INTDIR)\gsm-filetypes.obj"
	-@erase "$(INTDIR)\gsm-ringtones.obj"
	-@erase "$(INTDIR)\gsm-bitmaps.obj"
	-@erase "$(INTDIR)\gsm-networks.obj"
	-@erase "$(INTDIR)\rlp-common.obj"
	-@erase "$(INTDIR)\rlp-crc24.obj"
	-@erase "$(INTDIR)\midifile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\winserial.obj"
	-@erase "$(OUTDIR)\gnokii.exe"
	-@erase "$(OUTDIR)\gnokii.ilk"
	-@erase "$(OUTDIR)\gnokii.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "USE_NLS" /D "SECURITY" /D "HAVE_TIMEOPS" /D "WIN32" /D "VC6" /D "DEBUG" /D "_DEBUG" /D "__svr4__" /D "_CONSOLE" /D "_MBCS" /D MODEL=$(MODEL) /D PORT=$(PORT) /D VERSION=$(VERSION) $(DEBUG) /Fp"$(INTDIR)\gnokii.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gnokii.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\gnokii.pdb" /debug /machine:I386 /out:"$(OUTDIR)\gnokii.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cfgreader.obj" \
	"$(INTDIR)\n6110.obj" \
	"$(INTDIR)\n3110.obj" \
	"$(INTDIR)\sniff.obj" \
	"$(INTDIR)\fbusirda.obj" \
	"$(INTDIR)\fbus3110.obj" \
	"$(INTDIR)\n7110.obj" \
	"$(INTDIR)\newat.obj" \
	"$(INTDIR)\mbus.obj" \
	"$(INTDIR)\at.obj" \
	"$(INTDIR)\fbus.obj" \
	"$(INTDIR)\device.obj" \
	"$(INTDIR)\gsm-sms.obj" \
	"$(INTDIR)\gsm-wap.obj" \
	"$(INTDIR)\gsm-calendar.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\gnokii.obj" \
	"$(INTDIR)\gsm-api.obj" \
	"$(INTDIR)\gsm-coding.obj" \
	"$(INTDIR)\gsm-datetime.obj" \
	"$(INTDIR)\gsm-ringtones.obj" \
	"$(INTDIR)\gsm-phonebook.obj" \
	"$(INTDIR)\gsm-bitmaps.obj" \
	"$(INTDIR)\gsm-networks.obj" \
	"$(INTDIR)\rlp-common.obj" \
	"$(INTDIR)\rlp-crc24.obj" \
	"$(INTDIR)\midifile.obj" \
	"$(INTDIR)\winserial.obj" \
	"$(INTDIR)\gsm-filetypes.obj"

"$(OUTDIR)\gnokii.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("gnokii.dep")
!INCLUDE "gnokii.dep"
!ELSE 
!MESSAGE Warning: cannot find "gnokii.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "gnokii - Win32 Release" || "$(CFG)" == "gnokii - Win32 Debug"
SOURCE=cfgreader.c

"$(INTDIR)\cfgreader.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-sms.c"

"$(INTDIR)\gsm-sms.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-coding.c"

"$(INTDIR)\gsm-coding.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-datetime.c"

"$(INTDIR)\gsm-datetime.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="at.c"

"$(INTDIR)\at.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-wap.c"

"$(INTDIR)\gsm-wap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-calendar.c"

"$(INTDIR)\gsm-calendar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-phonebook.c"

"$(INTDIR)\gsm-phonebook.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="device.c"

"$(INTDIR)\device.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="n7110.c"

"$(INTDIR)\n7110.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="n6110.c"

"$(INTDIR)\n6110.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="n3110.c"

"$(INTDIR)\n3110.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="sniff.c"

"$(INTDIR)\sniff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="newat.c"

"$(INTDIR)\newat.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="mbus.c"

"$(INTDIR)\mbus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="fbus.c"

"$(INTDIR)\fbus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="fbus3110.c"

"$(INTDIR)\fbus3110.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-ringtones.c"

"$(INTDIR)\gsm-ringtones.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="gsm-bitmaps.c"

"$(INTDIR)\gsm-bitmaps.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="midifile.c"

"$(INTDIR)\midifile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=getopt.c

"$(INTDIR)\getopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=gnokii.c

"$(INTDIR)\gnokii.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="gsm-api.c"

"$(INTDIR)\gsm-api.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="gsm-filetypes.c"

"$(INTDIR)\gsm-filetypes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="gsm-networks.c"

"$(INTDIR)\gsm-networks.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE="rlp-common.c"

"$(INTDIR)\rlp-common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="rlp-crc24.c"

"$(INTDIR)\rlp-crc24.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=winserial.c

"$(INTDIR)\winserial.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=misc.c

"$(INTDIR)\misc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=fbusirda.c

"$(INTDIR)\fbusirda.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

!ENDIF 


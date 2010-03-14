# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------

VERSION = BCB.06.00
# ---------------------------------------------------------------------------
PROJECT = Output\gammu.exe
OBJFILES = Output\gammu.obj Output\sniff.obj Output\dct3.obj Output\wmx.obj \
    Output\wmx-sim.obj Output\wmx-util.obj Output\wmx-list.obj Output\n0650.obj \
    Output\wmx-gsm.obj Output\dct4.obj Output\dsiemens.obj Output\chiffre.obj \
    Output\s_files.obj Output\s_mysql.obj Output\smsdcore.obj Output\cfg.obj \
    Output\coding.obj Output\md5.obj Output\misc.obj Output\at.obj \
    Output\obex.obj Output\mrouter.obj Output\fbus2.obj Output\alcabus.obj \
    Output\phonet.obj Output\mbus2.obj Output\gsmback.obj Output\backldif.obj \
    Output\backlmb.obj Output\backvcs.obj Output\backvcf.obj \
    Output\backtext.obj Output\backics.obj Output\gsmcal.obj \
    Output\gsmlogo.obj Output\gsmnet.obj Output\gsmpbk.obj Output\gsmring.obj \
    Output\gsmdata.obj Output\gsmmisc.obj Output\gsmsms.obj Output\gsmems.obj \
    Output\gsmmulti.obj Output\gsmcomon.obj Output\gsmstate.obj \
    Output\n6110.obj Output\n6510.obj Output\n3320.obj Output\n3650.obj \
    Output\dct4func.obj Output\n7110.obj Output\n9210.obj Output\dct3func.obj \
    Output\nauto.obj Output\nfunc.obj Output\nfuncold.obj Output\alcatel.obj \
    Output\obexgen.obj Output\mroutgen.obj Output\pfunc.obj Output\atgen.obj \
    Output\siemens.obj Output\samsung.obj Output\sonyeric.obj Output\irda.obj \
    Output\bluetoth.obj Output\bluez.obj Output\blue_w32.obj Output\affix.obj \
    Output\ser_w32.obj Output\devfunc.obj
MAINSOURCE = gammu.bpf
RESDEPEN = $(RESFILES)
LIBFILES = 
IDLFILES = 
IDLGENFILES = 
LIBRARIES = 
PACKAGES = rtl.bpi vcl.bpi vclx.bpi bcbsmp.bpi dbrtl.bpi vcldb.bpi adortl.bpi \
    ibsmp.bpi bdertl.bpi vcldbx.bpi qrpt.bpi teeui.bpi teedb.bpi tee.bpi \
    dss.bpi teeqr.bpi ibxpress.bpi dsnap.bpi vclie.bpi inetdb.bpi inet.bpi \
    nmfast.bpi webdsnap.bpi bcbie.bpi dclocx.bpi bcb2kaxserver.bpi
SPARELIBS = 
DEFFILE = 
OTHERFILES = 
# ---------------------------------------------------------------------------
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
USERDEFINES = WIN32;NDEBUG;_CONSOLE
SYSDEFINES = _NO_VCL;_ASSERTE;NO_STRICT
INCLUDEPATH = ..\..\common\service\sms;..\depend\nokia;..\depend\siemens;..\depend\nokia\dct3trac;..\..\common\misc\coding;..\..\common\service\backup;..\;..;..\..\common\phone\symbian;..\..\common\protocol\symbian;..\..\common\protocol\obex;..\..\common\phone\obex;..\..\common\device;..\smsd;..\depend;..\..\common\device\serial;..\..\common\protocol\alcatel;..\..\common\phone\alcatel;..\..\common\phone;..\..\common\phone\nokia;..\..\common\phone\nokia\dct4;..\..\common\phone\nokia\dct3;..\..\common\device\irda;..\..\common\device\bluetoth;..\..\common;..\..\common\service;..\..\common\protocol\nokia;..\..\common\misc;..\..\common\phone\at;..\..\common\protocol\at;$(BCB)\include;$(BCB)\include\mfc;$(BCB)\include\atl
LIBPATH = ..\..\common\service\sms;..\depend\nokia;..\depend\siemens;..\depend\nokia\dct3trac;..\..\common\misc\coding;..\..\common\service\backup;..\;..;..\..\common\phone\symbian;..\..\common\protocol\symbian;..\..\common\device;..\..\common\protocol\obex;..\..\common\phone\obex;..\smsd;..\depend;..\..\common\phone\alcatel;..\..\common\protocol\alcatel;..\..\common\device\serial;..\..\common\device\bluetoth;..\..\common\phone;..\..\common\phone\nokia;..\..\common\phone\nokia\dct4;..\..\common\phone\nokia\dct3;..\..\common\device\irda;..\..\common;..\..\common\service;..\..\common\protocol\nokia;..\..\common\misc;..\..\common\phone\at;..\..\common\protocol\at;$(BCB)\lib;$(RELEASELIBPATH);$(BCB)\lib\psdk
WARNINGS= -w-par -w-8027 -w-8026 -w-csu
PATHCPP = .;..;..;..\depend\nokia;..\depend\nokia\dct3trac;..\depend\nokia\dct3trac;..\depend\nokia\dct3trac;..\depend\nokia\dct3trac;..\depend\nokia\dct3trac;..\depend\nokia;..\depend\siemens;..\depend\siemens;..\smsd;..\smsd;..\smsd;..\..\common\misc;..\..\common\misc\coding;..\..\common\misc\coding;..\..\common\misc;..\..\common\protocol\at;..\..\common\protocol\obex;..\..\common\protocol\symbian;..\..\common\protocol\nokia;..\..\common\protocol\alcatel;..\..\common\protocol\nokia;..\..\common\protocol\nokia;..\..\common\service\backup;..\..\common\service\backup;..\..\common\service\backup;..\..\common\service\backup;..\..\common\service\backup;..\..\common\service\backup;..\..\common\service\backup;..\..\common\service;..\..\common\service;..\..\common\service;..\..\common\service;..\..\common\service;..\..\common\service;..\..\common\service;..\..\common\service\sms;..\..\common\service\sms;..\..\common\service\sms;..\..\common;..\..\common;..\..\common\phone\nokia\dct3;..\..\common\phone\nokia\dct4;..\..\common\phone\nokia\dct4;..\..\common\phone\nokia\dct4;..\..\common\phone\nokia\dct4;..\..\common\phone\nokia\dct3;..\..\common\phone\nokia\dct3;..\..\common\phone\nokia\dct3;..\..\common\phone\nokia;..\..\common\phone\nokia;..\..\common\phone\nokia;..\..\common\phone\alcatel;..\..\common\phone\obex;..\..\common\phone\symbian;..\..\common\phone;..\..\common\phone\at;..\..\common\phone\at;..\..\common\phone\at;..\..\common\phone\at;..\..\common\device\irda;..\..\common\device\bluetoth;..\..\common\device\bluetoth;..\..\common\device\bluetoth;..\..\common\device\bluetoth;..\..\common\device\serial;..\..\common\device
PATHASM = .;
PATHPAS = .;
PATHRC = .;
PATHOBJ = .;$(LIBPATH)
# ---------------------------------------------------------------------------
CFLAG1 = -O2 -w- -VF -fp -X- -a8 -5 -b -k -vi -q -c
IDLCFLAGS = -src_suffix cpp -DWIN32 -DNDEBUG -D_CONSOLE -I..\. -I.. \
    -I..\..\common\protocol\obex -I..\..\common\phone\obex \
    -I..\..\common\service\sms -I..\..\common\misc\coding \
    -I..\..\common\phone\symbian -I..\..\common\device\serial -I..\smsd \
    -I..\..\common\phone -I..\depend -I..\depend\siemens -I..\depend\nokia \
    -I..\..\common\protocol\alcatel -I..\..\common\device\bluetoth \
    -I..\..\common\phone\nokia -I..\..\common\service\backup \
    -I..\depend\nokia\dct3trac -I..\..\common\phone\alcatel \
    -I..\..\common\protocol\symbian -I..\..\common\phone\nokia\dct4 \
    -I..\..\common\phone\nokia\dct3 -I..\..\common\device\irda -I..\..\common \
    -I..\..\common\service -I..\..\common\protocol\nokia -I..\..\common\misc \
    -I..\..\common\phone\at -I..\..\common\protocol\at -I$(BCB)\include \
    -I$(BCB)\include\mfc -I$(BCB)\include\atl
PFLAGS = -N2Output -N0Output -$YD -$A8
RFLAGS = /l 0x415 /d "NDEBUG" /i$(BCB)\include;$(BCB)\include\mfc
AFLAGS = /mx /w2 /zd
LFLAGS = -IOutput -D"" -ap -Tpe -x -Gn -v -q
# ---------------------------------------------------------------------------
ALLOBJ = c0x32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cw32.lib
# ---------------------------------------------------------------------------
!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0

[Version Info Keys]
CompanyName=
FileDescription=
FileVersion=1.0.0.0
InternalName=
LegalCopyright=
LegalTrademarks=
OriginalFilename=
ProductName=
ProductVersion=1.0.0.0
Comments=

[Debugging]
DebugSourceDirs=$(BCB)\source\vcl

!endif





# ---------------------------------------------------------------------------
# MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

!if !$d(BCC32)
BCC32 = bcc32
!endif

!if !$d(CPP32)
CPP32 = cpp32
!endif

!if !$d(DCC32)
DCC32 = dcc32
!endif

!if !$d(TASM32)
TASM32 = tasm32
!endif

!if !$d(LINKER)
LINKER = ilink32
!endif

!if !$d(BRCC32)
BRCC32 = brcc32
!endif


# ---------------------------------------------------------------------------
!if $d(PATHCPP)
.PATH.CPP = $(PATHCPP)
.PATH.C   = $(PATHCPP)
!endif

!if $d(PATHPAS)
.PATH.PAS = $(PATHPAS)
!endif

!if $d(PATHASM)
.PATH.ASM = $(PATHASM)
!endif

!if $d(PATHRC)
.PATH.RC  = $(PATHRC)
!endif

!if $d(PATHOBJ)
.PATH.OBJ  = $(PATHOBJ)
!endif
# ---------------------------------------------------------------------------
$(PROJECT): $(OTHERFILES) $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!
# ---------------------------------------------------------------------------
.pas.hpp:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.pas.obj:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.cpp.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.cpp.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.asm.obj:
    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@

.rc.res:
    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<



# ---------------------------------------------------------------------------





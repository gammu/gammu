EPOCROOT=/home/matlec/symbian_sdk_series60
DLLTOOL=arm-epoc-pe-dlltool
LD=arm-epoc-pe-ld
AS=arm-epoc-pe-as
CC=arm-epoc-pe-gcc
CXX=arm-epoc-pe-gcc
CPP=arm-epoc-pe-cpp
PETRAN=petran
CPPFLAGS=-D_UNICODE -march=armv4t -mthumb-interwork -c -nostdinc -Wall -Wno-ctor-dtor-privacy -Wno-unknown-pragmas -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__AVKON_ELAF -D__MARM_ARMI__ -D__DLL__ $(INCLUDE)

INCLUDE=-I/home/matlec/symbian_sdk_series60/include -I/home/matlec/symbian_sdk_series60/include/libc -I/home/matlec/Downloads/gnokii-0.6.14/include
LIBDIR=$(EPOCROOT)/Release/armi/urel
STATLIBS=$(LIBDIR)/eexe.lib $(LIBDIR)/egcc.lib $(LIBDIR)/esock.lib $(LIBDIR)/bluetooth.lib

OBJS=gnapplet.o pkt.o device.o ircomm.o rfcomm.o tcpip.o proto_info.o proto_phonebook.o proto_netinfo.o proto_power.o proto_debug.o proto_sms.o proto_calendar.o proto_clock.o proto_profile.o 
UID1=0x1000007a
UID2=0x1000007a
UID3=0x107d4df2
LIBS=$(LIBDIR)/send.lib $(LIBDIR)/euser.lib $(LIBDIR)/insock.lib $(LIBDIR)/cntmodel.lib $(LIBDIR)/c32.lib $(LIBDIR)/charconv.lib $(LIBDIR)/plpvariant.lib $(LIBDIR)/btmanclient.lib $(LIBDIR)/logwrap.lib $(LIBDIR)/logcli.lib $(LIBDIR)/efsrv.lib $(LIBDIR)/etel.lib $(LIBDIR)/gsmbas.lib $(LIBDIR)/gsmadv.lib $(LIBDIR)/sysutil.lib $(LIBDIR)/hal.lib $(LIBDIR)/sysagt.lib $(LIBDIR)/msgs.lib $(LIBDIR)/smcm.lib $(LIBDIR)/gsmu.lib $(LIBDIR)/agnmodel.lib $(LIBDIR)/estor.lib $(LIBDIR)/etext.lib $(LIBDIR)/estlib.lib $(LIBDIR)/ealwl.lib $(LIBDIR)/profileengine.lib $(LIBDIR)/edbms.lib $(LIBDIR)/flogger.lib

all: gnapplet.sis

gnapplet.app: gnapplet_ui.o
	$(DLLTOOL) --as=$(AS) -m arm_interwork --output-def gnapplet.inf gnapplet_ui.o
	perl5.8.8 -I/home/matlec/symbian_sdk_series60/tools -S makedef.pl -Deffile gnapplet.inf -1 NewApplication__Fv gnapplet.def
	$(RM) gnapplet.inf
	$(DLLTOOL) --as=$(AS) -m arm_interwork --def gnapplet.def --output-exp gnapplet.exp --dllname gnapplet[$(UID3)].app
	$(LD) -e _E32Dll -u _E32Dll gnapplet.exp -dll --base-file gnapplet.bas -o $@.tmp gnapplet_ui.o $(LIBDIR)/edll.lib $(LIBDIR)/edllstub.lib $(LIBDIR)/egcc.lib $(LIBDIR)/euser.lib $(LIBDIR)/apparc.lib $(LIBDIR)/cone.lib $(LIBDIR)/eikcore.lib $(LIBDIR)/avkon.lib
	$(RM) gnapplet.exp $@.tmp
	$(DLLTOOL) --as=$(AS) -m arm_interwork --def gnapplet.def --dllname gnapplet[$(UID3)].app --base-file gnapplet.bas --output-exp gnapplet.exp
	$(RM) gnapplet.bas
	$(LD) -e _E32Dll -u _E32Dll --dll gnapplet.exp -Map gnapplet.app.map -o $@.tmp gnapplet_ui.o $(LIBDIR)/edll.lib $(LIBDIR)/edllstub.lib $(LIBDIR)/egcc.lib $(LIBDIR)/euser.lib $(LIBDIR)/apparc.lib $(LIBDIR)/cone.lib $(LIBDIR)/eikcore.lib $(LIBDIR)/avkon.lib
	$(RM) gnapplet.exp
	$(PETRAN) $@.tmp gnapplet.app -nocall -uid1 0x10000079 -uid2 0x100039ce -uid3 $(UID3)
	$(RM) $@.tmp

gnapplet.exe: $(OBJS)
	$(LD) -e _E32Startup -u _E32Startup --base-file gnapplet.bas -o $@.tmp $(OBJS) $(STATLIBS) $(LIBS)
	$(RM) $@.tmp
	$(DLLTOOL) --as=$(AS) -m arm_interwork --base-file gnapplet.bas --output-exp gnapplet.exp
	$(RM) gnapplet.bas
	$(LD) -e _E32Startup -u _E32Startup gnapplet.exp -Map gnapplet.exe.map -o $@.tmp $(OBJS) $(STATLIBS) $(LIBS)
	$(RM) gnapplet.exp
	$(PETRAN) $@.tmp gnapplet.exe -nocall -uid1 $(UID1) -uid2 $(UID2) -uid3 $(UID3)
	$(RM) $@.tmp

gnapplet.rsc: gnapplet.rss
	$(CPP) -I. $(INCLUDE) -DLANGUAGE_en -D_UNICODE < gnapplet.rss > gnapplet.rpp
	rcomp -u -ognapplet.rsc -hgnapplet.rsg -ignapplet.rss -sgnapplet.rpp

gnapplet.sis: gnapplet.pkg gnapplet.app gnapplet.exe gnapplet.rsc gnapplet.ini
	makesis gnapplet.pkg

clean:
	$(RM) *.tmp *.rsc *.rsg *.rpp *.o

distclean: clean
	$(RM) *.app.map *.exe.map *.app *.exe *.sis

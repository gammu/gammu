# C++BuilderX Version: 1.0.0.1786
# Generated GNU Makefile
# Do not modify, as your changes will be lost on re-export

# User Defined Variables:
ifndef BCBX
  BCBX := C:\Program Files2\BorlandCPP
endif
# End User Defined Variables

# Start of configurations
# If the user specified no configuration on the command line, set a default:
ifndef CFG
  CFG := Debug Build
endif
ifeq ($(CFG),Debug Build)
  BUILD_DIR := Debug
  Debug Build := 1
endif
ifeq ($(CFG),Release Build)
  BUILD_DIR := Release
  Release Build := 1
endif
ifneq ($(CFG),Debug Build)
  ifneq ($(CFG),Release Build)
    $(error An incorrect configuration was specified)
  endif
endif
# End of configurations

ifeq ($(CFG),Debug Build)
  # Default build target if none specified:
default: builddir Debug\gammu.exe

all: default

Debug\praca\gammu\common\protocol\alcatel\alcabus.obj: ..\..\common\protocol\alcatel\alcabus.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\alcatel\alcabus.obj   ..\..\common\protocol\alcatel\alcabus.c

Debug\praca\gammu\common\phone\alcatel\alcatel.obj: \
         ..\..\common\phone\alcatel\alcatel.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\alcatel\alcatel.obj   ..\..\common\phone\alcatel\alcatel.c

Debug\praca\gammu\common\protocol\at\at.obj: \
         ..\..\common\protocol\at\at.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\at\at.obj   ..\..\common\protocol\at\at.c

Debug\praca\gammu\common\phone\at\atgen.obj: ..\..\common\phone\at\atgen.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\at\atgen.obj   ..\..\common\phone\at\atgen.c

Debug\praca\gammu\common\service\backup\backics.obj: \
         ..\..\common\service\backup\backics.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backics.obj   ..\..\common\service\backup\backics.c

Debug\praca\gammu\common\service\backup\backldif.obj: \
         ..\..\common\service\backup\backldif.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backldif.obj   ..\..\common\service\backup\backldif.c

Debug\praca\gammu\common\service\backup\backlmb.obj: \
         ..\..\common\service\backup\backlmb.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backlmb.obj   ..\..\common\service\backup\backlmb.c

Debug\praca\gammu\common\service\backup\backtext.obj: \
         ..\..\common\service\backup\backtext.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backtext.obj   ..\..\common\service\backup\backtext.c

Debug\praca\gammu\common\service\backup\backvcf.obj: \
         ..\..\common\service\backup\backvcf.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backvcf.obj   ..\..\common\service\backup\backvcf.c

Debug\praca\gammu\common\service\backup\backvcs.obj: \
         ..\..\common\service\backup\backvcs.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backvcs.obj   ..\..\common\service\backup\backvcs.c

Debug\praca\gammu\common\device\bluetoth\bluetoth.obj: \
         ..\..\common\device\bluetoth\bluetoth.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\bluetoth\bluetoth.obj   ..\..\common\device\bluetoth\bluetoth.c

Debug\praca\gammu\common\misc\cfg.obj: ..\..\common\misc\cfg.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\cfg.obj   ..\..\common\misc\cfg.c

Debug\praca\gammu\common\misc\coding\coding.obj: ..\..\common\misc\coding\coding.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\coding\coding.obj   ..\..\common\misc\coding\coding.c

Debug\praca\gammu\gammu\depend\dct3.obj: ..\depend\dct3.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3.obj   ..\depend\dct3.c

Debug\praca\gammu\common\phone\nokia\dct3\dct3func.obj: ..\..\common\phone\nokia\dct3\dct3func.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\dct3func.obj   ..\..\common\phone\nokia\dct3\dct3func.c

Debug\praca\gammu\gammu\depend\dct4.obj: \
         ..\depend\dct4.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct4.obj   ..\depend\dct4.c

Debug\praca\gammu\common\phone\nokia\dct4\dct4func.obj: ..\..\common\phone\nokia\dct4\dct4func.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct4\dct4func.obj   ..\..\common\phone\nokia\dct4\dct4func.c

Debug\praca\gammu\common\device\devfunc.obj: \
         ..\..\common\device\devfunc.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\devfunc.obj   ..\..\common\device\devfunc.c

Debug\praca\gammu\gammu\depend\dsiemens.obj: ..\depend\dsiemens.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dsiemens.obj   ..\depend\dsiemens.c

Debug\praca\gammu\common\protocol\nokia\fbus2.obj: ..\..\common\protocol\nokia\fbus2.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\nokia\fbus2.obj   ..\..\common\protocol\nokia\fbus2.c

Debug\praca\gammu\gammu\gammu.obj: ..\gammu.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\gammu.obj   ..\gammu.c

Debug\praca\gammu\common\service\backup\gsmback.obj: ..\..\common\service\backup\gsmback.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\gsmback.obj   ..\..\common\service\backup\gsmback.c

Debug\praca\gammu\common\service\gsmcal.obj: \
         ..\..\common\service\gsmcal.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmcal.obj   ..\..\common\service\gsmcal.c

Debug\praca\gammu\common\gsmcomon.obj: ..\..\common\gsmcomon.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\gsmcomon.obj   ..\..\common\gsmcomon.c

Debug\praca\gammu\common\service\sms\gsmems.obj: ..\..\common\service\sms\gsmems.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\sms\gsmems.obj   ..\..\common\service\sms\gsmems.c

Debug\praca\gammu\common\service\gsmlogo.obj: ..\..\common\service\gsmlogo.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmlogo.obj   ..\..\common\service\gsmlogo.c

Debug\praca\gammu\common\service\gsmmisc.obj: ..\..\common\service\gsmmisc.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmmisc.obj   ..\..\common\service\gsmmisc.c

Debug\praca\gammu\common\service\sms\gsmmulti.obj: \
         ..\..\common\service\sms\gsmmulti.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\sms\gsmmulti.obj   ..\..\common\service\sms\gsmmulti.c

Debug\praca\gammu\common\service\gsmnet.obj: \
         ..\..\common\service\gsmnet.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmnet.obj   ..\..\common\service\gsmnet.c

Debug\praca\gammu\common\service\gsmpbk.obj: ..\..\common\service\gsmpbk.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmpbk.obj   ..\..\common\service\gsmpbk.c

Debug\praca\gammu\common\service\gsmring.obj: ..\..\common\service\gsmring.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmring.obj   ..\..\common\service\gsmring.c

Debug\praca\gammu\common\service\sms\gsmsms.obj: ..\..\common\service\sms\gsmsms.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\sms\gsmsms.obj   ..\..\common\service\sms\gsmsms.c

Debug\praca\gammu\common\gsmstate.obj: ..\..\common\gsmstate.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\gsmstate.obj   ..\..\common\gsmstate.c

Debug\praca\gammu\common\service\gsmwap.obj: ..\..\common\service\gsmwap.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmwap.obj   ..\..\common\service\gsmwap.c

Debug\praca\gammu\common\device\irda\irda.obj: ..\..\common\device\irda\irda.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\irda\irda.obj   ..\..\common\device\irda\irda.c

Debug\praca\gammu\common\protocol\nokia\mbus2.obj: \
         ..\..\common\protocol\nokia\mbus2.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\nokia\mbus2.obj   ..\..\common\protocol\nokia\mbus2.c

Debug\praca\gammu\common\misc\coding\md5.obj: \
         ..\..\common\misc\coding\md5.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\coding\md5.obj   ..\..\common\misc\coding\md5.c

Debug\praca\gammu\common\misc\misc.obj: ..\..\common\misc\misc.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\misc.obj   ..\..\common\misc\misc.c

Debug\praca\gammu\common\protocol\symbian\mrouter.obj: ..\..\common\protocol\symbian\mrouter.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\symbian\mrouter.obj   ..\..\common\protocol\symbian\mrouter.c

Debug\praca\gammu\common\phone\symbian\mroutgen.obj: \
         ..\..\common\phone\symbian\mroutgen.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\symbian\mroutgen.obj   ..\..\common\phone\symbian\mroutgen.c

Debug\praca\gammu\common\phone\nokia\dct4\n3650.obj: \
         ..\..\common\phone\nokia\dct4\n3650.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct4\n3650.obj   ..\..\common\phone\nokia\dct4\n3650.c

Debug\praca\gammu\common\phone\nokia\dct3\n6110.obj: \
         ..\..\common\phone\nokia\dct3\n6110.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\n6110.obj   ..\..\common\phone\nokia\dct3\n6110.c

Debug\praca\gammu\common\phone\nokia\dct4\n6510.obj: \
         ..\..\common\phone\nokia\dct4\n6510.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct4\n6510.obj   ..\..\common\phone\nokia\dct4\n6510.c

Debug\praca\gammu\common\phone\nokia\dct3\n7110.obj: \
         ..\..\common\phone\nokia\dct3\n7110.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\n7110.obj   ..\..\common\phone\nokia\dct3\n7110.c

Debug\praca\gammu\common\phone\nokia\dct3\n9210.obj: \
         ..\..\common\phone\nokia\dct3\n9210.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\n9210.obj   ..\..\common\phone\nokia\dct3\n9210.c

Debug\praca\gammu\common\phone\nokia\nauto.obj: \
         ..\..\common\phone\nokia\nauto.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\nauto.obj   ..\..\common\phone\nokia\nauto.c

Debug\praca\gammu\common\phone\nokia\nfunc.obj: \
         ..\..\common\phone\nokia\nfunc.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\nfunc.obj   ..\..\common\phone\nokia\nfunc.c

Debug\praca\gammu\common\phone\nokia\nfuncold.obj: \
         ..\..\common\phone\nokia\nfuncold.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\nfuncold.obj   ..\..\common\phone\nokia\nfuncold.c

Debug\praca\gammu\common\protocol\obex\obex.obj: \
         ..\..\common\protocol\obex\obex.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\obex\obex.obj   ..\..\common\protocol\obex\obex.c

Debug\praca\gammu\common\phone\obex\obexgen.obj: \
         ..\..\common\phone\obex\obexgen.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\obex\obexgen.obj   ..\..\common\phone\obex\obexgen.c

Debug\praca\gammu\common\phone\pfunc.obj: ..\..\common\phone\pfunc.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\pfunc.obj   ..\..\common\phone\pfunc.c

Debug\praca\gammu\common\protocol\nokia\phonet.obj: ..\..\common\protocol\nokia\phonet.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\nokia\phonet.obj   ..\..\common\protocol\nokia\phonet.c

Debug\praca\gammu\gammu\smsd\s_files.obj: ..\smsd\s_files.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\smsd\s_files.obj   ..\smsd\s_files.c

Debug\praca\gammu\common\device\serial\ser_w32.obj: ..\..\common\device\serial\ser_w32.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\serial\ser_w32.obj   ..\..\common\device\serial\ser_w32.c

Debug\praca\gammu\common\phone\at\siemens.obj: \
         ..\..\common\phone\at\siemens.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\at\siemens.obj   ..\..\common\phone\at\siemens.c

Debug\praca\gammu\gammu\smsd\smsdcore.obj: ..\smsd\smsdcore.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\smsd\smsdcore.obj   ..\smsd\smsdcore.c

Debug\praca\gammu\gammu\sniff.obj: ..\sniff.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\sniff.obj   ..\sniff.c

Debug\praca\gammu\common\phone\at\sonyeric.obj: ..\..\common\phone\at\sonyeric.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\at\sonyeric.obj   ..\..\common\phone\at\sonyeric.c

Debug\praca\gammu\gammu\depend\dct3trac\wmx.obj: \
         ..\depend\dct3trac\wmx.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx.obj   ..\depend\dct3trac\wmx.c

Debug\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj: ..\depend\dct3trac\wmx-gsm.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj   ..\depend\dct3trac\wmx-gsm.c

Debug\praca\gammu\gammu\depend\dct3trac\wmx-list.obj: \
         ..\depend\dct3trac\wmx-list.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-list.obj   ..\depend\dct3trac\wmx-list.c

Debug\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj: \
         ..\depend\dct3trac\wmx-sim.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj   ..\depend\dct3trac\wmx-sim.c

Debug\praca\gammu\gammu\depend\dct3trac\wmx-util.obj: \
         ..\depend\dct3trac\wmx-util.c
	bcc32 -DDEBUG -DWIN32 -g100 -j25 -w- -Od -r- -k -y -v -vi- -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-util.obj   ..\depend\dct3trac\wmx-util.c

Debug\gammu.exe: Debug\praca\gammu\common\protocol\alcatel\alcabus.obj Debug\praca\gammu\common\phone\alcatel\alcatel.obj \
         Debug\praca\gammu\common\protocol\at\at.obj Debug\praca\gammu\common\phone\at\atgen.obj \
         Debug\praca\gammu\common\service\backup\backics.obj Debug\praca\gammu\common\service\backup\backldif.obj \
         Debug\praca\gammu\common\service\backup\backlmb.obj Debug\praca\gammu\common\service\backup\backtext.obj \
         Debug\praca\gammu\common\service\backup\backvcf.obj Debug\praca\gammu\common\service\backup\backvcs.obj \
         Debug\praca\gammu\common\device\bluetoth\bluetoth.obj Debug\praca\gammu\common\misc\cfg.obj \
         Debug\praca\gammu\common\misc\coding\coding.obj Debug\praca\gammu\gammu\depend\dct3.obj \
         Debug\praca\gammu\common\phone\nokia\dct3\dct3func.obj Debug\praca\gammu\gammu\depend\dct4.obj \
         Debug\praca\gammu\common\phone\nokia\dct4\dct4func.obj Debug\praca\gammu\common\device\devfunc.obj \
         Debug\praca\gammu\gammu\depend\dsiemens.obj Debug\praca\gammu\common\protocol\nokia\fbus2.obj \
         Debug\praca\gammu\gammu\gammu.obj Debug\praca\gammu\common\service\backup\gsmback.obj \
         Debug\praca\gammu\common\service\gsmcal.obj Debug\praca\gammu\common\gsmcomon.obj \
         Debug\praca\gammu\common\service\sms\gsmems.obj Debug\praca\gammu\common\service\gsmlogo.obj \
         Debug\praca\gammu\common\service\gsmmisc.obj Debug\praca\gammu\common\service\sms\gsmmulti.obj \
         Debug\praca\gammu\common\service\gsmnet.obj Debug\praca\gammu\common\service\gsmpbk.obj \
         Debug\praca\gammu\common\service\gsmring.obj Debug\praca\gammu\common\service\sms\gsmsms.obj \
         Debug\praca\gammu\common\gsmstate.obj Debug\praca\gammu\common\service\gsmwap.obj \
         Debug\praca\gammu\common\device\irda\irda.obj Debug\praca\gammu\common\protocol\nokia\mbus2.obj \
         Debug\praca\gammu\common\misc\coding\md5.obj Debug\praca\gammu\common\misc\misc.obj \
         Debug\praca\gammu\common\protocol\symbian\mrouter.obj Debug\praca\gammu\common\phone\symbian\mroutgen.obj \
         Debug\praca\gammu\common\phone\nokia\dct4\n3650.obj Debug\praca\gammu\common\phone\nokia\dct3\n6110.obj \
         Debug\praca\gammu\common\phone\nokia\dct4\n6510.obj Debug\praca\gammu\common\phone\nokia\dct3\n7110.obj \
         Debug\praca\gammu\common\phone\nokia\dct3\n9210.obj Debug\praca\gammu\common\phone\nokia\nauto.obj \
         Debug\praca\gammu\common\phone\nokia\nfunc.obj Debug\praca\gammu\common\phone\nokia\nfuncold.obj \
         Debug\praca\gammu\common\protocol\obex\obex.obj Debug\praca\gammu\common\phone\obex\obexgen.obj \
         Debug\praca\gammu\common\phone\pfunc.obj Debug\praca\gammu\common\protocol\nokia\phonet.obj \
         Debug\praca\gammu\gammu\smsd\s_files.obj Debug\praca\gammu\common\device\serial\ser_w32.obj \
         Debug\praca\gammu\common\phone\at\siemens.obj Debug\praca\gammu\gammu\smsd\smsdcore.obj \
         Debug\praca\gammu\gammu\sniff.obj Debug\praca\gammu\common\phone\at\sonyeric.obj \
         Debug\praca\gammu\gammu\depend\dct3trac\wmx.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj \
         Debug\praca\gammu\gammu\depend\dct3trac\wmx-list.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj \
         Debug\praca\gammu\gammu\depend\dct3trac\wmx-util.obj
	ilink32 -D -ap -Tpe -x -Gn -v -L$(BCBX)\lib  c0x32.obj Debug\praca\gammu\common\protocol\alcatel\alcabus.obj Debug\praca\gammu\common\phone\alcatel\alcatel.obj Debug\praca\gammu\common\protocol\at\at.obj Debug\praca\gammu\common\phone\at\atgen.obj Debug\praca\gammu\common\service\backup\backics.obj Debug\praca\gammu\common\service\backup\backldif.obj Debug\praca\gammu\common\service\backup\backlmb.obj Debug\praca\gammu\common\service\backup\backtext.obj Debug\praca\gammu\common\service\backup\backvcf.obj Debug\praca\gammu\common\service\backup\backvcs.obj Debug\praca\gammu\common\device\bluetoth\bluetoth.obj Debug\praca\gammu\common\misc\cfg.obj Debug\praca\gammu\common\misc\coding\coding.obj Debug\praca\gammu\gammu\depend\dct3.obj Debug\praca\gammu\common\phone\nokia\dct3\dct3func.obj Debug\praca\gammu\gammu\depend\dct4.obj Debug\praca\gammu\common\phone\nokia\dct4\dct4func.obj Debug\praca\gammu\common\device\devfunc.obj Debug\praca\gammu\gammu\depend\dsiemens.obj Debug\praca\gammu\common\protocol\nokia\fbus2.obj Debug\praca\gammu\gammu\gammu.obj Debug\praca\gammu\common\service\backup\gsmback.obj Debug\praca\gammu\common\service\gsmcal.obj Debug\praca\gammu\common\gsmcomon.obj Debug\praca\gammu\common\service\sms\gsmems.obj Debug\praca\gammu\common\service\gsmlogo.obj Debug\praca\gammu\common\service\gsmmisc.obj Debug\praca\gammu\common\service\sms\gsmmulti.obj Debug\praca\gammu\common\service\gsmnet.obj Debug\praca\gammu\common\service\gsmpbk.obj Debug\praca\gammu\common\service\gsmring.obj Debug\praca\gammu\common\service\sms\gsmsms.obj Debug\praca\gammu\common\gsmstate.obj Debug\praca\gammu\common\service\gsmwap.obj Debug\praca\gammu\common\device\irda\irda.obj Debug\praca\gammu\common\protocol\nokia\mbus2.obj Debug\praca\gammu\common\misc\coding\md5.obj Debug\praca\gammu\common\misc\misc.obj Debug\praca\gammu\common\protocol\symbian\mrouter.obj Debug\praca\gammu\common\phone\symbian\mroutgen.obj Debug\praca\gammu\common\phone\nokia\dct4\n3650.obj Debug\praca\gammu\common\phone\nokia\dct3\n6110.obj Debug\praca\gammu\common\phone\nokia\dct4\n6510.obj Debug\praca\gammu\common\phone\nokia\dct3\n7110.obj Debug\praca\gammu\common\phone\nokia\dct3\n9210.obj Debug\praca\gammu\common\phone\nokia\nauto.obj Debug\praca\gammu\common\phone\nokia\nfunc.obj Debug\praca\gammu\common\phone\nokia\nfuncold.obj Debug\praca\gammu\common\protocol\obex\obex.obj Debug\praca\gammu\common\phone\obex\obexgen.obj Debug\praca\gammu\common\phone\pfunc.obj Debug\praca\gammu\common\protocol\nokia\phonet.obj Debug\praca\gammu\gammu\smsd\s_files.obj Debug\praca\gammu\common\device\serial\ser_w32.obj Debug\praca\gammu\common\phone\at\siemens.obj Debug\praca\gammu\gammu\smsd\smsdcore.obj Debug\praca\gammu\gammu\sniff.obj Debug\praca\gammu\common\phone\at\sonyeric.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx-list.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj Debug\praca\gammu\gammu\depend\dct3trac\wmx-util.obj,$(BUILD_DIR)\gammu.exe,,cw32.lib import32.lib,,

builddir:
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\protocol
	-mkdir Debug\praca\gammu\common\protocol\alcatel
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\alcatel
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\protocol
	-mkdir Debug\praca\gammu\common\protocol\at
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\at
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\service
	-mkdir Debug\praca\gammu\common\service\backup
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\device
	-mkdir Debug\praca\gammu\common\device\bluetoth
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\misc
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\misc
	-mkdir Debug\praca\gammu\common\misc\coding
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\depend
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\nokia
	-mkdir Debug\praca\gammu\common\phone\nokia\dct3
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\nokia
	-mkdir Debug\praca\gammu\common\phone\nokia\dct4
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\device
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\protocol
	-mkdir Debug\praca\gammu\common\protocol\nokia
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\service
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\service
	-mkdir Debug\praca\gammu\common\service\sms
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\device
	-mkdir Debug\praca\gammu\common\device\irda
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\protocol
	-mkdir Debug\praca\gammu\common\protocol\symbian
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\symbian
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\nokia
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\protocol
	-mkdir Debug\praca\gammu\common\protocol\obex
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug\praca\gammu\common\phone\obex
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\phone
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\smsd
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\common
	-mkdir Debug\praca\gammu\common\device
	-mkdir Debug\praca\gammu\common\device\serial
	-mkdir Debug
	-mkdir Debug\praca
	-mkdir Debug\praca\gammu
	-mkdir Debug\praca\gammu\depend
	-mkdir Debug\praca\gammu\depend\dct3trac
	-mkdir Debug

clean:
	-del Debug\praca\gammu\common\protocol\alcatel\alcabus.obj
	-del Debug\praca\gammu\common\protocol\alcatel\alcabus.csm
	-del Debug\praca\gammu\common\phone\alcatel\alcatel.obj
	-del Debug\praca\gammu\common\phone\alcatel\alcatel.csm
	-del Debug\praca\gammu\common\protocol\at\at.obj
	-del Debug\praca\gammu\common\protocol\at\at.csm
	-del Debug\praca\gammu\common\phone\at\atgen.obj
	-del Debug\praca\gammu\common\phone\at\atgen.csm
	-del Debug\praca\gammu\common\service\backup\backics.obj
	-del Debug\praca\gammu\common\service\backup\backics.csm
	-del Debug\praca\gammu\common\service\backup\backldif.obj
	-del Debug\praca\gammu\common\service\backup\backldif.csm
	-del Debug\praca\gammu\common\service\backup\backlmb.obj
	-del Debug\praca\gammu\common\service\backup\backlmb.csm
	-del Debug\praca\gammu\common\service\backup\backtext.obj
	-del Debug\praca\gammu\common\service\backup\backtext.csm
	-del Debug\praca\gammu\common\service\backup\backvcf.obj
	-del Debug\praca\gammu\common\service\backup\backvcf.csm
	-del Debug\praca\gammu\common\service\backup\backvcs.obj
	-del Debug\praca\gammu\common\service\backup\backvcs.csm
	-del Debug\praca\gammu\common\device\bluetoth\bluetoth.obj
	-del Debug\praca\gammu\common\device\bluetoth\bluetoth.csm
	-del Debug\praca\gammu\common\misc\cfg.obj
	-del Debug\praca\gammu\common\misc\cfg.csm
	-del Debug\praca\gammu\common\misc\coding\coding.obj
	-del Debug\praca\gammu\common\misc\coding\coding.csm
	-del Debug\praca\gammu\gammu\depend\dct3.obj
	-del Debug\praca\gammu\gammu\depend\dct3.csm
	-del Debug\praca\gammu\common\phone\nokia\dct3\dct3func.obj
	-del Debug\praca\gammu\common\phone\nokia\dct3\dct3func.csm
	-del Debug\praca\gammu\gammu\depend\dct4.obj
	-del Debug\praca\gammu\gammu\depend\dct4.csm
	-del Debug\praca\gammu\common\phone\nokia\dct4\dct4func.obj
	-del Debug\praca\gammu\common\phone\nokia\dct4\dct4func.csm
	-del Debug\praca\gammu\common\device\devfunc.obj
	-del Debug\praca\gammu\common\device\devfunc.csm
	-del Debug\praca\gammu\gammu\depend\dsiemens.obj
	-del Debug\praca\gammu\gammu\depend\dsiemens.csm
	-del Debug\praca\gammu\common\protocol\nokia\fbus2.obj
	-del Debug\praca\gammu\common\protocol\nokia\fbus2.csm
	-del Debug\praca\gammu\gammu\gammu.obj
	-del Debug\praca\gammu\gammu\gammu.csm
	-del Debug\praca\gammu\common\service\backup\gsmback.obj
	-del Debug\praca\gammu\common\service\backup\gsmback.csm
	-del Debug\praca\gammu\common\service\gsmcal.obj
	-del Debug\praca\gammu\common\service\gsmcal.csm
	-del Debug\praca\gammu\common\gsmcomon.obj
	-del Debug\praca\gammu\common\gsmcomon.csm
	-del Debug\praca\gammu\common\service\sms\gsmems.obj
	-del Debug\praca\gammu\common\service\sms\gsmems.csm
	-del Debug\praca\gammu\common\service\gsmlogo.obj
	-del Debug\praca\gammu\common\service\gsmlogo.csm
	-del Debug\praca\gammu\common\service\gsmmisc.obj
	-del Debug\praca\gammu\common\service\gsmmisc.csm
	-del Debug\praca\gammu\common\service\sms\gsmmulti.obj
	-del Debug\praca\gammu\common\service\sms\gsmmulti.csm
	-del Debug\praca\gammu\common\service\gsmnet.obj
	-del Debug\praca\gammu\common\service\gsmnet.csm
	-del Debug\praca\gammu\common\service\gsmpbk.obj
	-del Debug\praca\gammu\common\service\gsmpbk.csm
	-del Debug\praca\gammu\common\service\gsmring.obj
	-del Debug\praca\gammu\common\service\gsmring.csm
	-del Debug\praca\gammu\common\service\sms\gsmsms.obj
	-del Debug\praca\gammu\common\service\sms\gsmsms.csm
	-del Debug\praca\gammu\common\gsmstate.obj
	-del Debug\praca\gammu\common\gsmstate.csm
	-del Debug\praca\gammu\common\service\gsmwap.obj
	-del Debug\praca\gammu\common\service\gsmwap.csm
	-del Debug\praca\gammu\common\device\irda\irda.obj
	-del Debug\praca\gammu\common\device\irda\irda.csm
	-del Debug\praca\gammu\common\protocol\nokia\mbus2.obj
	-del Debug\praca\gammu\common\protocol\nokia\mbus2.csm
	-del Debug\praca\gammu\common\misc\coding\md5.obj
	-del Debug\praca\gammu\common\misc\coding\md5.csm
	-del Debug\praca\gammu\common\misc\misc.obj
	-del Debug\praca\gammu\common\misc\misc.csm
	-del Debug\praca\gammu\common\protocol\symbian\mrouter.obj
	-del Debug\praca\gammu\common\protocol\symbian\mrouter.csm
	-del Debug\praca\gammu\common\phone\symbian\mroutgen.obj
	-del Debug\praca\gammu\common\phone\symbian\mroutgen.csm
	-del Debug\praca\gammu\common\phone\nokia\dct4\n3650.obj
	-del Debug\praca\gammu\common\phone\nokia\dct4\n3650.csm
	-del Debug\praca\gammu\common\phone\nokia\dct3\n6110.obj
	-del Debug\praca\gammu\common\phone\nokia\dct3\n6110.csm
	-del Debug\praca\gammu\common\phone\nokia\dct4\n6510.obj
	-del Debug\praca\gammu\common\phone\nokia\dct4\n6510.csm
	-del Debug\praca\gammu\common\phone\nokia\dct3\n7110.obj
	-del Debug\praca\gammu\common\phone\nokia\dct3\n7110.csm
	-del Debug\praca\gammu\common\phone\nokia\dct3\n9210.obj
	-del Debug\praca\gammu\common\phone\nokia\dct3\n9210.csm
	-del Debug\praca\gammu\common\phone\nokia\nauto.obj
	-del Debug\praca\gammu\common\phone\nokia\nauto.csm
	-del Debug\praca\gammu\common\phone\nokia\nfunc.obj
	-del Debug\praca\gammu\common\phone\nokia\nfunc.csm
	-del Debug\praca\gammu\common\phone\nokia\nfuncold.obj
	-del Debug\praca\gammu\common\phone\nokia\nfuncold.csm
	-del Debug\praca\gammu\common\protocol\obex\obex.obj
	-del Debug\praca\gammu\common\protocol\obex\obex.csm
	-del Debug\praca\gammu\common\phone\obex\obexgen.obj
	-del Debug\praca\gammu\common\phone\obex\obexgen.csm
	-del Debug\praca\gammu\common\phone\pfunc.obj
	-del Debug\praca\gammu\common\phone\pfunc.csm
	-del Debug\praca\gammu\common\protocol\nokia\phonet.obj
	-del Debug\praca\gammu\common\protocol\nokia\phonet.csm
	-del Debug\praca\gammu\gammu\smsd\s_files.obj
	-del Debug\praca\gammu\gammu\smsd\s_files.csm
	-del Debug\praca\gammu\common\device\serial\ser_w32.obj
	-del Debug\praca\gammu\common\device\serial\ser_w32.csm
	-del Debug\praca\gammu\common\phone\at\siemens.obj
	-del Debug\praca\gammu\common\phone\at\siemens.csm
	-del Debug\praca\gammu\gammu\smsd\smsdcore.obj
	-del Debug\praca\gammu\gammu\smsd\smsdcore.csm
	-del Debug\praca\gammu\gammu\sniff.obj
	-del Debug\praca\gammu\gammu\sniff.csm
	-del Debug\praca\gammu\common\phone\at\sonyeric.obj
	-del Debug\praca\gammu\common\phone\at\sonyeric.csm
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx.obj
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx.csm
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-gsm.csm
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-list.obj
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-list.csm
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-sim.csm
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-util.obj
	-del Debug\praca\gammu\gammu\depend\dct3trac\wmx-util.csm
	-del Debug\gammu.exe
	-del Debug\gammu.map
	-del Debug\gammu.ilc
	-del Debug\gammu.ilf
	-del Debug\gammu.ils
	-del Debug\gammu.tds
	-del Debug\gammu.ild

endif
ifeq ($(CFG),Release Build)
  # Default build target if none specified:
default: builddir Release\gammu.exe

all: default

Release\praca\gammu\common\protocol\alcatel\alcabus.obj: ..\..\common\protocol\alcatel\alcabus.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\alcatel\alcabus.obj   ..\..\common\protocol\alcatel\alcabus.c

Release\praca\gammu\common\phone\alcatel\alcatel.obj: \
         ..\..\common\phone\alcatel\alcatel.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\alcatel\alcatel.obj   ..\..\common\phone\alcatel\alcatel.c

Release\praca\gammu\common\protocol\at\at.obj: \
         ..\..\common\protocol\at\at.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\at\at.obj   ..\..\common\protocol\at\at.c

Release\praca\gammu\common\phone\at\atgen.obj: ..\..\common\phone\at\atgen.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\at\atgen.obj   ..\..\common\phone\at\atgen.c

Release\praca\gammu\common\service\backup\backics.obj: \
         ..\..\common\service\backup\backics.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backics.obj   ..\..\common\service\backup\backics.c

Release\praca\gammu\common\service\backup\backldif.obj: \
         ..\..\common\service\backup\backldif.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backldif.obj   ..\..\common\service\backup\backldif.c

Release\praca\gammu\common\service\backup\backlmb.obj: \
         ..\..\common\service\backup\backlmb.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backlmb.obj   ..\..\common\service\backup\backlmb.c

Release\praca\gammu\common\service\backup\backtext.obj: \
         ..\..\common\service\backup\backtext.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backtext.obj   ..\..\common\service\backup\backtext.c

Release\praca\gammu\common\service\backup\backvcf.obj: \
         ..\..\common\service\backup\backvcf.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backvcf.obj   ..\..\common\service\backup\backvcf.c

Release\praca\gammu\common\service\backup\backvcs.obj: \
         ..\..\common\service\backup\backvcs.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\backvcs.obj   ..\..\common\service\backup\backvcs.c

Release\praca\gammu\common\device\bluetoth\bluetoth.obj: \
         ..\..\common\device\bluetoth\bluetoth.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\bluetoth\bluetoth.obj   ..\..\common\device\bluetoth\bluetoth.c

Release\praca\gammu\common\misc\cfg.obj: \
         ..\..\common\misc\cfg.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\cfg.obj   ..\..\common\misc\cfg.c

Release\praca\gammu\common\misc\coding\coding.obj: ..\..\common\misc\coding\coding.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\coding\coding.obj   ..\..\common\misc\coding\coding.c

Release\praca\gammu\gammu\depend\dct3.obj: ..\depend\dct3.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3.obj   ..\depend\dct3.c

Release\praca\gammu\common\phone\nokia\dct3\dct3func.obj: ..\..\common\phone\nokia\dct3\dct3func.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\dct3func.obj   ..\..\common\phone\nokia\dct3\dct3func.c

Release\praca\gammu\gammu\depend\dct4.obj: \
         ..\depend\dct4.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct4.obj   ..\depend\dct4.c

Release\praca\gammu\common\phone\nokia\dct4\dct4func.obj: ..\..\common\phone\nokia\dct4\dct4func.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct4\dct4func.obj   ..\..\common\phone\nokia\dct4\dct4func.c

Release\praca\gammu\common\device\devfunc.obj: \
         ..\..\common\device\devfunc.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\devfunc.obj   ..\..\common\device\devfunc.c

Release\praca\gammu\gammu\depend\dsiemens.obj: ..\depend\dsiemens.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dsiemens.obj   ..\depend\dsiemens.c

Release\praca\gammu\common\protocol\nokia\fbus2.obj: ..\..\common\protocol\nokia\fbus2.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\nokia\fbus2.obj   ..\..\common\protocol\nokia\fbus2.c

Release\praca\gammu\gammu\gammu.obj: ..\gammu.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\gammu.obj   ..\gammu.c

Release\praca\gammu\common\service\backup\gsmback.obj: ..\..\common\service\backup\gsmback.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\backup\gsmback.obj   ..\..\common\service\backup\gsmback.c

Release\praca\gammu\common\service\gsmcal.obj: \
         ..\..\common\service\gsmcal.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmcal.obj   ..\..\common\service\gsmcal.c

Release\praca\gammu\common\gsmcomon.obj: ..\..\common\gsmcomon.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\gsmcomon.obj   ..\..\common\gsmcomon.c

Release\praca\gammu\common\service\sms\gsmems.obj: ..\..\common\service\sms\gsmems.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\sms\gsmems.obj   ..\..\common\service\sms\gsmems.c

Release\praca\gammu\common\service\gsmlogo.obj: \
         ..\..\common\service\gsmlogo.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmlogo.obj   ..\..\common\service\gsmlogo.c

Release\praca\gammu\common\service\gsmmisc.obj: ..\..\common\service\gsmmisc.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmmisc.obj   ..\..\common\service\gsmmisc.c

Release\praca\gammu\common\service\sms\gsmmulti.obj: \
         ..\..\common\service\sms\gsmmulti.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\sms\gsmmulti.obj   ..\..\common\service\sms\gsmmulti.c

Release\praca\gammu\common\service\gsmnet.obj: \
         ..\..\common\service\gsmnet.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmnet.obj   ..\..\common\service\gsmnet.c

Release\praca\gammu\common\service\gsmpbk.obj: ..\..\common\service\gsmpbk.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmpbk.obj   ..\..\common\service\gsmpbk.c

Release\praca\gammu\common\service\gsmring.obj: ..\..\common\service\gsmring.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmring.obj   ..\..\common\service\gsmring.c

Release\praca\gammu\common\service\sms\gsmsms.obj: \
         ..\..\common\service\sms\gsmsms.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\sms\gsmsms.obj   ..\..\common\service\sms\gsmsms.c

Release\praca\gammu\common\gsmstate.obj: ..\..\common\gsmstate.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\gsmstate.obj   ..\..\common\gsmstate.c

Release\praca\gammu\common\service\gsmwap.obj: ..\..\common\service\gsmwap.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\service\gsmwap.obj   ..\..\common\service\gsmwap.c

Release\praca\gammu\common\device\irda\irda.obj: ..\..\common\device\irda\irda.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\irda\irda.obj   ..\..\common\device\irda\irda.c

Release\praca\gammu\common\protocol\nokia\mbus2.obj: \
         ..\..\common\protocol\nokia\mbus2.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\nokia\mbus2.obj   ..\..\common\protocol\nokia\mbus2.c

Release\praca\gammu\common\misc\coding\md5.obj: \
         ..\..\common\misc\coding\md5.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\coding\md5.obj   ..\..\common\misc\coding\md5.c

Release\praca\gammu\common\misc\misc.obj: ..\..\common\misc\misc.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\misc\misc.obj   ..\..\common\misc\misc.c

Release\praca\gammu\common\protocol\symbian\mrouter.obj: \
         ..\..\common\protocol\symbian\mrouter.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\symbian\mrouter.obj   ..\..\common\protocol\symbian\mrouter.c

Release\praca\gammu\common\phone\symbian\mroutgen.obj: \
         ..\..\common\phone\symbian\mroutgen.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\symbian\mroutgen.obj   ..\..\common\phone\symbian\mroutgen.c

Release\praca\gammu\common\phone\nokia\dct4\n3650.obj: \
         ..\..\common\phone\nokia\dct4\n3650.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct4\n3650.obj   ..\..\common\phone\nokia\dct4\n3650.c

Release\praca\gammu\common\phone\nokia\dct3\n6110.obj: \
         ..\..\common\phone\nokia\dct3\n6110.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\n6110.obj   ..\..\common\phone\nokia\dct3\n6110.c

Release\praca\gammu\common\phone\nokia\dct4\n6510.obj: \
         ..\..\common\phone\nokia\dct4\n6510.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct4\n6510.obj   ..\..\common\phone\nokia\dct4\n6510.c

Release\praca\gammu\common\phone\nokia\dct3\n7110.obj: \
         ..\..\common\phone\nokia\dct3\n7110.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\n7110.obj   ..\..\common\phone\nokia\dct3\n7110.c

Release\praca\gammu\common\phone\nokia\dct3\n9210.obj: \
         ..\..\common\phone\nokia\dct3\n9210.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\dct3\n9210.obj   ..\..\common\phone\nokia\dct3\n9210.c

Release\praca\gammu\common\phone\nokia\nauto.obj: \
         ..\..\common\phone\nokia\nauto.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\nauto.obj   ..\..\common\phone\nokia\nauto.c

Release\praca\gammu\common\phone\nokia\nfunc.obj: \
         ..\..\common\phone\nokia\nfunc.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\nfunc.obj   ..\..\common\phone\nokia\nfunc.c

Release\praca\gammu\common\phone\nokia\nfuncold.obj: \
         ..\..\common\phone\nokia\nfuncold.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\nokia\nfuncold.obj   ..\..\common\phone\nokia\nfuncold.c

Release\praca\gammu\common\protocol\obex\obex.obj: \
         ..\..\common\protocol\obex\obex.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\obex\obex.obj   ..\..\common\protocol\obex\obex.c

Release\praca\gammu\common\phone\obex\obexgen.obj: \
         ..\..\common\phone\obex\obexgen.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\obex\obexgen.obj   ..\..\common\phone\obex\obexgen.c

Release\praca\gammu\common\phone\pfunc.obj: ..\..\common\phone\pfunc.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\pfunc.obj   ..\..\common\phone\pfunc.c

Release\praca\gammu\common\protocol\nokia\phonet.obj: \
         ..\..\common\protocol\nokia\phonet.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\protocol\nokia\phonet.obj   ..\..\common\protocol\nokia\phonet.c

Release\praca\gammu\gammu\smsd\s_files.obj: \
         ..\smsd\s_files.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\smsd\s_files.obj   ..\smsd\s_files.c

Release\praca\gammu\common\device\serial\ser_w32.obj: ..\..\common\device\serial\ser_w32.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\device\serial\ser_w32.obj   ..\..\common\device\serial\ser_w32.c

Release\praca\gammu\common\phone\at\siemens.obj: \
         ..\..\common\phone\at\siemens.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\at\siemens.obj   ..\..\common\phone\at\siemens.c

Release\praca\gammu\gammu\smsd\smsdcore.obj: ..\smsd\smsdcore.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\smsd\smsdcore.obj   ..\smsd\smsdcore.c

Release\praca\gammu\gammu\sniff.obj: ..\sniff.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\sniff.obj   ..\sniff.c

Release\praca\gammu\common\phone\at\sonyeric.obj: ..\..\common\phone\at\sonyeric.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\common\phone\at\sonyeric.obj   ..\..\common\phone\at\sonyeric.c

Release\praca\gammu\gammu\depend\dct3trac\wmx.obj: \
         ..\depend\dct3trac\wmx.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx.obj   ..\depend\dct3trac\wmx.c

Release\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj: ..\depend\dct3trac\wmx-gsm.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj   ..\depend\dct3trac\wmx-gsm.c

Release\praca\gammu\gammu\depend\dct3trac\wmx-list.obj: \
         ..\depend\dct3trac\wmx-list.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-list.obj   ..\depend\dct3trac\wmx-list.c

Release\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj: \
         ..\depend\dct3trac\wmx-sim.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj   ..\depend\dct3trac\wmx-sim.c

Release\praca\gammu\gammu\depend\dct3trac\wmx-util.obj: \
         ..\depend\dct3trac\wmx-util.c
	bcc32 -D -DWIN32 -g100 -j25 -w- -O2 -k- -vi -tWC -c -I$(BCBX)\include -o$(BUILD_DIR)\praca\gammu\gammu\depend\dct3trac\wmx-util.obj   ..\depend\dct3trac\wmx-util.c

Release\gammu.exe: Release\praca\gammu\common\protocol\alcatel\alcabus.obj Release\praca\gammu\common\phone\alcatel\alcatel.obj \
         Release\praca\gammu\common\protocol\at\at.obj Release\praca\gammu\common\phone\at\atgen.obj \
         Release\praca\gammu\common\service\backup\backics.obj Release\praca\gammu\common\service\backup\backldif.obj \
         Release\praca\gammu\common\service\backup\backlmb.obj Release\praca\gammu\common\service\backup\backtext.obj \
         Release\praca\gammu\common\service\backup\backvcf.obj Release\praca\gammu\common\service\backup\backvcs.obj \
         Release\praca\gammu\common\device\bluetoth\bluetoth.obj Release\praca\gammu\common\misc\cfg.obj \
         Release\praca\gammu\common\misc\coding\coding.obj Release\praca\gammu\gammu\depend\dct3.obj \
         Release\praca\gammu\common\phone\nokia\dct3\dct3func.obj Release\praca\gammu\gammu\depend\dct4.obj \
         Release\praca\gammu\common\phone\nokia\dct4\dct4func.obj Release\praca\gammu\common\device\devfunc.obj \
         Release\praca\gammu\gammu\depend\dsiemens.obj Release\praca\gammu\common\protocol\nokia\fbus2.obj \
         Release\praca\gammu\gammu\gammu.obj Release\praca\gammu\common\service\backup\gsmback.obj \
         Release\praca\gammu\common\service\gsmcal.obj Release\praca\gammu\common\gsmcomon.obj \
         Release\praca\gammu\common\service\sms\gsmems.obj Release\praca\gammu\common\service\gsmlogo.obj \
         Release\praca\gammu\common\service\gsmmisc.obj Release\praca\gammu\common\service\sms\gsmmulti.obj \
         Release\praca\gammu\common\service\gsmnet.obj Release\praca\gammu\common\service\gsmpbk.obj \
         Release\praca\gammu\common\service\gsmring.obj Release\praca\gammu\common\service\sms\gsmsms.obj \
         Release\praca\gammu\common\gsmstate.obj Release\praca\gammu\common\service\gsmwap.obj \
         Release\praca\gammu\common\device\irda\irda.obj Release\praca\gammu\common\protocol\nokia\mbus2.obj \
         Release\praca\gammu\common\misc\coding\md5.obj Release\praca\gammu\common\misc\misc.obj \
         Release\praca\gammu\common\protocol\symbian\mrouter.obj Release\praca\gammu\common\phone\symbian\mroutgen.obj \
         Release\praca\gammu\common\phone\nokia\dct4\n3650.obj Release\praca\gammu\common\phone\nokia\dct3\n6110.obj \
         Release\praca\gammu\common\phone\nokia\dct4\n6510.obj Release\praca\gammu\common\phone\nokia\dct3\n7110.obj \
         Release\praca\gammu\common\phone\nokia\dct3\n9210.obj Release\praca\gammu\common\phone\nokia\nauto.obj \
         Release\praca\gammu\common\phone\nokia\nfunc.obj Release\praca\gammu\common\phone\nokia\nfuncold.obj \
         Release\praca\gammu\common\protocol\obex\obex.obj Release\praca\gammu\common\phone\obex\obexgen.obj \
         Release\praca\gammu\common\phone\pfunc.obj Release\praca\gammu\common\protocol\nokia\phonet.obj \
         Release\praca\gammu\gammu\smsd\s_files.obj Release\praca\gammu\common\device\serial\ser_w32.obj \
         Release\praca\gammu\common\phone\at\siemens.obj Release\praca\gammu\gammu\smsd\smsdcore.obj \
         Release\praca\gammu\gammu\sniff.obj Release\praca\gammu\common\phone\at\sonyeric.obj \
         Release\praca\gammu\gammu\depend\dct3trac\wmx.obj Release\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj \
         Release\praca\gammu\gammu\depend\dct3trac\wmx-list.obj Release\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj \
         Release\praca\gammu\gammu\depend\dct3trac\wmx-util.obj
	ilink32 -ap -Tpe -x -Gn -L$(BCBX)\lib  c0x32.obj Release\praca\gammu\common\protocol\alcatel\alcabus.obj Release\praca\gammu\common\phone\alcatel\alcatel.obj Release\praca\gammu\common\protocol\at\at.obj Release\praca\gammu\common\phone\at\atgen.obj Release\praca\gammu\common\service\backup\backics.obj Release\praca\gammu\common\service\backup\backldif.obj Release\praca\gammu\common\service\backup\backlmb.obj Release\praca\gammu\common\service\backup\backtext.obj Release\praca\gammu\common\service\backup\backvcf.obj Release\praca\gammu\common\service\backup\backvcs.obj Release\praca\gammu\common\device\bluetoth\bluetoth.obj Release\praca\gammu\common\misc\cfg.obj Release\praca\gammu\common\misc\coding\coding.obj Release\praca\gammu\gammu\depend\dct3.obj Release\praca\gammu\common\phone\nokia\dct3\dct3func.obj Release\praca\gammu\gammu\depend\dct4.obj Release\praca\gammu\common\phone\nokia\dct4\dct4func.obj Release\praca\gammu\common\device\devfunc.obj Release\praca\gammu\gammu\depend\dsiemens.obj Release\praca\gammu\common\protocol\nokia\fbus2.obj Release\praca\gammu\gammu\gammu.obj Release\praca\gammu\common\service\backup\gsmback.obj Release\praca\gammu\common\service\gsmcal.obj Release\praca\gammu\common\gsmcomon.obj Release\praca\gammu\common\service\sms\gsmems.obj Release\praca\gammu\common\service\gsmlogo.obj Release\praca\gammu\common\service\gsmmisc.obj Release\praca\gammu\common\service\sms\gsmmulti.obj Release\praca\gammu\common\service\gsmnet.obj Release\praca\gammu\common\service\gsmpbk.obj Release\praca\gammu\common\service\gsmring.obj Release\praca\gammu\common\service\sms\gsmsms.obj Release\praca\gammu\common\gsmstate.obj Release\praca\gammu\common\service\gsmwap.obj Release\praca\gammu\common\device\irda\irda.obj Release\praca\gammu\common\protocol\nokia\mbus2.obj Release\praca\gammu\common\misc\coding\md5.obj Release\praca\gammu\common\misc\misc.obj Release\praca\gammu\common\protocol\symbian\mrouter.obj Release\praca\gammu\common\phone\symbian\mroutgen.obj Release\praca\gammu\common\phone\nokia\dct4\n3650.obj Release\praca\gammu\common\phone\nokia\dct3\n6110.obj Release\praca\gammu\common\phone\nokia\dct4\n6510.obj Release\praca\gammu\common\phone\nokia\dct3\n7110.obj Release\praca\gammu\common\phone\nokia\dct3\n9210.obj Release\praca\gammu\common\phone\nokia\nauto.obj Release\praca\gammu\common\phone\nokia\nfunc.obj Release\praca\gammu\common\phone\nokia\nfuncold.obj Release\praca\gammu\common\protocol\obex\obex.obj Release\praca\gammu\common\phone\obex\obexgen.obj Release\praca\gammu\common\phone\pfunc.obj Release\praca\gammu\common\protocol\nokia\phonet.obj Release\praca\gammu\gammu\smsd\s_files.obj Release\praca\gammu\common\device\serial\ser_w32.obj Release\praca\gammu\common\phone\at\siemens.obj Release\praca\gammu\gammu\smsd\smsdcore.obj Release\praca\gammu\gammu\sniff.obj Release\praca\gammu\common\phone\at\sonyeric.obj Release\praca\gammu\gammu\depend\dct3trac\wmx.obj Release\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj Release\praca\gammu\gammu\depend\dct3trac\wmx-list.obj Release\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj Release\praca\gammu\gammu\depend\dct3trac\wmx-util.obj,$(BUILD_DIR)\gammu.exe,,cw32.lib import32.lib,,

builddir:
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\protocol
	-mkdir Release\praca\gammu\common\protocol\alcatel
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\alcatel
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\protocol
	-mkdir Release\praca\gammu\common\protocol\at
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\at
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\service
	-mkdir Release\praca\gammu\common\service\backup
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\device
	-mkdir Release\praca\gammu\common\device\bluetoth
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\misc
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\misc
	-mkdir Release\praca\gammu\common\misc\coding
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\depend
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\nokia
	-mkdir Release\praca\gammu\common\phone\nokia\dct3
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\nokia
	-mkdir Release\praca\gammu\common\phone\nokia\dct4
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\device
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\protocol
	-mkdir Release\praca\gammu\common\protocol\nokia
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\service
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\service
	-mkdir Release\praca\gammu\common\service\sms
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\device
	-mkdir Release\praca\gammu\common\device\irda
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\protocol
	-mkdir Release\praca\gammu\common\protocol\symbian
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\symbian
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\nokia
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\protocol
	-mkdir Release\praca\gammu\common\protocol\obex
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release\praca\gammu\common\phone\obex
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\phone
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\smsd
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\common
	-mkdir Release\praca\gammu\common\device
	-mkdir Release\praca\gammu\common\device\serial
	-mkdir Release
	-mkdir Release\praca
	-mkdir Release\praca\gammu
	-mkdir Release\praca\gammu\depend
	-mkdir Release\praca\gammu\depend\dct3trac
	-mkdir Release

clean:
	-del Release\praca\gammu\common\protocol\alcatel\alcabus.obj
	-del Release\praca\gammu\common\protocol\alcatel\alcabus.csm
	-del Release\praca\gammu\common\phone\alcatel\alcatel.obj
	-del Release\praca\gammu\common\phone\alcatel\alcatel.csm
	-del Release\praca\gammu\common\protocol\at\at.obj
	-del Release\praca\gammu\common\protocol\at\at.csm
	-del Release\praca\gammu\common\phone\at\atgen.obj
	-del Release\praca\gammu\common\phone\at\atgen.csm
	-del Release\praca\gammu\common\service\backup\backics.obj
	-del Release\praca\gammu\common\service\backup\backics.csm
	-del Release\praca\gammu\common\service\backup\backldif.obj
	-del Release\praca\gammu\common\service\backup\backldif.csm
	-del Release\praca\gammu\common\service\backup\backlmb.obj
	-del Release\praca\gammu\common\service\backup\backlmb.csm
	-del Release\praca\gammu\common\service\backup\backtext.obj
	-del Release\praca\gammu\common\service\backup\backtext.csm
	-del Release\praca\gammu\common\service\backup\backvcf.obj
	-del Release\praca\gammu\common\service\backup\backvcf.csm
	-del Release\praca\gammu\common\service\backup\backvcs.obj
	-del Release\praca\gammu\common\service\backup\backvcs.csm
	-del Release\praca\gammu\common\device\bluetoth\bluetoth.obj
	-del Release\praca\gammu\common\device\bluetoth\bluetoth.csm
	-del Release\praca\gammu\common\misc\cfg.obj
	-del Release\praca\gammu\common\misc\cfg.csm
	-del Release\praca\gammu\common\misc\coding\coding.obj
	-del Release\praca\gammu\common\misc\coding\coding.csm
	-del Release\praca\gammu\gammu\depend\dct3.obj
	-del Release\praca\gammu\gammu\depend\dct3.csm
	-del Release\praca\gammu\common\phone\nokia\dct3\dct3func.obj
	-del Release\praca\gammu\common\phone\nokia\dct3\dct3func.csm
	-del Release\praca\gammu\gammu\depend\dct4.obj
	-del Release\praca\gammu\gammu\depend\dct4.csm
	-del Release\praca\gammu\common\phone\nokia\dct4\dct4func.obj
	-del Release\praca\gammu\common\phone\nokia\dct4\dct4func.csm
	-del Release\praca\gammu\common\device\devfunc.obj
	-del Release\praca\gammu\common\device\devfunc.csm
	-del Release\praca\gammu\gammu\depend\dsiemens.obj
	-del Release\praca\gammu\gammu\depend\dsiemens.csm
	-del Release\praca\gammu\common\protocol\nokia\fbus2.obj
	-del Release\praca\gammu\common\protocol\nokia\fbus2.csm
	-del Release\praca\gammu\gammu\gammu.obj
	-del Release\praca\gammu\gammu\gammu.csm
	-del Release\praca\gammu\common\service\backup\gsmback.obj
	-del Release\praca\gammu\common\service\backup\gsmback.csm
	-del Release\praca\gammu\common\service\gsmcal.obj
	-del Release\praca\gammu\common\service\gsmcal.csm
	-del Release\praca\gammu\common\gsmcomon.obj
	-del Release\praca\gammu\common\gsmcomon.csm
	-del Release\praca\gammu\common\service\sms\gsmems.obj
	-del Release\praca\gammu\common\service\sms\gsmems.csm
	-del Release\praca\gammu\common\service\gsmlogo.obj
	-del Release\praca\gammu\common\service\gsmlogo.csm
	-del Release\praca\gammu\common\service\gsmmisc.obj
	-del Release\praca\gammu\common\service\gsmmisc.csm
	-del Release\praca\gammu\common\service\sms\gsmmulti.obj
	-del Release\praca\gammu\common\service\sms\gsmmulti.csm
	-del Release\praca\gammu\common\service\gsmnet.obj
	-del Release\praca\gammu\common\service\gsmnet.csm
	-del Release\praca\gammu\common\service\gsmpbk.obj
	-del Release\praca\gammu\common\service\gsmpbk.csm
	-del Release\praca\gammu\common\service\gsmring.obj
	-del Release\praca\gammu\common\service\gsmring.csm
	-del Release\praca\gammu\common\service\sms\gsmsms.obj
	-del Release\praca\gammu\common\service\sms\gsmsms.csm
	-del Release\praca\gammu\common\gsmstate.obj
	-del Release\praca\gammu\common\gsmstate.csm
	-del Release\praca\gammu\common\service\gsmwap.obj
	-del Release\praca\gammu\common\service\gsmwap.csm
	-del Release\praca\gammu\common\device\irda\irda.obj
	-del Release\praca\gammu\common\device\irda\irda.csm
	-del Release\praca\gammu\common\protocol\nokia\mbus2.obj
	-del Release\praca\gammu\common\protocol\nokia\mbus2.csm
	-del Release\praca\gammu\common\misc\coding\md5.obj
	-del Release\praca\gammu\common\misc\coding\md5.csm
	-del Release\praca\gammu\common\misc\misc.obj
	-del Release\praca\gammu\common\misc\misc.csm
	-del Release\praca\gammu\common\protocol\symbian\mrouter.obj
	-del Release\praca\gammu\common\protocol\symbian\mrouter.csm
	-del Release\praca\gammu\common\phone\symbian\mroutgen.obj
	-del Release\praca\gammu\common\phone\symbian\mroutgen.csm
	-del Release\praca\gammu\common\phone\nokia\dct4\n3650.obj
	-del Release\praca\gammu\common\phone\nokia\dct4\n3650.csm
	-del Release\praca\gammu\common\phone\nokia\dct3\n6110.obj
	-del Release\praca\gammu\common\phone\nokia\dct3\n6110.csm
	-del Release\praca\gammu\common\phone\nokia\dct4\n6510.obj
	-del Release\praca\gammu\common\phone\nokia\dct4\n6510.csm
	-del Release\praca\gammu\common\phone\nokia\dct3\n7110.obj
	-del Release\praca\gammu\common\phone\nokia\dct3\n7110.csm
	-del Release\praca\gammu\common\phone\nokia\dct3\n9210.obj
	-del Release\praca\gammu\common\phone\nokia\dct3\n9210.csm
	-del Release\praca\gammu\common\phone\nokia\nauto.obj
	-del Release\praca\gammu\common\phone\nokia\nauto.csm
	-del Release\praca\gammu\common\phone\nokia\nfunc.obj
	-del Release\praca\gammu\common\phone\nokia\nfunc.csm
	-del Release\praca\gammu\common\phone\nokia\nfuncold.obj
	-del Release\praca\gammu\common\phone\nokia\nfuncold.csm
	-del Release\praca\gammu\common\protocol\obex\obex.obj
	-del Release\praca\gammu\common\protocol\obex\obex.csm
	-del Release\praca\gammu\common\phone\obex\obexgen.obj
	-del Release\praca\gammu\common\phone\obex\obexgen.csm
	-del Release\praca\gammu\common\phone\pfunc.obj
	-del Release\praca\gammu\common\phone\pfunc.csm
	-del Release\praca\gammu\common\protocol\nokia\phonet.obj
	-del Release\praca\gammu\common\protocol\nokia\phonet.csm
	-del Release\praca\gammu\gammu\smsd\s_files.obj
	-del Release\praca\gammu\gammu\smsd\s_files.csm
	-del Release\praca\gammu\common\device\serial\ser_w32.obj
	-del Release\praca\gammu\common\device\serial\ser_w32.csm
	-del Release\praca\gammu\common\phone\at\siemens.obj
	-del Release\praca\gammu\common\phone\at\siemens.csm
	-del Release\praca\gammu\gammu\smsd\smsdcore.obj
	-del Release\praca\gammu\gammu\smsd\smsdcore.csm
	-del Release\praca\gammu\gammu\sniff.obj
	-del Release\praca\gammu\gammu\sniff.csm
	-del Release\praca\gammu\common\phone\at\sonyeric.obj
	-del Release\praca\gammu\common\phone\at\sonyeric.csm
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx.obj
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx.csm
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-gsm.obj
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-gsm.csm
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-list.obj
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-list.csm
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-sim.obj
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-sim.csm
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-util.obj
	-del Release\praca\gammu\gammu\depend\dct3trac\wmx-util.csm
	-del Release\gammu.exe
	-del Release\gammu.map
	-del Release\gammu.ilc
	-del Release\gammu.ilf
	-del Release\gammu.ils
	-del Release\gammu.tds
	-del Release\gammu.ild

endif

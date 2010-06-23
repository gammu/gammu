
#
# Makefile for the GNOKII tool suite.
#

#
# For this common directory is used "subsystem.o" .o files concept.
#   (the list of object files to be linked together (to COMMON.o),
#   and other dirs is used this _one_ file (instead of all OBJS)
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

CFLAGS += $(PTHREAD_CFLAGS)

DIRS =		protocol \
		oldmodules \
		newmodules

OBJS =	devices/device.o \
	data/rlp-common.o \
	data/rlp-crc24.o \
	files/midifile.o \
	gsm-ringtones.o \
	gsm-coding.o \
	gsm-datetime.o \
	gsm-wap.o \
	gsm-api.o \
	gsm-phonebook.o \
	gsm-calendar.o \
	gsm-networks.o \
	gsm-bitmaps.o \
	gsm-sms.o \
	files/cfgreader.o \
	misc.o \
	protocol/fbus.o \
	protocol/fbus3110.o \
	protocol/fbusirda.o \
	protocol/mbus.o \
	protocol/at.o \
	newmodules/sniff/sniff.o \
	newmodules/newat.o \
	newmodules/n6110.o \
	newmodules/n3110.o \
	newmodules/n7110.o

DATA_OBJS = data/virtmodem.o \
	data/at-emulator.o \
	data/datapump.o

ifdef WIN32
	OBJS += $(TOPDIR)/win32/winserial.o
else
	OBJS += devices/unixserial.o \
	        devices/unixirda.o \
		devices/tekram.o
endif

ifdef XPM_CFLAGS
    CFLAGS += $(XPM_CFLAGS)
endif

all: COMMON.o DATA.o gsm-filetypes.o

COMMON.o: $(OBJS)
	$(LD) $(LDREL) $(LDOUT) COMMON.o $(OBJS)

DATA.o: $(DATA_OBJS)
	$(LD) $(LDREL) $(LDOUT) DATA.o $(DATA_OBJS)

gsm-filetypes.o: files/gsm-filetypes.c 
	$(CC) $(CFLAGS) -c files/gsm-filetypes.c

makelib: $(OBJS) DATA.o gsm-filetypes.o
	$(CC) -shared -o libmygnokii.so $(OBJS) DATA.o gsm-filetypes.o

clean:
	$(RM) $(OBJS) $(DATA_OBJS) *~ depend libmygnokii.so COMMON.o gsm-filetypes.o DATA.o
	
install:
	$(INSTALL) -d $(libdir)
	$(INSTALL) libmygnokii.so $(libdir)
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif


.PHONY: all install clean dep depend

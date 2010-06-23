

#
# Makefile for the xGNOKII tool suite.
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

CFLAGS += -DXGNOKIIDOCSDIR=\"$(docdir)\" \
	  $(PTHREAD_CFLAGS) \
	  $(GTK_CFLAGS) 

LDLIBS	+= $(PTHREAD_LIBS) \
	   $(GTK_LIBS) 

ifdef XPM_LIBS
    LDLIBS += $(XPM_LIBS)
endif

OBJS = 	xgnokii.o \
	xgnokii_common.o \
	xgnokii_contacts.o \
	xgnokii_sms.o \
	xgnokii_netmon.o \
	xgnokii_dtmf.o \
	xgnokii_cfg.o \
	xgnokii_speed.o \
	xgnokii_xkeyb.o \
	xgnokii_calendar.o \
	xgnokii_logos.o \
	xgnokii_xring.o \
	xgnokii_lowlevel.o \
	xgnokii_data.o

all: xgnokii

xgnokii: $(OBJS) $(TOPDIR)/common/COMMON.o $(TOPDIR)/common/DATA.o $(TOPDIR)/common/gsm-filetypes.o
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TOPDIR)/common/COMMON.o: 
	$(MAKE) -C $(TOPDIR)/common COMMON.o

$(TOPDIR)/common/DATA.o:
	$(MAKE) -C $(TOPDIR)/common DATA.o

$(TOPDIR)/common/gsm-filetypes.o: 
	$(MAKE) -C $(TOPDIR)/common gsm-filetypes.o

$(TOPDIR)/common/libmygnokii.so:
	$(MAKE) -C $(TOPDIR)/common makelib
	
xgnokiilib: $(OBJS) $(TOPDIR)/common/libmygnokii.so
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $(OBJS) -L$(TOPDIR)/common -lmygnokii $(LOADLIBES) $(LDLIBS) -o $@

makelib: xgnokiilib
	
clean:
	$(RM) $(OBJS) *~ xpm/*~ depend xgnokii xgnokiilib

install: all
	$(INSTALL) -d $(xbindir)
	$(INSTALL) xgnokii $(xbindir)
	$(INSTALL) xgnokiilib $(xbindir)/xgnokii
	$(INSTALL) -d $(xgnokii_libdir)/xpm

install-strip: all
	$(INSTALL) -d $(xbindir)
	$(INSTALL) -s xgnokii $(xbindir)
	$(INSTALL) -s xgnokiilib $(xbindir)/xgnokii
	$(INSTALL) -d $(xgnokii_libdir)/xpm
	@echo "done"

install-suid: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(xbindir)
	$(INSTALL) -o root -g gnokii -m 0750 xgnokii $(xbindir)
	$(INSTALL) -o root -g gnokii -m 0750 xgnokiilib $(xbindir)/xgnokii
	$(INSTALL) -o root -g gnokii -d -m 0750 $(xgnokii_libdir)/xpm
	@echo

install-ss: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -o root -g gnokii -d $(xbindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s xgnokii $(xbindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s xgnokiilib $(xbindir)/xgnokii
	$(INSTALL) -o root -g gnokii -d -m 0750 $(xgnokii_libdir)/xpm
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif


.PHONY: all install clean dep depend

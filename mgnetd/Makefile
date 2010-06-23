
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Janík ml.
#               2000 Karel Zak
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

CFLAGS	+= $(PTHREAD_CFLAGS)
LDFLAGS += $(PTHREAD_LIBS)

OBJS =	mgnetd.o

all: mgnetd

mgnetd: $(OBJS) $(TOPDIR)/common/COMMON.o

$(TOPDIR)/common/COMMON.o: 
	$(MAKE) -C $(TOPDIR)/common COMMON.o

$(TOPDIR)/common/libmygnokii.so:
	$(MAKE) -C $(TOPDIR)/common makelib
	
makelib: $(OBJS) $(TOPDIR)/common/libmygnokii.so
	$(CC) $(LDFLAGS) -L$(TOPDIR)/common -lmygnokii $(OBJS) -o mgnetdlib

clean:
	$(RM) $(OBJS) *~ depend mgnetd mgnetdlib *.exe core *.bak

install: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) mgnetd $(sbindir)
	$(INSTALL) mgnetdlib $(sbindir)/mgnetd

install-strip: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -s mgnetd $(sbindir)
	$(INSTALL) -s mgnetdlib $(sbindir)/mgnetd
	@echo "done"

install-suid: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 0750 mgnetd $(sbindir)
	$(INSTALL) -o root -g gnokii -m 0750 mgnetdlib $(sbindir)/mgnetd
	@echo

install-ss: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s mgnetd $(sbindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s mgnetdlib $(sbindir)/mgnetd
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif

.PHONY: all install clean dep depend

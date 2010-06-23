
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

OBJS =	gnokiid.o

all: gnokiid

gnokiid: $(OBJS) $(TOPDIR)/common/COMMON.o $(TOPDIR)/common/DATA.o

$(TOPDIR)/common/COMMON.o: 
	$(MAKE) -C $(TOPDIR)/common COMMON.o

$(TOPDIR)/common/DATA.o:
	$(MAKE) -C $(TOPDIR)/common DATA.o

$(TOPDIR)/common/libmygnokii.so:
	$(MAKE) -C $(TOPDIR)/common makelib

makelib: $(OBJS) $(TOPDIR)/common/libmygnokii.so
	$(CC) $(LDFLAGS) -L$(TOPDIR)/common -lmygnokii $(OBJS) -o gnokiidlib

clean:
	$(RM) $(OBJS) *~ depend gnokiid gnokiidlib *.exe core *.bak

install: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) gnokiid $(sbindir)
	$(INSTALL) gnokiidlib $(sbindir)/gnokiid

install-strip: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -s gnokiid $(sbindir)
	$(INSTALL) -s gnokiidlib $(sbindir)/gnokiid
	@echo "done"

install-suid: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 4750 gnokiid $(sbindir)
	$(INSTALL) -o root -g gnokii -m 4750 gnokiidlib $(sbindir)/gnokiid
	@echo "done"

install-ss: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 4750 -s gnokiid $(sbindir)
	$(INSTALL) -o root -g gnokii -m 4750 -s gnokiidlib $(sbindir)/gnokiid
	@echo "done"

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif

.PHONY: all install clean dep depend


#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Janík ml.
#               2000 Karel Zak
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

OBJS = mgnokiidev.o

all: mgnokiidev

mgnokiidev: $(OBJS) 

makelib: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o mgnokiidev
	
clean:
	$(RM) $(OBJS) *~ depend mgnokiidev *.exe core *.bak

install: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) mgnokiidev $(sbindir)
	$(INSTALL) todologo $(bindir)
	$(INSTALL) sendsms $(bindir)
	
install-strip: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -s mgnokiidev $(sbindir)
	$(INSTALL) todologo $(bindir)
	$(INSTALL) sendsms $(bindir)
	
install-suid: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 4750 mgnokiidev $(sbindir)
	$(INSTALL) todologo $(bindir)
	$(INSTALL) sendsms $(bindir)
	
install-ss: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 4750 -s mgnokiidev $(sbindir)
	$(INSTALL) todologo $(bindir)
	$(INSTALL) sendsms $(bindir)	
	
depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif


.PHONY: all install clean dep depend


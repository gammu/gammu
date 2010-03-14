
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Janík ml.
#               2000 Karel Zak
#

TOPDIR=../..
include $(TOPDIR)/Makefile.global

OBJS =	mg_demo_client.o \
        mg_net_api.o

all: mg_demo_client

mg_demo_client: $(OBJS)

makelib: mg_demo_client
	@echo

clean:
	$(RM) $(OBJS) *~ depend mg_demo_client *.exe core *.bak

install: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) mg_demo_client $(sbindir)

install-strip: all
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -s mg_demo_client $(sbindir)
	@echo "done"

install-suid: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 0750 mg_demo_client $(sbindir)
	@echo

install-ss: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(sbindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s mg_demo_client $(sbindir)
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif

.PHONY: all install clean dep depend

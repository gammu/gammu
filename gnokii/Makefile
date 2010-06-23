
#
# Makefile for the GNOKII tool suite.
#

TOPDIR=..
include $(TOPDIR)/Makefile.global


CFLAGS	+= $(PTHREAD_CFLAGS)
LDLIBS	+= $(PTHREAD_LIBS)

ifdef XPM_LIBS
    LDLIBS += $(XPM_LIBS)
endif

OBJS =	gnokii.o

ifdef OWN_GETOPT
    OBJS += $(TOPDIR)/getopt/libgetopt.a
endif

all: gnokii

gnokii: $(OBJS) $(TOPDIR)/common/COMMON.o $(TOPDIR)/common/gsm-filetypes.o
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TOPDIR)/common/COMMON.o: 
	$(MAKE) -C $(TOPDIR)/common COMMON.o

$(TOPDIR)/common/gsm-filetypes.o: 
	$(MAKE) -C $(TOPDIR)/common gsm-filetypes.o

$(TOPDIR)/getopt/libgetopt.a:
	$(MAKE) -C $(TOPDIR)/getopt

$(TOPDIR)/common/libmygnokii.so:
	$(MAKE) -C $(TOPDIR)/common makelib
	
gnokiilib: $(OBJS) $(TOPDIR)/common/libmygnokii.so
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $(OBJS) -L$(TOPDIR)/common -lmygnokii $(LOADLIBES) $(LDLIBS) -o $@

makelib: gnokiilib

clean:
	$(RM) $(OBJS) *~ depend gnokii gnokiilib *.exe core *.bak

install: all
	$(INSTALL) -d $(bindir)
	$(INSTALL) gnokii $(bindir)
	$(INSTALL) gnokiilib $(bindir)/gnokii

install-strip: all
	$(INSTALL) -d $(bindir)
	$(INSTALL) -s gnokii $(bindir)
	$(INSTALL) -s gnokiilib $(bindir)/gnokii
	@echo "done"

install-suid: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(bindir)
	$(INSTALL) -o root -g gnokii -m 0750 gnokii $(bindir)
	$(INSTALL) -o root -g gnokii -m 0750 gnokiilib $(bindir)/gnokii
	@echo

install-ss: all
	@if [ -z "/bin/grep -e '^gnokii:' /etc/group" ]; then \
		/usr/sbin/groupadd gnokii; \
	fi
	$(INSTALL) -d $(bindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s gnokii $(bindir)
	$(INSTALL) -o root -g gnokii -m 0750 -s gnokiilib $(bindir)/gnokii
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif

.PHONY: all install clean dep depend

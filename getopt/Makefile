TOPDIR=..
include $(TOPDIR)/Makefile.global

CFLAGS	+= $(PTHREAD_CFLAGS)
LDFLAGS =

SRCS = getopt.c getopt1.c
OBJS = getopt.o getopt1.o

all: libgetopt.a

libgetopt.a: $(OBJS)
	$(AR) cq $@ $(OBJS)

depend dep:
	$(CPP) -M $(CFLAGS) $(SRCS) > depend

distclean clean::
	$(RM) $(OBJS) *~ depend *.a

#####
# DO NOT DELETE THIS LINE -- make depend depends on it.
 
ifeq (depend,$(wildcard depend))
include depend
endif

.PHONY: all clean dep depend

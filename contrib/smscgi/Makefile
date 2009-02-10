# Note sometimes it happens that the pkg config files for gammu cannot be found
# in that case you can try the following to set the PKG_CONFIG_PATH.
#
# shell > export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
# 
# Or something equivalient. You have to install gammu anyway .. :) .
#

CFLAGS=$(shell pkg-config --cflags gammu) -Wall
LIBS=$(shell pkg-config --libs gammu)

ALL=smscgid
OBJS=$(foreach FILE, $(wildcard *.c), $(basename $(FILE)).o)

.PHONY: all clean

all: $(ALL)

clean: 
	rm -f $(ALL) $(OBJS)

smscgid:$(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

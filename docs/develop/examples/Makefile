# Path to directory, where is gammu built
GAMMU_BUILD=../../../build-configure/

CFLAGS=$(shell PKG_CONFIG_PATH=$(GAMMU_BUILD)/pkgconfig pkg-config --cflags --libs gammu-smsd) $(shell PKG_CONFIG_PATH=$(GAMMU_BUILD)/pkgconfig pkg-config --cflags --libs gammu) -Wall

ALL=phone-info sms-send smsd

.PHONY: all clean

all: $(ALL)

clean: 
	rm -f $(ALL)

%:%.c
	$(CC) $< $(CFLAGS) -o $@

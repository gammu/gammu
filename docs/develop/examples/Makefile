# Path to directory, where is gammu built
GAMMU_BUILD=../../../build/
#-configure/

CFLAGS=$(shell PKG_CONFIG_PATH=$(GAMMU_BUILD)/pkgconfig pkg-config --cflags --libs gammu) -Wall

all: phone-info

%:%.c
	$(CC) $< $(CFLAGS) -o $@

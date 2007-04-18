CC=i586-mingw32msvc-gcc
NM=i586-mingw32msvc-nm
DLLWRAP=i586-mingw32msvc-dllwrap
DLLTOOL=i586-mingw32msvc-dlltool
PYVER=25
PYDIR=/home/michal/.wine/drive_c/Python$(PYVER)
GAMMUDIR=/home/michal/work/gammu/gammu-fixes/build-mingw/
INCLUDES=-I$(PYDIR)/include -I./ -I../ -I$(GAMMUDIR)/../cfg -I$(GAMMUDIR)/../common
LDFLAGS=-L$(PYDIR)/libs -lGammu -L$(GAMMUDIR) -lws2_32 -lsetupapi -ladvapi32
LIBS=-lpython$(PYVER)
CFLAGS=-fno-strict-aliasing -DNDEBUG -g -O3 -Wall -Wstrict-prototypes $(INCLUDES)
LIB2DEF=./lib2def.py

LIBPY_DEF=$(PYDIR)/libs/libpython$(PYVER).def
LIBPY_A=$(PYDIR)/libs/libpython$(PYVER).a

MODULE=gammu
INPUT=\
        errors.o \
        misc.o \
        convertors/misc.o \
        convertors/string.o \
        convertors/time.o \
        convertors/base.o \
        convertors/sms.o \
        convertors/memory.o \
        convertors/todo.o \
        convertors/calendar.o \
        convertors/bitmap.o \
        convertors/ringtone.o \
        convertors/backup.o \
        convertors/file.o \
        convertors/call.o \
        convertors/wap.o \
        gammu.o
#OUTPUT=$(PYDIR)/Lib/site-packages/$(MODULE).pyd
OUTPUT=$(MODULE).pyd
MODULE_DEF=$(MODULE).def

all: $(OUTPUT)

$(LIBPY_DEF): $(PYDIR)/libs/python$(PYVER).lib
	NM=$(NM) PYVER=$(PYVER) $(LIB2DEF) $^ $@

$(LIBPY_A): $(LIBPY_DEF)
	$(DLLTOOL) --dllname python$(PYVER).dll --def $^ --output-lib $@

$(OUTPUT): $(INPUT) $(LIBPY_A) $(MODULE_DEF)
	$(DLLWRAP) --dllname $(MODULE).pyd --driver-name $(CC) --def $(MODULE_DEF) -o $@ $(INPUT) -s --entry _DllMain@12 --target=i586-mingw32 $(LDFLAGS) $(LIBS)

convertors/%.o: ../convertors/%.c pyg-config.h convertors
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../%.c pyg-config.h
	$(CC) $(CFLAGS) -c $< -o $@

convertors:
	mkdir convertors

pyg-config.h: ../setup.py
	../setup.py --dump-config > pyg-config.h

clean:
	rm -rf convertors
	rm -f pyg-config.h
	rm -f $(INPUT)
	rm -f $(OUTPUT)

# Python version
PYVER=25

# Paths to various needed libraries/tools
PYDIR=/home/michal/.wine/drive_c/Python$(PYVER)/
GAMMUDIR=/home/michal/work/gammu/gammu/build-mingw/
MINGWDIR=/usr/lib/gcc/i586-mingw32msvc/3.4.5/
GETTEXTDIR=/home/michal/work/win-cross/gettext/

# Program names
CC=i586-mingw32msvc-gcc
LD=i586-mingw32msvc-ld
NM=i586-mingw32msvc-nm
DLLWRAP=i586-mingw32msvc-dllwrap
DLLTOOL=i586-mingw32msvc-dlltool

INCLUDES=-I$(PYDIR)/include -I./ -I../ -I$(GAMMUDIR)/cfg -I$(GAMMUDIR)/../common
MINGWLD=-L$(MINGWDIR) -L$(MINGWDIR)/../../../../i586-mingw32msvc/lib
LDFLAGS=$(MINGWLD) -L$(PYDIR)/libs -L$(GAMMUDIR) -L$(GETTEXTDIR)/lib -lGammu -lws2_32 -lsetupapi -ladvapi32 -lintl -lmsvcrt -lkernel32 -lshell32 -lmoldname -lmingwex  -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -luser32 -lkernel32 -ladvapi32 -lshell32 -lmingw32  -lmoldname -lmingwex
LIBS=-lpython$(PYVER)
CFLAGS=-fno-strict-aliasing -DNDEBUG -g -O3 -Wall -Wstrict-prototypes $(INCLUDES)
LIB2DEF=./lib2def.py

LIBPY_DEF=$(PYDIR)/libs/libpython$(PYVER).def
LIBPY_A=$(PYDIR)/libs/libpython$(PYVER).a

MODULE=Core
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

$(OUTPUT): $(INPUT) $(LIBPY_A) $(MODULE_DEF) libmsvcrt.a
	@#$(DLLWRAP) -L --dllname $(MODULE).pyd --driver-name $(CC) --def $(MODULE_DEF) -o $@ $(INPUT) -s --entry _DllMain@12 --target=i586-mingw32 $(LDFLAGS) $(LIBS)
	$(LD) --dll   -o $@ $(MODULE_DEF) $(INPUT) -s --entry _DllMain@12 $(LDFLAGS) $(LIBS)

libmsvcrt.a: /usr/i586-mingw32msvc/lib/libmsvcr71.a
	cp /usr/i586-mingw32msvc/lib/libmsvcr71.a libmsvcrt.a

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

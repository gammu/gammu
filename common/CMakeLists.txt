# cmake <http://www.cmake.org> build file for Gammu
# Copyright © 2007 - 2009 Michal Čihař
# vim: expandtab sw=4 ts=4 sts=4:

project (libGammu C)

include(GammuTuneCompiler)

set (LIBRARY_SRC
    gsmcomon.c
    gsmphones.c
    gsmstate.c
    api.c
    debug.c
    misc/misc.c
    misc/cfg.c
    misc/coding/coding.c
    misc/coding/md5.c
    service/sms/gsmsms.c
    service/sms/gsmems.c
    service/sms/gsmmulti.c
    service/gsmcal.c
    service/gsmdata.c
    service/gsmpbk.c
    service/gsmring.c
    service/gsmlogo.c
    service/gsmmisc.c
    service/gsmnet.c
    service/backup/gsmback.c
    service/backup/backldif.c
    service/backup/backlmb.c
    service/backup/backtext.c
    service/backup/backvcs.c
    service/backup/backvcf.c
    service/backup/backics.c
    device/bluetoth/bluetoth.c
    device/irda/irda.c
    device/devfunc.c
    protocol/at/at.c
    protocol/alcatel/alcabus.c
    protocol/nokia/mbus2.c
    protocol/nokia/fbus2.c
    protocol/nokia/phonet.c
    protocol/obex/obex.c
    protocol/symbian/gnapbus.c
    phone/pfunc.c
    phone/at/atgen.c
    phone/at/siemens.c
    phone/at/samsung.c
    phone/at/motorola.c
    phone/alcatel/alcatel.c
    phone/atobex/atobex.c
    phone/nokia/dct3/n0650.c
    phone/nokia/dct3/n6110.c
    phone/nokia/dct3/n7110.c
    phone/nokia/dct3/n9210.c
    phone/nokia/dct3/dct3func.c
    phone/nokia/dct4s40/n3320.c
    phone/nokia/dct4s40/dct4func.c
    phone/nokia/dct4s40/6510/n6510.c
    phone/nokia/dct4s40/6510/6510cal.c
    phone/nokia/dct4s40/6510/6510file.c
    phone/nokia/wd2/n3650.c
    phone/nokia/nauto.c
    phone/nokia/nfunc.c
    phone/nokia/nfuncold.c
    phone/obex/obexgen.c
    phone/dummy/dummy.c
    phone/symbian/gnapgen.c)

if (BLUEZ_FOUND)
    list (APPEND LIBRARY_SRC device/bluetoth/bluez.c)
endif (BLUEZ_FOUND)
if (LIBUSB_FOUND)
    list (APPEND LIBRARY_SRC device/usb/usb.c)
endif (LIBUSB_FOUND)
if (FBSD_BLUE_FOUND)
    list (APPEND LIBRARY_SRC device/bluetoth/blue_fbsd.c)
endif (FBSD_BLUE_FOUND)
if (OSX_BLUE_FOUND)
    list (APPEND LIBRARY_SRC device/bluetoth/blue_osx.c)
endif (OSX_BLUE_FOUND)
if (WIN32 AND NOT CYGWIN)
    list (APPEND LIBRARY_SRC device/bluetoth/blue_w32.c)
endif (WIN32 AND NOT CYGWIN)

if (CYGWIN OR NOT WIN32 AND NOT DJGPP)
    list (APPEND LIBRARY_SRC device/serial/ser_unx.c)
endif (CYGWIN OR NOT WIN32 AND NOT DJGPP)
if (DJGPP)
    list (APPEND LIBRARY_SRC device/serial/ser_djg.c)
endif (DJGPP)
if (WIN32 AND NOT CYGWIN)
    list (APPEND LIBRARY_SRC device/serial/ser_w32.c)
endif (WIN32 AND NOT CYGWIN)

# Library
add_library (libGammu ${LIBRARY_SRC})
set_target_properties (libGammu PROPERTIES
    VERSION ${SOVERSION}.${VERSION}
    SOVERSION ${SOVERSION}
    OUTPUT_NAME Gammu)
if (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)
    set_target_properties (libGammu PROPERTIES COMPILE_FLAGS -fPIC)
endif (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)


if (BLUEZ_FOUND)
    target_link_libraries (libGammu ${BLUEZ_LIBRARIES})
    include_directories (${BLUEZ_INCLUDE_DIR})
endif (BLUEZ_FOUND)

if (LIBUSB_FOUND)
    target_link_libraries (libGammu ${LIBUSB_LIBRARIES})
    include_directories (${LIBUSB_INCLUDE_DIR})
endif (LIBUSB_FOUND)

if (FBSD_BLUE_FOUND)
    target_link_libraries (libGammu ${FBSD_BLUE_LIBRARIES})
    include_directories (${FBSD_BLUE_INCLUDE_DIR})
endif (FBSD_BLUE_FOUND)

if (OSX_BLUE_FOUND)
    target_link_libraries (libGammu ${OSX_BLUE_LIBS})
    include_directories (${OSX_BLUE_INCLUDE_DIR})
endif (OSX_BLUE_FOUND)

if (ICONV_FOUND)
    target_link_libraries (libGammu ${ICONV_LIBRARIES})
    include_directories (${ICONV_INCLUDE_DIR})
endif (ICONV_FOUND)

if (UNIX)
    find_library (MATH_LIBRARIES m)
    target_link_libraries (libGammu ${MATH_LIBRARIES})
endif (UNIX)

if (GETTEXTLIBS_FOUND)
    target_link_libraries (libGammu ${GETTEXT_LIBRARIES})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

# dirent emulation layer
if(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)
    target_link_libraries (win32dirent)
endif(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)

# Include search path
include_directories ("${Gammu_BINARY_DIR}/include")
include_directories ("${Gammu_SOURCE_DIR}/common")

# Add libraries needed for windows build
if (WIN32 AND NOT CYGWIN)
    # Magic needed for SHGetFolderPathA
    target_link_libraries (libGammu shfolder shell32)
    # Win sock
    target_link_libraries (libGammu ws2_32)
    # These libraries are needed for serial driver
    target_link_libraries (libGammu setupapi advapi32)
endif (WIN32 AND NOT CYGWIN)

# Install instructions
install (TARGETS libGammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT library
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        COMPONENT library
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        COMPONENT library
        )

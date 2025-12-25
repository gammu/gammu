# cmake <http://www.cmake.org> build file for Gammu
# Copyright © 2007 - 2009 Michal Čihař
# vim: expandtab sw=4 ts=4 sts=4:

project (libGammu C)

set (LIBRARY_SRC
    gsmcomon.c
    gsmphones.c
    gsmstate.c
    api.c
    debug.c
    misc/array.c
    misc/misc.c
    misc/cfg.c
    misc/string.c
    misc/coding/coding.c
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
    service/backup/backvnt.c
    device/bluetooth/bluetooth.c
    device/irda/irda.c
    device/usb/usb.c
    device/devfunc.c
    protocol/at/at.c
    protocol/alcatel/alcabus.c
    protocol/nokia/mbus2.c
    protocol/nokia/fbus2.c
    protocol/nokia/phonet.c
    protocol/obex/obex.c
    protocol/symbian/gnapbus.c
    protocol/s60/s60.c
    phone/pfunc.c
    phone/at/atgen.c
    phone/at/at-sms.c
    phone/at/siemens.c
    phone/at/samsung.c
    phone/at/motorola.c
    phone/at/sonyericsson.c
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
    phone/obex/mobex.c
    phone/symbian/gnapgen.c
    phone/s60/s60phone.c)

if (NOT HAVE_STRPTIME)
    list (APPEND LIBRARY_SRC misc/strptime.c)
endif (NOT HAVE_STRPTIME)

# dirent emulation layer
if(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)
    list (APPEND LIBRARY_SRC misc/win32-dirent.c)
endif(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)

# Own or glib based MD5 implementation
if (Glib_FOUND)
    list (APPEND LIBRARY_SRC misc/coding/md5-glib.c)
else (Glib_FOUND)
    list (APPEND LIBRARY_SRC misc/coding/md5.c)
endif (Glib_FOUND)

if (WITH_BACKUP)
    list (APPEND LIBRARY_SRC phone/dummy/dummy.c)
endif (WITH_BACKUP)
if (BLUEZ_FOUND)
    list (APPEND LIBRARY_SRC device/bluetooth/bluez.c)
endif (BLUEZ_FOUND)
if (BSD_BLUE_FOUND)
    list (APPEND LIBRARY_SRC device/bluetooth/blue_bsd.c)
endif (BSD_BLUE_FOUND)
if (OSX_BLUE_FOUND)
    list (APPEND LIBRARY_SRC device/bluetooth/blue_osx.c)
endif (OSX_BLUE_FOUND)
if (WIN32 AND NOT CYGWIN)
    list (APPEND LIBRARY_SRC device/bluetooth/blue_w32.c)
endif (WIN32 AND NOT CYGWIN)
if (NOT WIN32 AND HAVE_PTHREAD AND HAVE_SIGTIMEDWAIT)
    list (APPEND LIBRARY_SRC device/proxy/proxy.c)
endif (NOT WIN32 AND HAVE_PTHREAD AND HAVE_SIGTIMEDWAIT)

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
if(MSVC)
    add_library (libGammu STATIC ${LIBRARY_SRC})
else(MSVC)
    add_library (libGammu ${LIBRARY_SRC})
endif(MSVC)
add_coverage (libGammu)
set_target_properties (libGammu PROPERTIES
    VERSION ${GAMMU_SOVERSION}.${GAMMU_VERSION}
    SOVERSION ${GAMMU_SOVERSION}
    OUTPUT_NAME Gammu)
if(CMAKE_COMPILER_IS_MINGW AND BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES(libGammu PROPERTIES LINK_FLAGS
        "-Wl,--output-def,${CMAKE_CURRENT_BINARY_DIR}/libGammu.def")
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libGammu.def
            DESTINATION "${INSTALL_LIB_DIR}"
            COMPONENT library)
endif(CMAKE_COMPILER_IS_MINGW AND BUILD_SHARED_LIBS)


if (Glib_FOUND)
    target_link_libraries (libGammu ${Glib_LIBRARIES} ${GObject_LIBRARIES})
    include_directories(${Glib_INCLUDE_DIRS} ${GObject_INCLUDE_DIRS})
endif (Glib_FOUND)

if (HAVE_PTHREAD)
    target_link_libraries (libGammu ${CMAKE_THREAD_LIBS_INIT})
endif (HAVE_PTHREAD)

if (BLUEZ_FOUND)
    target_link_libraries (libGammu ${BLUEZ_LIBRARIES})
    include_directories (${BLUEZ_INCLUDE_DIR})
endif (BLUEZ_FOUND)

if (LIBUSB_FOUND)
    target_link_libraries (libGammu ${LIBUSB_LIBRARIES})
    include_directories (${LIBUSB_INCLUDE_DIR})
endif (LIBUSB_FOUND)

if (BSD_BLUE_FOUND)
    target_link_libraries (libGammu ${BSD_BLUE_LIBRARIES})
    include_directories (${BSD_BLUE_INCLUDE_DIR})
endif (BSD_BLUE_FOUND)

if (OSX_BLUE_FOUND)
    target_link_libraries (libGammu ${OSX_BLUE_LIBS})
    include_directories (${OSX_BLUE_INCLUDE_DIR})
endif (OSX_BLUE_FOUND)

if (ICONV_FOUND)
    target_link_libraries (libGammu ${ICONV_LIBRARIES})
    include_directories (${ICONV_INCLUDE_DIR})
endif (ICONV_FOUND)

if (UNIX)
    target_link_libraries (libGammu ${MATH_LIBRARIES})
endif (UNIX)

if (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
    target_link_libraries (libGammu ${LIBINTL_LIBRARIES})
    include_directories (${LIBINTL_INCLUDE_DIR})
endif (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)

# Include search path
include_directories ("${CMAKE_CURRENT_BINARY_DIR}/../include")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/../common")

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

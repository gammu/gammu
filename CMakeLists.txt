# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar

project (Gammu C)

# I didn't check older versions
cmake_minimum_required (VERSION 2.4.2)

# Read version
file (READ "${CMAKE_SOURCE_DIR}/version" VERSION_WIN)
string (REPLACE "\n" "" VERSION_WIN ${VERSION_WIN})
string (REPLACE "\r" "" VERSION_WIN ${VERSION_WIN})
string (REPLACE "," "." VERSION ${VERSION_WIN})
message ("Configuring ${CMAKE_PROJECT_NAME} ${VERSION}")

if (${Gammu_BINARY_DIR} STREQUAL ${Gammu_SOURCE_DIR})
    message (FATAL_ERROR "In tree build is not supported by Gammu, run cmake from different directory where binaries will be placed!")
endif (${Gammu_BINARY_DIR} STREQUAL ${Gammu_SOURCE_DIR})

# For debugging
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (CMAKE_VERBOSE_MAKEFILE ON)
	set (CFG_DEBUG 1)
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (CMAKE_VERBOSE_MAKEFILE OFF)
	set (CFG_DEBUG 0)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# FIXME: we should detect whether compiler supports -Wno-pointer-sign
set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wall -Wno-pointer-sign")
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-pointer-sign")

# No rpath by default
if (NOT DEFINED WITH_RPATH)
	set (WITH_RPATH FALSE CACHE BOOL "Enable rpath in resulting libraries")
	mark_as_advanced (WITH_RPATH)
endif (NOT DEFINED WITH_RPATH)

if (WITH_RPATH)
	set (CMAKE_SKIP_RPATH OFF)
else (WITH_RPATH)
	set (CMAKE_SKIP_RPATH ON)
endif (WITH_RPATH)

# Static library by default
if (NOT DEFINED ENABLE_SHARED)
	set (ENABLE_SHARED FALSE CACHE BOOL "Build shared library")
endif (NOT DEFINED ENABLE_SHARED)

if (ENABLE_SHARED)
	set (BUILD_SHARED_LIBS ON)
endif (ENABLE_SHARED)


# Where to lookup modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Standard packages
include (CheckCSourceCompiles)
include (CheckIncludeFile)
include (CheckFunctionExists)

# Packages in our sources (mostly taken from KDE)
include (MacroOptionalFindPackage)
include (MacroGammuOption)

# WE use pkgconfig later
find_package (PkgConfig)

# Standard packages
macro_optional_find_package (Doxygen)
macro_optional_find_package (Threads)

# Packages in sources

macro_optional_find_package (MySQL)
macro_optional_find_package (Postgres)
macro_optional_find_package (Bluez)

# Search for needed includes and functions

check_include_file (dirent.h HAVE_DIRENT_H)
check_include_file (sys/ioctl.h HAVE_SYS_IOCTL_H)
check_function_exists (scandir HAVE_SCANDIR)
check_function_exists (alphasort HAVE_ALPHASORT)
check_function_exists (iswspace HAVE_ISWSPACE)
check_function_exists (towlower HAVE_TOWLOWER)

set (LOCALES 
    cs 
    de 
    es 
    it 
    pl 
    ru 
    us
    )

set (EXAMPLES
    docs/examples/config/smsdrc
    docs/examples/config/gammurc
    docs/examples/config/mysql.sql
    docs/examples/config/pgsql.sql
    docs/examples/fax/faxreceive
    docs/examples/fax/faxsend
    docs/examples/media/aliens.nlm
    docs/examples/media/axelf.txt
    docs/examples/ppp/data
    docs/examples/ppp/gprs
    docs/examples/ppp/startppp
    docs/examples/ppp/statsppp
    other/php/php1/admin.php
    other/php/php2/linked.php
    other/php/php2/linked.sql
    other/php/php3/config.php
    other/php/php3/funcoes/func.gammu.php
    other/php/php3/funcoes/func.sql.php
    other/php/php3/index.php
    other/php/php3/intergammu.txt
    other/php/php3/proc/admin.php
    other/php/php3/proclast.sql
    other/php/php4/sms.php
    )

set (DEVELDOCS
    docs/docs/english/replace.txt
    docs/docs/english/readme.htm
    docs/docs/english/gammu.htm
    docs/docs/gammu.png
    docs/docs/italian/gammu.txt
    docs/docs/italian/readme.txt
    )

set (DOCS
    README 
    ChangeLog 
    COPYING
    )

set (SYMBIAN_FILES
    other/symbian/readme.txt
    other/symbian/gnapplet.ini
    other/symbian/gnapplet.sis
    )

set (MAN1_PAGES
    docs/docs/english/gammu.1
    )

set (HEADERS
    common/device/bluetoth/affix.h
    common/device/bluetoth/blue_w32.h
    common/device/bluetoth/bluetoth.h
    common/device/bluetoth/bluez.h
    common/device/devfunc.h
    common/device/irda/irda.h
    common/device/irda/irda_unx.h
    common/device/irda/irda_w32.h
    common/device/serial/ser_djg.h
    common/device/serial/ser_unx.h
    common/device/serial/ser_w32.h
    common/gammu.h
    common/misc/coding/md5.h
    common/misc/coding/coding.h
    common/misc/cfg.h
    common/misc/misc.h
    common/phone/alcatel/alcatel.h
    common/phone/at/samsung.h
    common/phone/at/siemens.h
    common/phone/at/atfunc.h
    common/phone/at/atgen.h
    common/phone/nokia/dct3/dct3comm.h
    common/phone/nokia/dct3/dct3func.h
    common/phone/nokia/dct3/n0650.h
    common/phone/nokia/dct3/n6110.h
    common/phone/nokia/dct3/n7110.h
    common/phone/nokia/dct3/n9210.h
    common/phone/nokia/nfuncold.h
    common/phone/nokia/ncommon.h
    common/phone/nokia/nfunc.h
    common/phone/nokia/wd2/n3650.h
    common/phone/nokia/dct4s40/6510/6510cal.h
    common/phone/nokia/dct4s40/6510/6510file.h
    common/phone/nokia/dct4s40/6510/n6510.h
    common/phone/nokia/dct4s40/dct4func.h
    common/phone/nokia/dct4s40/n3320.h
    common/phone/obex/obexgen.h
    common/phone/obex/obexfunc.h
    common/phone/pfunc.h
    common/phone/symbian/gnapgen.h
    common/phone/sonyeric/sefunc.h
    common/phone/sonyeric/sonyeric.h
    common/protocol/alcatel/alcabus.h
    common/protocol/at/at.h
    common/protocol/nokia/fbus2.h
    common/protocol/nokia/mbus2.h
    common/protocol/nokia/phonet.h
    common/protocol/obex/obex.h
    common/protocol/protocol.h
    common/protocol/symbian/gnapbus.h
    common/service/backup/backics.h
    common/service/backup/backldif.h
    common/service/backup/backlmb.h
    common/service/backup/backtext.h
    common/service/backup/backvcf.h
    common/service/backup/backvcs.h
    common/service/backup/gsmback.h
    common/service/backup/backgen.h
    common/service/gsmnet.h
    common/service/gsmprof.h
    common/service/sms/gsmems.h
    common/service/sms/gsmmulti.h
    common/service/sms/gsmsms.h
    common/service/gsmlogo.h
    common/service/gsmcal.h
    common/service/gsmring.h
    common/service/gsmpbk.h
    common/service/gsmcall.h
    common/service/gsmdata.h
    common/service/gsmmisc.h
    common/gsmstate.h
    common/gsmcomon.h
    common/config.h
    )

set (LIBRARY_SRC
	common/gsmcomon.c
	common/gsmstate.c
	common/misc/misc.c
	common/misc/cfg.c
	common/misc/coding/coding.c
	common/misc/coding/md5.c
	common/service/sms/gsmsms.c
	common/service/sms/gsmems.c
	common/service/sms/gsmmulti.c
	common/service/gsmcal.c
	common/service/gsmdata.c
	common/service/gsmpbk.c
	common/service/gsmring.c
	common/service/gsmlogo.c
	common/service/gsmmisc.c
	common/service/gsmnet.c
	common/service/backup/gsmback.c
	common/service/backup/backldif.c
	common/service/backup/backlmb.c
	common/service/backup/backtext.c
	common/service/backup/backvcs.c
	common/service/backup/backvcf.c
	common/service/backup/backics.c
	common/device/bluetoth/affix.c
	common/device/bluetoth/bluez.c
	common/device/bluetoth/blue_w32.c
	common/device/bluetoth/bluetoth.c
	common/device/serial/ser_unx.c
	common/device/serial/ser_djg.c
	common/device/irda/irda.c
	common/device/devfunc.c
	common/protocol/at/at.c
	common/protocol/alcatel/alcabus.c
	common/protocol/nokia/mbus2.c
	common/protocol/nokia/fbus2.c
	common/protocol/nokia/phonet.c
	common/protocol/obex/obex.c
	common/protocol/symbian/gnapbus.c
	common/phone/pfunc.c
	common/phone/at/atgen.c
	common/phone/at/siemens.c
	common/phone/at/samsung.c
	common/phone/alcatel/alcatel.c
	common/phone/sonyeric/sonyeric.c
	common/phone/nokia/dct3/n0650.c
	common/phone/nokia/dct3/n6110.c
	common/phone/nokia/dct3/n7110.c
	common/phone/nokia/dct3/n9210.c
	common/phone/nokia/dct3/dct3func.c
	common/phone/nokia/dct4s40/n3320.c
	common/phone/nokia/dct4s40/dct4func.c
	common/phone/nokia/dct4s40/6510/n6510.c
	common/phone/nokia/dct4s40/6510/6510cal.c
	common/phone/nokia/dct4s40/6510/6510file.c
	common/phone/nokia/wd2/n3650.c
	common/phone/nokia/nauto.c
	common/phone/nokia/nfunc.c
	common/phone/nokia/nfuncold.c
	common/phone/obex/obexgen.c
	common/phone/symbian/gnapgen.c)

set (BINARY_SRC
	gammu/depend/nokia/dct3.c
	gammu/depend/nokia/dct3trac/wmx.c
	gammu/depend/nokia/dct3trac/wmx-gsm.c
	gammu/depend/nokia/dct3trac/wmx-list.c
	gammu/depend/nokia/dct3trac/wmx-sim.c
	gammu/depend/nokia/dct3trac/wmx-util.c
	gammu/depend/nokia/dct4.c
	gammu/depend/siemens/dsiemens.c
	gammu/depend/siemens/chiffre.c
	gammu/smsd/smsdcore.c
	gammu/smsd/s_files.c
	gammu/smsd/s_mysql.c
	gammu/smsd/s_pgsql.c
	gammu/sniff.c
	gammu/gammu.c)

# Library
add_library (Gammu ${LIBRARY_SRC})
set_target_properties (Gammu PROPERTIES VERSION ${VERSION} SOVERSION 1)

# Gammu program
add_executable (gammu ${BINARY_SRC})


target_link_libraries (gammu Gammu)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

# Tune options for found libraries
if (MYSQL_FOUND)
	target_link_libraries (gammu ${MYSQL_LIBRARIES})
    set (HAVE_MYSQL_MYSQL_H TRUE)
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
	target_link_libraries (gammu ${POSTGRES_LIBRARY})
    set (HAVE_POSTGRESQL_LIBPQ_FE_H TRUE)
endif (POSTGRES_FOUND)

if (BLUEZ_FOUND)
	target_link_libraries (Gammu ${BLUEZ_LIBRARIES})
    set (GSM_ENABLE_BLUEZ TRUE)
    macro_gammu_option (BLUETOOTH_RF_SEARCHING "Searching for RF channels with Bluetooth stack" ON)
    if (WITH_BLUETOOTH_RF_SEARCHING)
        set (BLUETOOTH_RF_SEARCHING ON)
    endif (WITH_BLUETOOTH_RF_SEARCHING)
endif (BLUEZ_FOUND)

if (CMAKE_USE_PTHREADS_INIT)
    set (HAVE_PTHREAD ON)
endif (CMAKE_USE_PTHREADS_INIT)

# Phone and protocol configuration 
macro_gammu_option (NOKIA_SUPPORT "Nokia phones support" ON)
if (WITH_NOKIA_SUPPORT)
    macro_gammu_option (MBUS2 "Nokia MBUS2 protocol" ON)
    macro_gammu_option (FBUS2 "Nokia FBUS2 protocol" ON)
    macro_gammu_option (FBUS2DLR3 "Nokia FBUS2DLR3 protocol" ON)
    macro_gammu_option (DKU2PHONET "Nokia DKU2PHONET protocol" ON)
    macro_gammu_option (DKU2AT "Nokia DKU2AT protocol" ON)
    macro_gammu_option (DKU5FBUS2 "Nokia DKU5FBUS2 protocol" ON)
    macro_gammu_option (FBUS2PL2303 "Nokia FBUS2PL2303 protocol" ON)

    macro_gammu_option (FBUS2BLUE "Nokia FBUS2BLUE protocol" ON)
    macro_gammu_option (PHONETBLUE "Nokia PHONETBLUE protocol" ON)
    macro_gammu_option (BLUEFBUS2 "Nokia BLUEFBUS2 protocol" ON)
    macro_gammu_option (BLUEPHONET "Nokia BLUEPHONET protocol" ON)

    macro_gammu_option (IRDAPHONET "Nokia IRDAPHONET protocol" ON)
    macro_gammu_option (FBUS2IRDA "Nokia FBUS2IRDA protocol" ON)

    macro_gammu_option (NOKIA3320 "Nokia 3320 and compatible phones support" ON)
    macro_gammu_option (NOKIA650 "Nokia 650 and compatible phones support" ON)
    macro_gammu_option (NOKIA6110 "Nokia 61xx and compatible phones support" ON)
    macro_gammu_option (NOKIA6510 "Nokia 6510 and compatible phones support" ON)
    if (WITH_NOKIA6510)
        macro_gammu_option (DCT4_CALENDAR_6210 "Force using 6210 frames for calendar for DCT4 phones" OFF)
        if (WITH_DCT4_CALENDAR_6210)
            set (GSM_FORCE_DCT4_CALENDAR_6210 TRUE)
        endif (WITH_DCT4_CALENDAR_6210)
    endif (WITH_NOKIA6510)
    macro_gammu_option (NOKIA7110 "Nokia 7110 and compatible phones support" ON)
    macro_gammu_option (NOKIA9210 "Nokia 9210 and compatible phones support" ON)
    if (WITH_NOKIA7110 OR WITH_NOKIA9210)
        macro_gammu_option (N71_92INCOMINGINFO "Nokia 62xx/71xx/9xxx incoming call/SMS info" OFF)
    endif (WITH_NOKIA7110 OR WITH_NOKIA9210)
    macro_gammu_option (NOKIA3650 "Nokia 3650 and compatible phones support" ON)
endif (WITH_NOKIA_SUPPORT)

macro_gammu_option (AT_SUPPORT "AT compatible phones support" ON)
if (WITH_AT_SUPPORT)
    macro_gammu_option (AT "AT protocol" ON)
    macro_gammu_option (BLUEAT "AT protocol over Bluetooth" ON)
    macro_gammu_option (IRDAAT "AT protocol over IrDA" ON)

    macro_gammu_option (ATGEN "AT phones support" ON)

    macro_gammu_option (ALCATEL_SUPPORT "Alcatel phones support" ON)
    if (WITH_ALCATEL_SUPPORT)
        macro_gammu_option (ALCABUS "Alcatel protocol" ON)

        macro_gammu_option (ALCATEL "Alcatel phones support" ON)
    endif (WITH_ALCATEL_SUPPORT)
endif (WITH_AT_SUPPORT)

macro_gammu_option (OBEX_SUPPORT "OBEX compatible phones support" ON)
if (WITH_OBEX_SUPPORT)
    macro_gammu_option (IRDAOBEX "OBEX protocol over IrDA" ON)
    macro_gammu_option (BLUEOBEX "OBEX protocol over Bluetooth" ON)

    macro_gammu_option (OBEXGEN "Generic OBEX phones support" ON)
    if (WITH_AT_SUPPORT)
        macro_gammu_option (SONYERICSSON "Sony-Ericsson phones support" ON)
    endif (WITH_AT_SUPPORT)
endif (WITH_OBEX_SUPPORT)

macro_gammu_option (GNAPPLET_SUPPORT "Symbian phones support (gnapplet)" ON)
if (WITH_GNAPPLET_SUPPORT)
    macro_gammu_option (IRDAGNAPBUS "GNAPBUS protocol over IrDA" ON)
    macro_gammu_option (BLUEGNAPBUS "GNAPBUS protocol over Bluetooth" ON)

    macro_gammu_option (GNAPGEN "Gnapplet phones support" ON)
endif (WITH_GNAPPLET_SUPPORT)

option (ENABLE_LGPL "Enable LGPL licensing (disables all GPL only code)" OFF)
mark_as_advanced (ENABLE_LGPL)


# Some generic configurations
macro_gammu_option (CELLBROADCAST "Cell Broadcast messages support" ON)
macro_gammu_option (BACKUP "Backup/Restore functions" ON)


# Generate config.h
configure_file (cfg/config.h.cmake cfg/config.h ESCAPE_QUOTES)

# Include search path
include_directories (
    ${Gammu_BINARY_DIR}/cfg
	${Gammu_SOURCE_DIR}/common
	${POSTGRES_INCLUDE_DIR}
	${MYSQL_INCLUDE_DIR}
	${BLUEZ_INCLUDE_DIR}
	)

# Generate Doxygen file
set (DOXYGEN_INPUT ${Gammu_SOURCE_DIR}/common)
set (DOXYGEN_OUTPUT ${Gammu_BINARY_DIR}/gammu-doc)
if (${DOXYGEN_DOT_EXECUTABLE} STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "NO")
    set (DOXYGEN_DOT_PATH "")
else (${DOXYGEN_DOT_EXECUTABLE} STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "YES")
    # Strip binary name from variable
    string (REGEX REPLACE "/dot$" "" DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE})
endif (${DOXYGEN_DOT_EXECUTABLE} STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
configure_file (cfg/Doxyfile.cmake cfg/Doxyfile)

# Target for generating API documentation
add_custom_target (apidoc ${DOXYGEN_EXECUTABLE} cfg/Doxyfile 
    DEPENDS ${LIBRARY_SRC} ${HEADERS}
    COMMENT "Generating API documentation")

# Install paths
if (NOT DEFINED INSTALL_BIN_DIR)
	set (INSTALL_BIN_DIR "bin" CACHE FILEPATH "Path for binaries installation")
    mark_as_advanced (INSTALL_BIN_DIR)
endif (NOT DEFINED INSTALL_BIN_DIR)
if (NOT DEFINED INSTALL_LIB_DIR)
	set (INSTALL_LIB_DIR "lib" CACHE FILEPATH "Path for libraries installation")
    mark_as_advanced (INSTALL_LIB_DIR)
endif (NOT DEFINED INSTALL_LIB_DIR)
if (NOT DEFINED INSTALL_INC_DIR)
	set (INSTALL_INC_DIR "include/gammu" CACHE FILEPATH "Path for includes installation")
    mark_as_advanced (INSTALL_INC_DIR)
endif (NOT DEFINED INSTALL_INC_DIR)
if (NOT DEFINED INSTALL_LOC_DIR)
	set (INSTALL_LOC_DIR "share/gammu" CACHE FILEPATH "Path for locales installation")
    mark_as_advanced (INSTALL_LOC_DIR)
endif (NOT DEFINED INSTALL_LOC_DIR)
if (NOT DEFINED INSTALL_DOC_DIR)
	set (INSTALL_DOC_DIR "share/doc/gammu" CACHE FILEPATH "Path for documentation installation")
    mark_as_advanced (INSTALL_DOC_DIR)
endif (NOT DEFINED INSTALL_DOC_DIR)
if (NOT DEFINED INSTALL_MAN_DIR)
	set (INSTALL_MAN_DIR "share/man" CACHE FILEPATH "Path for man pages installation")
    mark_as_advanced (INSTALL_MAN_DIR)
endif (NOT DEFINED INSTALL_MAN_DIR)

# Generate pkgconfig file
configure_file (cfg/gammu.pc.cmake cfg/gammu.pc @ONLY)

# Install instructions
install (TARGETS gammu Gammu
        RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT "runtime"
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT "runtime"
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT "development"
        )

foreach (HEADER ${HEADERS})
    string (REGEX REPLACE "common(.*)/[^/]*$" "\\1" DIRNAME ${HEADER})
    install (
        FILES ${HEADER} 
        DESTINATION "${INSTALL_INC_DIR}/gammu/${DIRNAME}" 
        COMPONENT "development"
        )
endforeach (HEADER)

foreach (LOCALE ${LOCALES})
    install (
        FILES "docs/docs/locale/gammu_${LOCALE}.txt" 
        DESTINATION "${INSTALL_LOC_DIR}/gammu_${LOCALE}.txt"
        COMPONENT "locales"
        )
endforeach (LOCALE)

install (
    FILES "${Gammu_BINARY_DIR}/cfg/gammu.pc"
    DESTINATION "lib/pkgconfig"
    COMPONENT "development"
    )

install (
    FILES ${DOCS}
    DESTINATION ${INSTALL_DOC_DIR}
    COMPONENT "basic-documentation"
    )

install (
    FILES ${SYMBIAN_FILES}
    DESTINATION ${INSTALL_DOC_DIR}/symbian
    COMPONENT "gnapplet"
    )

foreach (DOC ${DEVELDOCS})
    string (REGEX REPLACE "docs/docs(.*)/[^/]*$" "\\1" DIRNAME ${DOC})
    install (
        FILES ${DOC}
        DESTINATION "${INSTALL_DOC_DIR}/devel/${DIRNAME}"
        COMPONENT "developer-documentation"
        )
endforeach (DOC)

foreach (EXAMPLE ${EXAMPLES})
    string (REGEX REPLACE "(docs/examples|other/php)(.*)/[^/]*$" "\\2" DIRNAME ${EXAMPLE})
    install (
        FILES ${EXAMPLE}
        DESTINATION "${INSTALL_DOC_DIR}/examples/${DIRNAME}"
        COMPONENT "developer-documentation"
        )
endforeach (EXAMPLE)

install (
    FILES ${MAN1_PAGES}
    DESTINATION "${INSTALL_MAN_DIR}/man1"
    COMPONENT "basic-documentation"
    )

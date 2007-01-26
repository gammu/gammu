# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar

project (Gammu C)

# I didn't check older versions
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.2)

# Read version
FILE(READ "${CMAKE_SOURCE_DIR}/version" VERSION_WIN)
string(REPLACE "\n" "" VERSION_WIN ${VERSION_WIN})
string(REPLACE "\r" "" VERSION_WIN ${VERSION_WIN})
string(REPLACE "," "." VERSION ${VERSION_WIN})
message ("Configuring ${CMAKE_PROJECT_NAME} ${VERSION}")

# For debugging
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (CMAKE_VERBOSE_MAKEFILE ON)
	set (CFG_DEBUG 1)
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (CMAKE_VERBOSE_MAKEFILE OFF)
	set (CFG_DEBUG 0)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# FIXME: we should detect whether compiler supports -Wno-pointer-sign
SET ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wall -Wno-pointer-sign")
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-pointer-sign")

# No rpath by default
if (NOT DEFINED WITH_RPATH)
	set (WITH_RPATH FALSE CACHE BOOL "Enable rpath in resulting libraries")
	mark_as_advanced(WITH_RPATH)
endif (NOT DEFINED WITH_RPATH)

if (WITH_RPATH)
	set(CMAKE_SKIP_RPATH OFF)
else (WITH_RPATH)
	set(CMAKE_SKIP_RPATH ON)
endif (WITH_RPATH)

# Where to lookup modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Standard packages
include (CheckCSourceCompiles)

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
macro_optional_find_package (Bluez)

include_directories (
	${GAMMU_SOURCE_DIR}/common
	${MYSQL_INCLUDE_DIR}
	${BLUEZ_INCLUDE_DIR}
	)

link_directories (
	${GAMMU_SOURCE_DIR}/common
	)

add_library (Gammu 
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

add_executable (gammu 
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
	gammu/sniff.c
	gammu/gammu.c)


target_link_libraries (gammu Gammu)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

if (MYSQL_FOUND)
	target_link_libraries (gammu ${MYSQL_LIBRARIES})
endif (MYSQL_FOUND)

if (BLUEZ_FOUND)
	target_link_libraries (Gammu ${BLUEZ_LIBRARIES})
endif (BLUEZ_FOUND)

macro_gammu_option(NOKIA "Nokia phones support" ON)
if (WITH_NOKIA)
    macro_gammu_option(MBUS2 "Nokia MBUS2 protocol" ON)
    macro_gammu_option(FBUS2 "Nokia FBUS2 protocol" ON)
    macro_gammu_option(FBUS2DLR3 "Nokia FBUS2DLR3 protocol" ON)
    macro_gammu_option(DKU2PHONET "Nokia DKU2PHONET protocol" ON)
    macro_gammu_option(DKU2AT "Nokia DKU2AT protocol" ON)
    macro_gammu_option(DKU5FBUS2 "Nokia DKU5FBUS2 protocol" ON)
    macro_gammu_option(FBUS2PL2303 "Nokia FBUS2PL2303 protocol" ON)
    macro_gammu_option(FBUS2BLUE "Nokia FBUS2BLUE protocol" ON)
    macro_gammu_option(FBUS2IRDA "Nokia FBUS2IRDA protocol" ON)
    macro_gammu_option(PHONETBLUE "Nokia PHONETBLUE protocol" ON)
endif (GSM_ENALE_NOKIA)

macro_gammu_option(AT "AT protocol" ON)

CONFIGURE_FILE(cfg/cmake/config.h.in cfg/config.h)


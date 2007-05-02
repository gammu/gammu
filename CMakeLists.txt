# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu C)

# I didn't check older versions
cmake_minimum_required (VERSION 2.4.2)

# Where to lookup modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Support for cross compilation
if (UNIX)
    option (CROSS_MINGW "Cross compile using MinGW on linux" OFF)
else (UNIX)
    set (CROSS_MINGW OFF)
endif (UNIX)
if (CROSS_MINGW)
    message ("Cross compilign using MinGW")
    set (CMAKE_CXX_COMPILER "i586-mingw32msvc-c++")
    set (CMAKE_COMPILER_IS_GNUCXX 1)
    set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wall -Werror-implicit-function-declaration")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror-implicit-function-declaration")
    set (CMAKE_C_COMPILER "i586-mingw32msvc-gcc")
    set (CMAKE_AR "i586-mingw32msvc-ar")
    set (CMAKE_RANLIB "i586-mingw32msvc-ranlib")
    set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "l")
    set (WIN32 ON)
    set (MINGW ON)

    include (WindowsCross)

    # Detection not supported for cross compiling
    set (HAVE_ISWSPACE ON)
    set (HAVE_TOWLOWER ON)

    # CPack configuration
    set (CPACK_GENERATOR "ZIP;NSIS")
    set (CPACK_SOURCE_GENERATOR "ZIP")
    set (CPACK_TOPLEVEL_TAG "Source")
    set (CPACK_SYSTEM_NAME "Windows")
else (CROSS_MINGW)
# FIXME: we should detect whether compiler supports -Wno-pointer-sign
    set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wall -Wno-pointer-sign -Werror-implicit-function-declaration")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-pointer-sign -Werror-implicit-function-declaration")
endif (CROSS_MINGW)

# Read version
file (READ "${CMAKE_SOURCE_DIR}/VERSION" VERSION_WIN)
string (REPLACE "\n" "" VERSION_WIN ${VERSION_WIN})
string (REPLACE "\r" "" VERSION_WIN ${VERSION_WIN})
string (REPLACE "," "." VERSION ${VERSION_WIN})
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\1" VERSION_MAJOR ${VERSION})
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\2" VERSION_MINOR ${VERSION})
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\3" VERSION_PATCH ${VERSION})
set (VERSION_WIN "${VERSION_WIN},0")
message ("Configuring ${CMAKE_PROJECT_NAME} ${VERSION}")

if ("${Gammu_BINARY_DIR}" STREQUAL "${Gammu_SOURCE_DIR}")
    message (FATAL_ERROR "In tree build is not supported by Gammu, run cmake from different directory where binaries will be placed!")
endif ("${Gammu_BINARY_DIR}" STREQUAL "${Gammu_SOURCE_DIR}")

# For debugging
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_VERBOSE_MAKEFILE ON)
    set (DEBUG 1)
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_VERBOSE_MAKEFILE OFF)
    set (DEBUG 0)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

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


# Packaging support
set (CPACK_PACKAGE_NAME "Gammu")
set (CPACK_PACKAGE_VERSION "${VERSION}")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gammu ${VERSION}")
set (CPACK_PACKAGE_VENDOR "Michal Čihař, Macin Wiacek and others")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
if (WIN32)
set (CPACK_PACKAGE_INSTALL_DIRECTORY "Gammu ${VERSION}")
endif (WIN32)
if (WIN32)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #set (CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  #set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set (CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Gammu")
  set (CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.gammu.org")
  set (CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.gammu.org")
  set (CPACK_NSIS_CONTACT "michal@cihar.com")
  set (CPACK_NSIS_MODIFY_PATH ON)
else (WIN32)
  set (CPACK_STRIP_FILES "gammu")
  set (CPACK_SOURCE_STRIP_FILES "")
endif (WIN32)
include (CPack)

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

# Packages in sources
macro_optional_find_package (Gettext)

# Check disabled for cross, but we have some libraries
if (CROSS_MINGW)

    set (MYSQL_CROSS_PATH "${Gammu_SOURCE_DIR}/../../win-cross/mysql" CACHE FILEPATH "Path to MySQL for cross compilation")
    find_library (MYSQL_LIBRARIES 
        mysql "${MYSQL_CROSS_PATH}/lib/opt" 
        NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
    if (MYSQL_LIBRARIES) 
        set (MYSQL_FOUND ON)
        set (MYSQL_INCLUDE_DIR "${MYSQL_CROSS_PATH}/include")
        message ("Cross MySQL found, libs: ${MYSQL_LIBRARIES}, incs: ${MYSQL_INCLUDE_DIR}")
    else (MYSQL_LIBRARIES) 
        message ("Cross MySQL not found")
    endif (MYSQL_LIBRARIES) 

    set (POSTGRES_CROSS_PATH "${Gammu_SOURCE_DIR}/../../win-cross/pgsql" CACHE FILEPATH "Path to PostgreSQL for cross compilation")
    find_library (POSTGRES_LIBRARY 
        pq "${POSTGRES_CROSS_PATH}/lib"
        NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
    if (POSTGRES_LIBRARY) 
        set (POSTGRES_FOUND ON)
        set (POSTGRES_INCLUDE_DIR "${POSTGRES_CROSS_PATH}/include")
        message ("Cross PostgreSQL found, libs: ${POSTGRES_LIBRARY}, incs: ${POSTGRES_INCLUDE_DIR}")
    else (POSTGRES_LIBRARY) 
        message ("Cross PostgreSQL not found")
    endif (POSTGRES_LIBRARY) 

    set (GETTEXT_CROSS_PATH "${Gammu_SOURCE_DIR}/../../win-cross/gettext" CACHE FILEPATH "Path to Gettext for cross compilation")
    find_library (GETTEXT_LIBRARY 
        intl "${GETTEXT_CROSS_PATH}/lib"
        NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
    if (GETTEXT_LIBRARY) 
        set (GETTEXTLIBS_FOUND ON)
        set (GETTEXT_INCLUDE_DIR "${GETTEXT_CROSS_PATH}/include")
        message ("Cross Gettext found, libs: ${GETTEXT_LIBRARY}, incs: ${GETTEXT_INCLUDE_DIR}")
    else (GETTEXT_LIBRARY) 
        message ("Cross Gettext not found")
    endif (GETTEXT_LIBRARY) 

else (CROSS_MINGW)
    # Standard packages
    macro_optional_find_package (Threads)

    # Packages in sources
    macro_optional_find_package (MySQL)
    macro_optional_find_package (Postgres)
    macro_optional_find_package (GettextLibs)

    # Search for needed includes and functions
    check_include_file (dirent.h HAVE_DIRENT_H)
    check_include_file (sys/ioctl.h HAVE_SYS_IOCTL_H)
    check_include_file (sys/utsname.h HAVE_SYS_UTSNAME_H)
    check_function_exists (scandir HAVE_SCANDIR)
    check_function_exists (alphasort HAVE_ALPHASORT)
    check_function_exists (iswspace HAVE_ISWSPACE)
    check_function_exists (towlower HAVE_TOWLOWER)
    check_function_exists (strncasecmp HAVE_STRNCASECMP)
    check_function_exists (strcasecmp HAVE_STRCASECMP)
    check_function_exists (strcasestr HAVE_STRCASESTR)
    check_function_exists (strchrnul HAVE_STRCHRNUL)
    check_function_exists (sin HAVE_SIN)

endif (CROSS_MINGW)

macro_optional_find_package (Bluez)

OPTION(WITH_IrDA "Search for IrDA package" ON)
if (WITH_IrDA)
    if (NOT CROSS_MINGW)
        check_c_source_compiles (
            "
#include <sys/socket.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/irda.h>
    int main(int argc, char **argv) { 
        return 0; 
    }
            " 
            IRDA_FOUND
            )
    else (NOT CROSS_MINGW)
        set (IRDA_FOUND YES)
    endif (NOT CROSS_MINGW)
else (WITH_IrDA)
    set (IRDA_FOUND)
endif (WITH_IrDA)

set (LOCALES 
    cs 
    de 
    es 
    it 
    pl 
    ru 
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
    docs/develop/Gammu.htm
    docs/develop/develop.txt
    docs/develop/gammu_hints.txt
    "docs/develop/protocol/TDMA 5120.txt"
    docs/develop/protocol/carkit.txt
    docs/develop/protocol/n6110.txt
    docs/develop/protocol/n6510.txt
    docs/develop/protocol/n7110.txt
    docs/develop/protocol/nokia.txt
    docs/develop/protocol/readme
    docs/develop/sms/charset.txt
    docs/develop/sms/convert.txt
    docs/develop/sms/readme
    docs/develop/sounds/readme
    docs/develop/sounds/ring2.txt
    )

set (DOCS
    README 
    SUPPORTERS
    ChangeLog 
    COPYING
    docs/user/replace.txt
    docs/user/readme.htm
    docs/user/gammu.htm
    docs/user/gammu.it.txt
    docs/user/readme.it.txt
    )

set (UTILS
    utils/gammu-config
    )

set (SYMBIAN_FILES
    other/symbian/readme.txt
    other/symbian/gnapplet.ini
    other/symbian/gnapplet.sis
    )

set (MAN1_PAGES
    docs/user/gammu.1
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
    common/gsmerror.h
    cfg/config.h
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
    common/device/serial/ser_w32.c
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

# Locales
set (LOCALEFILES)
foreach (LOCALE ${LOCALES})
    list(APPEND LOCALEFILES "locale/${LOCALE}/gammu.po")
endforeach (LOCALE ${LOCALES})
GETTEXT_CREATE_TRANSLATIONS("locale/gammu.pot" ALL ${LOCALEFILES})

# Library
add_library (Gammu ${LIBRARY_SRC})
if (CROSS_MINGW)
    if (ENABLE_SHARED)
        set_target_properties (Gammu PROPERTIES 
            PREFIX "" 
            SUFFIX ".dll"
            IMPORT_PREFIX ""
            IMPORT_SUFFIX ".lib"
            )
    else (ENABLE_SHARED)
        set_target_properties (Gammu PROPERTIES PREFIX "" SUFFIX ".lib")
    endif (ENABLE_SHARED)
else (CROSS_MINGW)
    set_target_properties (Gammu PROPERTIES VERSION ${VERSION} SOVERSION 1)

    # -fPIC on Linux
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        set_target_properties (Gammu PROPERTIES COMPILE_FLAGS "-fPIC")
    endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

endif (CROSS_MINGW)

# Include search path
include_directories (
    "${Gammu_BINARY_DIR}/cfg"
    "${Gammu_SOURCE_DIR}/common"
    )

# Gammu program
add_executable (gammu ${BINARY_SRC})
if (CROSS_MINGW)
    set_target_properties (gammu PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)

target_link_libraries (gammu Gammu)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

# Tune options for found libraries
if (MYSQL_FOUND)
    target_link_libraries (gammu ${MYSQL_LIBRARIES})
    include_directories (${MYSQL_INCLUDE_DIR})
    set (HAVE_MYSQL_MYSQL_H TRUE)
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
    target_link_libraries (gammu ${POSTGRES_LIBRARY})
    include_directories (${POSTGRES_INCLUDE_DIR})

    set (HAVE_POSTGRESQL_LIBPQ_FE_H TRUE)
endif (POSTGRES_FOUND)

if (GETTEXTLIBS_FOUND)
    target_link_libraries (gammu ${GETTEXT_LIBRARY})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

if (BLUEZ_FOUND)
    target_link_libraries (Gammu ${BLUEZ_LIBRARIES})
    include_directories (${BLUEZ_INCLUDE_DIR})

    if (NOT CROSS_MINGW)
        set (GSM_ENABLE_BLUEZ TRUE)
    endif (NOT CROSS_MINGW)
    macro_gammu_option (BLUETOOTH_RF_SEARCHING "Searching for RF channels with Bluetooth stack" ON ON ON)
    if (WITH_BLUETOOTH_RF_SEARCHING)
        set (BLUETOOTH_RF_SEARCHING ON)
    endif (WITH_BLUETOOTH_RF_SEARCHING)
endif (BLUEZ_FOUND)

if (NOT HAVE_SIN)
    target_link_libraries (Gammu m)
endif (NOT HAVE_SIN)

if (CMAKE_USE_PTHREADS_INIT)
    set (HAVE_PTHREAD ON)
endif (CMAKE_USE_PTHREADS_INIT)

# Phone and protocol configuration 

# Nokia phones
macro_gammu_option (NOKIA_SUPPORT "Nokia support" ON ON ON)

macro_gammu_option (MBUS2 "Nokia MBUS2 protocol" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (FBUS2 "Nokia FBUS2 protocol" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (FBUS2DLR3 "Nokia FBUS2DLR3 protocol" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (DKU2PHONET "Nokia DKU2PHONET protocol" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (DKU2AT "Nokia DKU2AT protocol" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (DKU5FBUS2 "Nokia DKU5FBUS2 protocol" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (FBUS2PL2303 "Nokia FBUS2PL2303 protocol" ON WITH_NOKIA_SUPPORT ON)

macro_gammu_option (FBUS2BLUE "Nokia FBUS2BLUE protocol" ON WITH_NOKIA_SUPPORT BLUEZ_FOUND)
macro_gammu_option (PHONETBLUE "Nokia PHONETBLUE protocol" ON WITH_NOKIA_SUPPORT BLUEZ_FOUND)
macro_gammu_option (BLUEFBUS2 "Nokia BLUEFBUS2 protocol" ON WITH_NOKIA_SUPPORT BLUEZ_FOUND)
macro_gammu_option (BLUEPHONET "Nokia BLUEPHONET protocol" ON WITH_NOKIA_SUPPORT BLUEZ_FOUND)

macro_gammu_option (IRDAPHONET "Nokia IRDAPHONET protocol" ON WITH_NOKIA_SUPPORT IRDA_FOUND)
macro_gammu_option (FBUS2IRDA "Nokia FBUS2IRDA protocol" ON WITH_NOKIA_SUPPORT IRDA_FOUND)

macro_gammu_option (NOKIA3320 "Nokia 3320 and compatible phones support" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (NOKIA650 "Nokia 650 and compatible phones support" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (NOKIA6110 "Nokia 61xx and compatible phones support" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (NOKIA6510 "Nokia 6510 and compatible phones support" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (DCT4_CALENDAR_6210 "Force using 6210 frames for calendar for DCT4 phones" OFF WITH_NOKIA_SUPPORT WITH_NOKIA6510)
if (WITH_DCT4_CALENDAR_6210)
    set (GSM_FORCE_DCT4_CALENDAR_6210 TRUE)
endif (WITH_DCT4_CALENDAR_6210)
macro_gammu_option (NOKIA7110 "Nokia 7110 and compatible phones support" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (NOKIA9210 "Nokia 9210 and compatible phones support" ON WITH_NOKIA_SUPPORT ON)
if (WITH_NOKIA7110 OR WITH_NOKIA9210)
    set (wITH_71_92 ON)
else (WITH_NOKIA7110 OR WITH_NOKIA9210)
    set (wITH_71_92)
endif (WITH_NOKIA7110 OR WITH_NOKIA9210)
macro_gammu_option (N71_92INCOMINGINFO "Nokia 62xx/71xx/9xxx incoming call/SMS info" OFF WITH_NOKIA_SUPPORT WITH_71_92)
macro_gammu_option (NOKIA3650 "Nokia 3650 and compatible phones support" ON WITH_NOKIA_SUPPORT ON)

# AT phones
macro_gammu_option (AT_SUPPORT "AT support" ON ON ON)

macro_gammu_option (AT "AT protocol" ON WITH_AT_SUPPORT ON)
macro_gammu_option (BLUEAT "AT protocol over Bluetooth" ON WITH_AT_SUPPORT BLUEZ_FOUND)
macro_gammu_option (IRDAAT "AT protocol over IrDA" ON WITH_AT_SUPPORT IRDA_FOUND)

macro_gammu_option (ATGEN "AT phones support" ON WITH_AT_SUPPORT ON)

macro_gammu_option (ALCATEL_SUPPORT "Alcatel support" ON WITH_AT_SUPPORT ON)

macro_gammu_option (ALCABUS "Alcatel protocol" ON WITH_ALCATEL_SUPPORT ON)

macro_gammu_option (ALCATEL "Alcatel phones support" ON WITH_ALCATEL_SUPPORT ON)

# OBEX phones
macro_gammu_option (OBEX_SUPPORT "OBEX compatible phones support" ON ON ON)

macro_gammu_option (BLUEOBEX "OBEX protocol over Bluetooth" ON WITH_OBEX_SUPPORT BLUEZ_FOUND)
macro_gammu_option (IRDAOBEX "OBEX protocol over IrDA" ON WITH_OBEX_SUPPORT IRDA_FOUND)

macro_gammu_option (OBEXGEN "Generic OBEX phones support" ON WITH_OBEX_SUPPORT ON)
macro_gammu_option (SONYERICSSON "Sony-Ericsson phones support" ON WITH_OBEX_SUPPORT WITH_AT_SUPPORT)

macro_gammu_option (GNAPPLET_SUPPORT "Symbian phones support (gnapplet)" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (BLUEGNAPBUS "GNAPBUS protocol over Bluetooth" ON WITH_GNAPPLET_SUPPORT BLUEZ_FOUND)
macro_gammu_option (IRDAGNAPBUS "GNAPBUS protocol over IrDA" ON WITH_GNAPPLET_SUPPORT IRDA_FOUND)

macro_gammu_option (GNAPGEN "Gnapplet phones support" ON WITH_GNAPPLET_SUPPORT ON)

option (ENABLE_LGPL "Enable LGPL licensing (disables all GPL only code)" OFF)
mark_as_advanced (ENABLE_LGPL)

# Some generic configurations
macro_gammu_option (CELLBROADCAST "Cell Broadcast messages support" ON ON ON)
macro_gammu_option (BACKUP "Backup/Restore functions" ON ON ON)

# Add libraries needed for windows build
if (WIN32)
    target_link_libraries (Gammu ws2_32)
    #    target_link_libraries (Gammu ntdll) # needed for MySQL
    #    target_link_libraries (Gammu msvcr71) # needed for MySQL
    # These libraries are needed for serial driver
    if (GSM_ENABLE_DKU2PHONET OR GSM_ENABLE_DKU2AT)
        target_link_libraries (Gammu setupapi advapi32)
    endif (GSM_ENABLE_DKU2PHONET OR GSM_ENABLE_DKU2AT)
endif (WIN32)

# Generate config.h
configure_file (templates/config.h.cmake cfg/config.h ESCAPE_QUOTES)

# Generate Doxygen file
set (DOXYGEN_INPUT "${Gammu_SOURCE_DIR}/common ${Gammu_SOURCE_DIR}/gammu")
set (DOXYGEN_OUTPUT "${Gammu_BINARY_DIR}/gammu-doc")
if (${DOXYGEN_DOT_EXECUTABLE} STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "NO")
    set (DOXYGEN_DOT_PATH "")
else (${DOXYGEN_DOT_EXECUTABLE} STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "YES")
    # Strip binary name from variable
    string (REGEX REPLACE "/dot$" "" DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE})
endif (${DOXYGEN_DOT_EXECUTABLE} STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
configure_file (templates/Doxyfile.cmake cfg/Doxyfile)
configure_file (templates/mainpage.desc.cmake mainpage.desc)

# Target for generating API documentation
add_custom_target (apidoc ${DOXYGEN_EXECUTABLE} cfg/Doxyfile 
    COMMAND find ${DOXYGEN_OUTPUT}/html -name '*.html' -print0 | xargs -0 sed -i 's@text/html\;charset=iso-8859-1@text/html\;charset=utf-8@'
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
configure_file (templates/gammu.pc.cmake cfg/gammu.pc @ONLY)

# Install instructions
install (TARGETS gammu Gammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT "runtime"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT "runtime"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT "development"
        )

# Install Windows libraries
if (CROSS_MINGW)
    # MySQL
    if (MYSQL_FOUND)
        install (FILES "${MYSQL_CROSS_PATH}/lib/opt/libmysql.dll"
                DESTINATION "${INSTALL_BIN_DIR}" COMPONENT "runtime"
                )
    endif (MYSQL_FOUND)

    # PostgreSQL and dependencies
    if (POSTGRES_FOUND)
        install (FILES 
                "${POSTGRES_CROSS_PATH}/lib/libpq.dll"
                "${POSTGRES_CROSS_PATH}/bin/comerr32.dll"
                "${POSTGRES_CROSS_PATH}/bin/libeay32.dll"
                "${POSTGRES_CROSS_PATH}/bin/libiconv-2.dll"
                "${POSTGRES_CROSS_PATH}/bin/libintl-2.dll"
                "${POSTGRES_CROSS_PATH}/bin/krb5_32.dll"
                "${POSTGRES_CROSS_PATH}/bin/ssleay32.dll"
                "${POSTGRES_CROSS_PATH}/bin/wldap32.dll"
                DESTINATION ${INSTALL_BIN_DIR} COMPONENT "runtime"
                )
    endif (POSTGRES_FOUND)

    # Gettext and dependencies
    if (GETTEXTLIBS_FOUND)
        install (FILES 
                "${GETTEXT_CROSS_PATH}/bin/libintl3.dll"
                "${GETTEXT_CROSS_PATH}/bin/libiconv2.dll"
                DESTINATION "${INSTALL_BIN_DIR}" COMPONENT "runtime"
                )
    endif (GETTEXTLIBS_FOUND)
endif (CROSS_MINGW)

foreach (HEADER ${HEADERS})
    string (REGEX REPLACE "(common|cfg)(.*)/[^/]*$" "\\2" DIRNAME ${HEADER})
    install (
        FILES ${HEADER} 
        DESTINATION "${INSTALL_INC_DIR}/${DIRNAME}" 
        COMPONENT "development"
        )
endforeach (HEADER)

install (
    PROGRAMS ${UTILS}
    DESTINATION "${INSTALL_BIN_DIR}"
    COMPONENT "Utilities"
    )

install (
    FILES "${Gammu_BINARY_DIR}/cfg/gammu.pc"
    DESTINATION "lib/pkgconfig"
    COMPONENT "development"
    )

install (
    FILES ${DOCS}
    DESTINATION "${INSTALL_DOC_DIR}"
    COMPONENT "basic-documentation"
    )

install (
    FILES ${SYMBIAN_FILES}
    DESTINATION "${INSTALL_DOC_DIR}/symbian"
    COMPONENT "gnapplet"
    )

foreach (DOC ${DEVELDOCS})
    string (REGEX REPLACE "docs/develop(.*)/[^/]*$" "\\1" DIRNAME ${DOC})
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

# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

# I didn't check older versions
# However 2.6 is needed for crosscompiling
cmake_minimum_required (VERSION 2.4.2)

# We've set up compiler, now we can use it
project (Gammu C)

# Where to lookup modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Silent some warnings from CMake 2.6
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

option (COVERAGE "Add flags for Coverage analysis" OFF)

if (WIN32)
    set (CPACK_SYSTEM_NAME "Windows" CACHE STRING "Windows package name")
endif (WIN32)

if (MINGW)
    # My MinGW is hacked to use msvcr70.dll, to work with Python
    add_definitions(-D_MSVCRT_VERSION__=0x070)
endif (MINGW)

# Set version
set (VERSION "1.22.90" CACHE INTERNAL "")
set (VERSION_WIN "1,22,90,0" CACHE INTERNAL "")
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)$" "\\1" "VERSION_MAJOR" "${VERSION}")
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)$" "\\2" "VERSION_MINOR" "${VERSION}")
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)$" "\\3" "VERSION_PATCH" "${VERSION}")
message (STATUS "Configuring ${CMAKE_PROJECT_NAME} ${VERSION}")

set (SOVERSION "5" CACHE INTERNAL "")

if ("${Gammu_BINARY_DIR}" STREQUAL "${Gammu_SOURCE_DIR}")
    message ("Warning: In tree build is not recommended way to build Gammu.")
endif ("${Gammu_BINARY_DIR}" STREQUAL "${Gammu_SOURCE_DIR}")

# For debugging
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set (CMAKE_VERBOSE_MAKEFILE ON)
    set (DEBUG 1)
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (DEBUG 0)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

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
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #set (CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  #set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set (CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  set (CPACK_NSIS_HELP_LINK "http://www.gammu.org")
  set (CPACK_NSIS_URL_INFO_ABOUT "http://www.gammu.org")
  set (CPACK_NSIS_CONTACT "michal@cihar.com")
  set (CPACK_NSIS_MODIFY_PATH ON)
else (WIN32)
  set (CPACK_STRIP_FILES "gammu")
  set (CPACK_SOURCE_STRIP_FILES "")
endif (WIN32)
include (CPack)

# We want some tests. Even when cross compiling for Windows tests can be
# done in Wine. This is better than nothing, even though Wine is a bit
# more fault tolerant when linking DLLs.
include(CTest)

# Standard packages
include (CheckCSourceCompiles)
include (CheckCCompilerFlag)
include (CheckIncludeFile)
include (CheckFunctionExists)
include (CheckSymbolExists)
include (CheckTypeSize)
include (CheckLibraryExists)

# Packages in our sources (mostly taken from KDE)
include (MacroOptionalFindPackage)
include (MacroAddDLL)
include (MacroGammuOption)
include (MacroTuneCompiler)

# WE use pkgconfig later
find_package (PkgConfig)

# Standard packages
macro_optional_find_package (Doxygen)

# Configuration checks
check_symbol_exists (strncasecmp "string.h" HAVE_STRNCASECMP)
check_symbol_exists (strcasecmp "string.h" HAVE_STRCASECMP)
check_function_exists (strcasestr HAVE_STRCASESTR)
check_function_exists (strchrnul HAVE_STRCHRNUL)
check_function_exists (strtoull HAVE_STRTOULL)
check_include_file (dirent.h HAVE_DIRENT_H)
check_include_file (sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_file (sys/utsname.h HAVE_SYS_UTSNAME_H)
check_include_file (unistd.h HAVE_UNISTD_H)
check_include_file (wchar.h HAVE_WCHAR_H)
check_include_file (wctype.h HAVE_WCTYPE_H)
# These checks are required for check_type_size() on cmake < 2.6.0
check_include_file (sys/types.h HAVE_SYS_TYPES_H)
check_include_file (stdint.h HAVE_STDINT_H)
check_include_file (stddef.h HAVE_STDDEF_H)
check_function_exists (scandir HAVE_SCANDIR)
check_function_exists (alphasort HAVE_ALPHASORT)
check_symbol_exists (iswspace "wctype.h" HAVE_ISWSPACE)
check_symbol_exists (towlower "wctype.h" HAVE_TOWLOWER)
check_symbol_exists (getopt "unistd.h" HAVE_GETOPT)
check_symbol_exists (getopt_long "getopt.h" HAVE_GETOPT_LONG)
check_symbol_exists (daemon "unistd.h" HAVE_DAEMON)
check_symbol_exists (kill "signal.h" HAVE_KILL)
check_symbol_exists (SIGHUP "signal.h" HAVE_SIGHUP)
check_c_source_compiles ("
#define _BSD_SOURCE
#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>

int main(void) {
    va_list ap;
    vsyslog(LOG_NOTICE, \"aaa\", ap);
    return 0;
}
"  HAVE_VSYSLOG)
# Some compilers (eg. BCC) have this in ctype.h
if (NOT HAVE_TOWLOWER)
    check_symbol_exists (towlower "ctype.h" HAVE_TOWLOWER_CTYPE)
    if (HAVE_TOWLOWER_CTYPE)
        set (HAVE_TOWLOWER ${HAVE_TOWLOWER_CTYPE} CACHE INTERNAL "")
    endif (HAVE_TOWLOWER_CTYPE)
endif (NOT HAVE_TOWLOWER)

# Standard packages
macro_optional_find_package (Threads)

# Packages in sources
macro_optional_find_package (MySQL)
macro_optional_find_package (Postgres)
macro_optional_find_package (GettextLibs)
macro_optional_find_package (Iconv)
macro_optional_find_package (CURL)

if (CMAKE_CROSSCOMPILING AND MINGW)
    set (HAVE_WINT_T 4 CACHE INTERNAL "")
    set (HAVE_WCHAR_T 4 CACHE INTERNAL "")
else (CMAKE_CROSSCOMPILING AND MINGW)
    # Search for needed includes and functions
    if (HAVE_WCHAR_H)
        set (CMAKE_EXTRA_INCLUDE_FILES wchar.h)
        check_type_size (wchar_t HAVE_WCHAR_T)
    endif (HAVE_WCHAR_H)
    if (HAVE_WCTYPE_H)
        set (CMAKE_EXTRA_INCLUDE_FILES wctype.h)
        check_type_size (wint_t  HAVE_WINT_T)
    endif (HAVE_WCTYPE_H)
    set (CMAKE_EXTRA_INCLUDE_FILES)
endif (CMAKE_CROSSCOMPILING AND MINGW)

check_type_size (ssize_t HAVE_SSIZE_T)
check_type_size(intptr_t HAVE_INTPTR_T)

check_c_source_compiles ("
#include <stdio.h>

int main(void) {
    printf( __FUNCTION__);
    return 0;
}
"  HAVE_MACRO_FUNCTION)

check_c_source_compiles ("
#include <time.h>

int main(void) {
    struct tm tm;
    tm.tm_zone;
    return 0;
}
"  HAVE_STRUCT_TM_TM_ZONE)

check_c_source_compiles ("
#include <stdio.h>

int main(void) {
    printf( __FUNC__);
    return 0;
}
"  HAVE_MACRO_FUNC)

OPTION(WITH_BLUETOOTH "Bluetooth support" ON)
if (WITH_BLUETOOTH)
    if (WIN32)
        # FIXME: This is currently hardcoded here, maybe there is test?
        set(BLUETOOTH_FOUND ON)
        message(STATUS "Using Windows native Bluetooth")
        set(BLUETOOTH_SEARCH TRUE)
    else (WIN32)
        find_package (Bluez)
        if (BLUEZ_FOUND)
            set(BLUETOOTH_FOUND ON)
            set(BLUETOOTH_SEARCH TRUE)
        endif (BLUEZ_FOUND)
        find_package (FBSDBluetooth)
        if (FBSD_BLUE_FOUND)
            set(BLUETOOTH_FOUND ON)
            set(BLUETOOTH_SEARCH FALSE)
        endif (FBSD_BLUE_FOUND)
        find_package (OSXBluetooth)
        if (OSX_BLUE_FOUND)
            set(BLUETOOTH_FOUND ON)
            set(BLUETOOTH_SEARCH FALSE)
        endif (OSX_BLUE_FOUND)
    endif (WIN32)
else (WITH_BLUETOOTH)
    set(BLUETOOTH_FOUND)
    set(BLUETOOTH_INCLUDE_DIR)
    set(BLUETOOTH_INCLUDES)
    set(BLUETOOTH_LIBRARY)
    set(BLUETOOTH_LIBRARIES)
endif (WITH_BLUETOOTH)

if (BLUETOOTH_FOUND)
    message(STATUS "Bluetooth support enabled")
else (BLUETOOTH_FOUND)
    if (WITH_BLUETOOTH)
        message("Bluetooth support disabled, please install libbluetooth-dev or equivalent to enable Bluetooth.")
    endif (WITH_BLUETOOTH)
endif (BLUETOOTH_FOUND)

macro_gammu_option (BLUETOOTH_RF_SEARCHING "Searching for RF channels with Bluetooth stack" ON BLUETOOTH_FOUND BLUETOOTH_SEARCH)
if (WITH_BLUETOOTH_RF_SEARCHING)
    set (BLUETOOTH_RF_SEARCHING ON)
endif (WITH_BLUETOOTH_RF_SEARCHING)

OPTION(WITH_IRDA "IrDA support" ON)
if (WITH_IRDA)
    if (WIN32)
        # FIXME: This is currently hardcoded here, maybe there is test?
        set (IRDA_FOUND YES)
    else (WIN32)
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
    endif (WIN32)
else (WITH_IRDA)
    set (IRDA_FOUND)
endif (WITH_IRDA)

if (IRDA_FOUND)
    message(STATUS "IrDA support enabled")
endif (IRDA_FOUND)

if (MYSQL_FOUND)
    set (HAVE_MYSQL_MYSQL_H TRUE)
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
    set (HAVE_POSTGRESQL_LIBPQ_FE_H TRUE)
endif (POSTGRES_FOUND)

set (GAMMU_LIBS "")

if (BLUEZ_FOUND)
    if (NOT "${BLUEZ_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${BLUEZ_LIBRARIES}")
    endif (NOT "${BLUEZ_LIBRARIES}" STREQUAL "")
endif (BLUEZ_FOUND)

if (FBSD_BLUE_FOUND)
    if (NOT "${FBSD_BLUE_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${FBSD_BLUE_LIBRARIES}")
    endif (NOT "${FBSD_BLUE_LIBRARIES}" STREQUAL "")
endif (FBSD_BLUE_FOUND)

if (ICONV_FOUND)
    if (NOT "${ICONV_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${ICONV_LIBRARIES}")
    endif (NOT "${ICONV_LIBRARIES}" STREQUAL "")
endif (ICONV_FOUND)

if (GETTEXTLIBS_FOUND)
    if (NOT "${GETTEXT_LIBRARIES}" STREQUAL "" AND NOT "${GETTEXT_LIBRARIES}" STREQUAL "/usr/lib/libc.so")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${GETTEXT_LIBRARIES}")
    endif (NOT "${GETTEXT_LIBRARIES}" STREQUAL "" AND NOT "${GETTEXT_LIBRARIES}" STREQUAL "/usr/lib/libc.so")
endif (GETTEXTLIBS_FOUND)

set (SMSD_LIBS "")

if (MYSQL_FOUND)
    if (NOT "${MYSQL_LIBRARIES}" STREQUAL "")
        set (SMSD_LIBS "${SMSD_LIBS} -l${MYSQL_LIBRARIES}")
    endif (NOT "${MYSQL_LIBRARIES}" STREQUAL "")
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
    if (NOT "${POSTGRES_LIBRARY}" STREQUAL "")
        set (SMSD_LIBS "${SMSD_LIBS} -l${POSTGRES_LIBRARY}")
    endif (NOT "${POSTGRES_LIBRARY}" STREQUAL "")
endif (POSTGRES_FOUND)

# Tweak compiler flags
if(MSVC)
    # MSVC needs different flags at all
    MACRO_TUNE_COMPILER("/W3")
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_MINGW OR
    CMAKE_COMPILER_IS_CYGWIN OR CMAKE_COMPILER_IS_GNUCXX)
    # Check for extra compiler flags we want to use
    if (NOT VERSION_PATCH EQUAL 0)
        MACRO_TUNE_COMPILER("-Werror")
    endif (NOT VERSION_PATCH EQUAL 0)

    #
    # Warnings related flags
    #
    MACRO_TUNE_COMPILER("-Wall")
    MACRO_TUNE_COMPILER("-Werror-implicit-function-declaration")
    MACRO_TUNE_COMPILER("-Wno-deprecated-declarations")
    MACRO_TUNE_COMPILER("-Wdeclaration-after-statement")
    MACRO_TUNE_COMPILER("-Wpointer-arith")
    MACRO_TUNE_COMPILER("-Wfloat-equal")
    MACRO_TUNE_COMPILER("-Wbad-function-cast")
    MACRO_TUNE_COMPILER("-Wmissing-format-attribute")
    MACRO_TUNE_COMPILER("-Wmissing-noreturn")
#   This should be enabled and only public functions exported, but it is far future
#    MACRO_TUNE_COMPILER("-Wmissing-prototypes")
    MACRO_TUNE_COMPILER("-Wpointer-arith")
    MACRO_TUNE_COMPILER("-Wredundant-decls")
    MACRO_TUNE_COMPILER("-Wshadow")
    MACRO_TUNE_COMPILER("-Wstrict-prototypes")
#   We need some typecasting (especially for iconv)
#    MACRO_TUNE_COMPILER("-Wcast-qual")
    MACRO_TUNE_COMPILER("-Wwrite-strings")

    # Does not work well
    #MACRO_TUNE_COMPILER("-Wunreachable-code")

    # Just silent this for now, too much code to fix here
    MACRO_TUNE_COMPILER("-Wno-pointer-sign")

    # Check format strings
    MACRO_TUNE_COMPILER("-Wshadow")
    MACRO_TUNE_COMPILER("-Wformat=2")
    MACRO_TUNE_COMPILER("-Wno-format-y2k")
    # Win32 implementation of gettext does not allow us to use so strict warnings
    if (WIN32)
        MACRO_TUNE_COMPILER("-Wno-format-nonliteral")
        MACRO_TUNE_COMPILER("-Wno-format-security")
    endif (WIN32)

    # Win32 implementation of gettext does not allow us to use so strict warnings
    if (COVERAGE)
        MACRO_TUNE_COMPILER_LINKER("-fprofile-arcs -ftest-coverage")
    endif (COVERAGE)

    # This is extremely noisy, use only in debug builds
    if (DEBUG)
        MACRO_TUNE_COMPILER("-Wextra")
        # Just silent this for now, too much code to fix here
        MACRO_TUNE_COMPILER("-Wno-unused-parameter")
        # Not sure if this is safe for all architectures
        MACRO_TUNE_COMPILER("-fstrict-aliasing")
    endif (DEBUG)

    #
    # Security related flags
    #
    set (ENABLE_PROTECTION ON CACHE BOOL "Enable compile time protections (stack, read only code, fortify in libc...)")
    if (ENABLE_PROTECTION)
        MACRO_TUNE_COMPILER("-Wp,-D_FORTIFY_SOURCE=2")

        # Text address randomisation, disabled for now, seems to cause problems
        #    MACRO_TUNE_COMPILER("-fPIE")
        #    MACRO_TUNE_LINKER("-pie")
        # These do not work on Windows right now
        if (NOT WIN32)
            # Stack protector
            MACRO_TUNE_COMPILER("-fstack-protector")
            # Mark code read only
            MACRO_TUNE_LINKER("-Wl,-zrelro")
        endif (NOT WIN32)
    endif (ENABLE_PROTECTION)

    #
    # Other flags
    #

    if (WIN32 AND BUILD_SHARED_LIBS)
        MACRO_TUNE_LINKER("-Wl,--enable-auto-import")
    endif (WIN32 AND BUILD_SHARED_LIBS)
endif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_MINGW OR
    CMAKE_COMPILER_IS_CYGWIN OR CMAKE_COMPILER_IS_GNUCXX)

if(BORLAND)
    # Borland
    # Suggests parenthesis
    MACRO_TUNE_COMPILER("-wprc")
    # Do not warn about unused params
    MACRO_TUNE_COMPILER("-w-par")
    # Warn about unused vars
    MACRO_TUNE_COMPILER("-wuse")
endif(BORLAND)


# Define required libraries for gammu library if it is static
if (BUILD_SHARED_LIBS)
    set (GAMMU_PRIVATE_LIBS "" CACHE INTERNAL "Private libraries for gammu library")
    set (SMSD_PRIVATE_LIBS "" CACHE INTERNAL "Private libraries for gammu smsd library")
else (BUILD_SHARED_LIBS)
    string(REGEX REPLACE "-l(/usr)?/lib/lib([^ ]*).so" "-l\\2" G_LIB "${GAMMU_LIBS}")
    string(REGEX REPLACE "-l(/usr)?/lib/lib([^ ]*).so" "-l\\2" S_LIB "${SMSD_LIBS}")
    set (GAMMU_PRIVATE_LIBS "${G_LIB}" CACHE INTERNAL "Private libraries for gammu library")
    set (SMSD_PRIVATE_LIBS "${S_LIB}" CACHE INTERNAL "Private libraries for gammu smsd library")
endif (BUILD_SHARED_LIBS)

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
    docs/develop/PORTING
    docs/develop/develop.txt
    docs/develop/gammu_hints.txt
    docs/develop/protocol/TDMA_5120.txt
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
    docs/develop/examples/Makefile
    docs/develop/examples/phone-info.c
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
    utils/jadmaker
    )

set (SYMBIAN_FILES
    other/symbian/readme.txt
    other/symbian/gnapplet.ini
    other/symbian/gnapplet.sis
    )

set (MAN1_PAGES
    docs/user/gammu.1
    docs/user/gammu-config.1
    docs/user/gammu-smsd.1
    docs/user/jadmaker.1
    )

set (MAN5_PAGES
    docs/user/gammu-smsdrc.5
    docs/user/gammurc.5
    )

set (PUBLIC_HEADERS
    include/gammu.h
    include/gammu-info.h
    include/gammu-callback.h
    include/gammu-memory.h
    include/gammu-security.h
    include/gammu-datetime.h
    include/gammu-limits.h
    include/gammu-types.h
    include/gammu-backup.h
    include/gammu-unicode.h
    include/gammu-misc.h
    include/gammu-settings.h
    include/gammu-error.h
    include/gammu-wap.h
    include/gammu-bitmap.h
    include/gammu-category.h
    include/gammu-debug.h
    include/gammu-message.h
    include/gammu-file.h
    include/gammu-statemachine.h
    include/gammu-keys.h
    include/gammu-inifile.h
    include/gammu-calendar.h
    include/gammu-call.h
    include/gammu-ringtone.h
    include/gammu-nokia.h
    smsd/gammu-smsd.h
    ${Gammu_BINARY_DIR}/include/gammu-config.h
    )

set (HEADERS
    common/device/bluetoth/blue_fbsd.h
    common/device/bluetoth/blue_osx.h
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
    common/misc/coding/md5.h
    common/misc/coding/coding.h
    common/misc/misc.h
    common/phone/alcatel/alcatel.h
    common/phone/at/samsung.h
    common/phone/at/siemens.h
    common/phone/at/motorola.h
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
    common/phone/atobex/atobexfunc.h
    common/phone/atobex/atobex.h
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
    common/service/gsmnet.h
    common/service/sms/gsmems.h
    common/service/sms/gsmmulti.h
    common/service/gsmlogo.h
    common/service/gsmcal.h
    common/service/gsmring.h
    common/service/gsmpbk.h
    common/service/gsmcall.h
    common/service/gsmdata.h
    common/service/gsmmisc.h
    common/gsmstate.h
    common/gsmcomon.h
    ${Gammu_BINARY_DIR}/include/gammu-config.h
    )

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

macro_gammu_option (FBUS2BLUE "Nokia FBUS2BLUE protocol" ON WITH_NOKIA_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (PHONETBLUE "Nokia PHONETBLUE protocol" ON WITH_NOKIA_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (BLUEFBUS2 "Nokia BLUEFBUS2 protocol" ON WITH_NOKIA_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (BLUEPHONET "Nokia BLUEPHONET protocol" ON WITH_NOKIA_SUPPORT BLUETOOTH_FOUND)

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
macro_gammu_option (BLUEAT "AT protocol over Bluetooth" ON WITH_AT_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (IRDAAT "AT protocol over IrDA" ON WITH_AT_SUPPORT IRDA_FOUND)

macro_gammu_option (ATGEN "AT phones support" ON WITH_AT_SUPPORT ON)

macro_gammu_option (ALCATEL_SUPPORT "Alcatel support" ON WITH_AT_SUPPORT ON)

macro_gammu_option (ALCABUS "Alcatel protocol" ON WITH_ALCATEL_SUPPORT ON)

macro_gammu_option (ALCATEL "Alcatel phones support" ON WITH_ALCATEL_SUPPORT ON)

# OBEX phones
macro_gammu_option (OBEX_SUPPORT "OBEX compatible phones support" ON ON ON)

macro_gammu_option (BLUEOBEX "OBEX protocol over Bluetooth" ON WITH_OBEX_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (IRDAOBEX "OBEX protocol over IrDA" ON WITH_OBEX_SUPPORT IRDA_FOUND)

macro_gammu_option (OBEXGEN "Generic OBEX phones support" ON WITH_OBEX_SUPPORT ON)
macro_gammu_option (ATOBEX "AT with OBEX phones support" ON WITH_OBEX_SUPPORT WITH_AT_SUPPORT)

macro_gammu_option (GNAPPLET_SUPPORT "Symbian phones support (gnapplet)" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (BLUEGNAPBUS "GNAPBUS protocol over Bluetooth" ON WITH_GNAPPLET_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (IRDAGNAPBUS "GNAPBUS protocol over IrDA" ON WITH_GNAPPLET_SUPPORT IRDA_FOUND)

macro_gammu_option (GNAPGEN "Gnapplet phones support" ON WITH_GNAPPLET_SUPPORT ON)

# Some generic configurations
macro_gammu_option (CELLBROADCAST "Cell Broadcast messages support" ON ON ON)
macro_gammu_option (BACKUP "Backup/Restore functions" ON ON ON)

# Generate config.h
configure_file ("${Gammu_SOURCE_DIR}/templates/gammu-config.h.cmake" "${Gammu_BINARY_DIR}/include/gammu-config.h" ESCAPE_QUOTES)

# Fill in include dir for build
foreach (HEADER ${PUBLIC_HEADERS})
    if (NOT ${HEADER} MATCHES "gammu-config.h")
        string (REGEX REPLACE "(include|smsd)(.*)/([^/]*)$" "\\3" FILENAME ${HEADER})
        configure_file ("${Gammu_SOURCE_DIR}/${HEADER}" "${Gammu_BINARY_DIR}/include/${FILENAME}" COPY_ONLY)
    endif (NOT ${HEADER} MATCHES "gammu-config.h")
endforeach (HEADER)

# Generate Doxygen file
set (DOXYGEN_INPUT "${Gammu_BINARY_DIR}/include")
set (DOXYGEN_INTERNAL_INPUT "${Gammu_SOURCE_DIR}/common")
set (DOXYGEN_OUTPUT "${Gammu_BINARY_DIR}/gammu-doc")
set (DOXYGEN_INTERNAL_OUTPUT "${Gammu_BINARY_DIR}/gammu-internal-doc")
if ("${DOXYGEN_DOT_EXECUTABLE}" STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "NO")
    set (DOXYGEN_DOT_PATH "")
else ("${DOXYGEN_DOT_EXECUTABLE}" STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "YES")
    # Strip binary name from variable
    string (REGEX REPLACE "/dot$" "" DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}")
endif ("${DOXYGEN_DOT_EXECUTABLE}" STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
configure_file ("${Gammu_SOURCE_DIR}/templates/Doxyfile.cmake" "${Gammu_BINARY_DIR}/doxygen/Doxyfile")
configure_file ("${Gammu_SOURCE_DIR}/templates/Doxyfile-internal.cmake" "${Gammu_BINARY_DIR}/doxygen/Doxyfile-internal")
configure_file ("${Gammu_SOURCE_DIR}/templates/api.desc.cmake" "${Gammu_BINARY_DIR}/doxygen/api.desc")
configure_file ("${Gammu_SOURCE_DIR}/templates/internals.desc.cmake" "${Gammu_BINARY_DIR}/doxygen/internals.desc")

# Target for generating API documentation
add_custom_target (apidoc ${DOXYGEN_EXECUTABLE} doxygen/Doxyfile
    COMMAND find ${DOXYGEN_OUTPUT}/html -name '*.html' -print0 | xargs -0 sed -i 's@text/html\;charset=iso-8859-1@text/html\;charset=utf-8@'
    DEPENDS ${PUBLIC_HEADERS}
    COMMENT "Generating API documentation")

add_custom_target (interndoc ${DOXYGEN_EXECUTABLE} doxygen/Doxyfile-internal
    COMMAND find ${DOXYGEN_INTERNAL_OUTPUT}/html -name '*.html' -print0 | xargs -0 sed -i 's@text/html\;charset=iso-8859-1@text/html\;charset=utf-8@'
    DEPENDS ${LIBRARY_SRC} ${HEADERS}
    COMMENT "Generating internal documentation")

# Install paths
set (INSTALL_BIN_DIR "bin" CACHE STRING "Path for binaries installation")
mark_as_advanced (INSTALL_BIN_DIR)

set (INSTALL_LIB_DIR "lib${LIB_SUFFIX}" CACHE STRING "Path for libraries installation")
mark_as_advanced (INSTALL_LIB_DIR)

set (INSTALL_LIBDATA_DIR "lib${LIB_SUFFIX}" CACHE STRING "Path for libraries data (eg. pkgconfig data) installation")
mark_as_advanced (INSTALL_LIBDATA_DIR)

set (INSTALL_INC_DIR "include/gammu" CACHE STRING "Path for includes installation")
mark_as_advanced (INSTALL_INC_DIR)

set (INSTALL_LOC_DIR "share/gammu" CACHE STRING "Path for locales installation")
mark_as_advanced (INSTALL_LOC_DIR)

set (INSTALL_DOC_DIR "share/doc/gammu" CACHE STRING "Path for documentation installation")
mark_as_advanced (INSTALL_DOC_DIR)

set (INSTALL_MAN_DIR "share/man" CACHE STRING "Path for man pages installation")
mark_as_advanced (INSTALL_MAN_DIR)

# Generate pkgconfig file
configure_file ("${Gammu_SOURCE_DIR}/templates/gammu.pc.cmake" "${Gammu_BINARY_DIR}/cfg/gammu.pc" @ONLY)
configure_file ("${Gammu_SOURCE_DIR}/templates/gammu-smsd.pc.cmake" "${Gammu_BINARY_DIR}/cfg/gammu-smsd.pc" @ONLY)
configure_file ("${Gammu_SOURCE_DIR}/templates/gammu-uninstalled.pc.cmake" "${Gammu_BINARY_DIR}/pkgconfig/gammu.pc" @ONLY)
configure_file ("${Gammu_SOURCE_DIR}/templates/gammu-smsd-uninstalled.pc.cmake" "${Gammu_BINARY_DIR}/pkgconfig/gammu-smsd.pc" @ONLY)

# Install instructions

# Install Windows libraries
if (WIN32)
    # MySQL
    if (MYSQL_FOUND)
        ADD_DLL("libmysql.dll" "${MYSQL_LIBRARIES}")
    endif (MYSQL_FOUND)

    # PostgreSQL and dependencies
    if (POSTGRES_FOUND)
        ADD_DLL("libpq.dll" "${POSTGRES_LIBRARY}")
    endif (POSTGRES_FOUND)

    # iconv and dependencies
    if (ICONV_FOUND)
       ADD_DLL("libiconv-2.dll" "${ICONV_LIBRARIES}")
    endif (ICONV_FOUND)

    # Gettext and dependencies
    if (GETTEXTLIBS_FOUND)
        ADD_DLL("libintl-8.dll" "${LIBINTL_LIBRARY}")
    endif (GETTEXTLIBS_FOUND)

    # Gettext and dependencies
    if (CURL_FOUND)
        ADD_DLL("libcurl-4.dll" "${CURL_LIBRARIES}")
        ADD_DLL("mgwz.dll" "${CURL_LIBRARIES}")
    endif (CURL_FOUND)

    install (FILES ${WIN32_INSTALL_DLL}
            DESTINATION "${INSTALL_BIN_DIR}"
            )
endif (WIN32)

foreach (HEADER ${PUBLIC_HEADERS})
    string (REPLACE "${Gammu_BINARY_DIR}" "" DIRNAME ${HEADER})
    string (REGEX REPLACE "(include|smsd)(.*)/[^/]*$" "\\2" DIRNAME ${DIRNAME})
    install (
        FILES ${HEADER}
        DESTINATION "${INSTALL_INC_DIR}/${DIRNAME}"
        )
endforeach (HEADER)

install (
    PROGRAMS ${UTILS}
    DESTINATION "${INSTALL_BIN_DIR}"
    )

install (
    FILES "${Gammu_BINARY_DIR}/cfg/gammu.pc"
    DESTINATION "${INSTALL_LIBDATA_DIR}/pkgconfig"
    )

install (
    FILES ${DOCS}
    DESTINATION "${INSTALL_DOC_DIR}"
    )

install (
    FILES ${SYMBIAN_FILES}
    DESTINATION "${INSTALL_DOC_DIR}/symbian"
    )

foreach (DOC ${DEVELDOCS})
    string (REGEX REPLACE "docs/develop(.*)/[^/]*$" "\\1" DIRNAME ${DOC})
    install (
        FILES ${DOC}
        DESTINATION "${INSTALL_DOC_DIR}/devel/${DIRNAME}"
        )
endforeach (DOC)

foreach (EXAMPLE ${EXAMPLES})
    string (REGEX REPLACE "(docs/examples|other/php)(.*)/[^/]*$" "\\2" DIRNAME ${EXAMPLE})
    install (
        FILES ${EXAMPLE}
        DESTINATION "${INSTALL_DOC_DIR}/examples/${DIRNAME}"
        )
endforeach (EXAMPLE)

install (
    FILES ${MAN1_PAGES}
    DESTINATION "${INSTALL_MAN_DIR}/man1"
    )

install (
    FILES ${MAN5_PAGES}
    DESTINATION "${INSTALL_MAN_DIR}/man5"
    )

add_subdirectory(common)

# Include tests, we build them also when cross compiling
add_subdirectory(tests)

add_subdirectory(smsd)

add_subdirectory(gammu)

add_subdirectory(locale)

enable_testing()
include(Dart)


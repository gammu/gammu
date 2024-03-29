# cmake <https://cmake.org/> build file for Gammu
# Copyright (c) 2007 - 2018 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

cmake_minimum_required (VERSION 3.0)
INCLUDE (CMakeForceCompiler)

project (Gammu C)

# Where to lookup modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Silent some warnings from CMake 2.6
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0009 NEW)
cmake_policy(SET CMP0110 NEW)

option (COVERAGE "Add flags for Coverage analysis" OFF)

option (ONLINE_TESTING "Enable testing of parts which use remote servers" OFF)
option (PSQL_TESTING "Enable testing of PostgreSQL SMSD backend" OFF)
option (MYSQL_TESTING "Enable testing of MySQL SMSD backend" OFF)
option (ODBC_TESTING "Enable testing of ODBC MySQL SMSD backend" OFF)
option (BUILD_SHARED_LIBS "Build shared libraries" ON)

option (LARGE_FILES "Support for large files" ON)
if (LARGE_FILES)
    add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
endif (LARGE_FILES)

# Set version
set (GAMMU_VERSION "1.42.0" CACHE INTERNAL "")
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)$" "\\1" "GAMMU_VERSION_MAJOR" "${GAMMU_VERSION}")
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)$" "\\2" "GAMMU_VERSION_MINOR" "${GAMMU_VERSION}")
string (REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)$" "\\3" "GAMMU_VERSION_PATCH" "${GAMMU_VERSION}")
math(EXPR GAMMU_VERSION_NUM "${GAMMU_VERSION_MAJOR} * 10000 + ${GAMMU_VERSION_MINOR} * 100 + ${GAMMU_VERSION_PATCH}")
message (STATUS "Configuring ${CMAKE_PROJECT_NAME} ${GAMMU_VERSION}")

set(CMAKE_C_STANDARD 11)

set (GAMMU_SOVERSION "8" CACHE INTERNAL "")

if ("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    message ("Warning: In tree build is not recommended way to build Gammu.")
endif ("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

# For debugging
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set (CMAKE_VERBOSE_MAKEFILE ON)
    set (DEBUG 1)
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (DEBUG 0)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Standard packages
include (CheckCSourceCompiles)
include (CheckCCompilerFlag)
include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)
include (CheckTypeSize)
include (CheckLibraryExists)
include (FindPkgConfig)

# Packages in our sources (mostly taken from KDE)
include (MacroOptionalFindPackage)
include (MacroAddDLL)
include (MacroGammuOption)
include (MacroTuneCompiler)
include (MSVCRuntime)

# WE use pkgconfig later
find_package (PkgConfig)

# enable code coverage
find_package(codecov)

# Standard packages
macro_optional_find_package (Doxygen)

# I'd say this is task of CMake to export this
check_symbol_exists (__clang__ "" CLANG_COMPILER)

# Configuration checks
check_symbol_exists (strncasecmp "string.h" HAVE_STRNCASECMP)
check_symbol_exists (strcasecmp "string.h" HAVE_STRCASECMP)
check_symbol_exists (_strnicmp "string.h" HAVE_STRNICMP)
check_symbol_exists (_stricmp "string.h" HAVE_STRICMP)
check_function_exists (strcasestr HAVE_STRCASESTR)
check_function_exists (strchrnul HAVE_STRCHRNUL)
check_function_exists (strtoull HAVE_STRTOULL)
check_include_file (dirent.h HAVE_DIRENT_H)
check_include_file (sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_file (sys/utsname.h HAVE_SYS_UTSNAME_H)
check_include_file (unistd.h HAVE_UNISTD_H)

check_include_file (wchar.h HAVE_WCHAR_H)
if (HAVE_WCHAR_H)
    check_include_files ("wchar.h;wctype.h" HAVE_WCTYPE_H)
    check_symbol_exists (iswspace "wchar.h;wctype.h" HAVE_ISWSPACE)
    check_symbol_exists (towlower "wchar.h;wctype.h" HAVE_TOWLOWER)
else (HAVE_WCHAR_H)
    check_include_file (wctype.h HAVE_WCTYPE_H)
    check_symbol_exists (iswspace "wctype.h" HAVE_ISWSPACE)
    check_symbol_exists (towlower "wctype.h" HAVE_TOWLOWER)
endif (HAVE_WCHAR_H)
check_symbol_exists (I_SETSIG "stropts.h" HAVE_I_SETSIG)
check_include_file (strings.h HAVE_STRINGS_H)
check_function_exists (scandir HAVE_SCANDIR)
check_function_exists (alphasort HAVE_ALPHASORT)

set (ENABLE_GETOPT ON CACHE BOOL "Enable getopt usage")
set (USE_WCHAR_T ON CACHE BOOL "Use native wchar_t type")
if (ENABLE_GETOPT)
check_symbol_exists (getopt "unistd.h" HAVE_GETOPT)
check_symbol_exists (getopt_long "getopt.h" HAVE_GETOPT_LONG)
endif (ENABLE_GETOPT)
check_symbol_exists (daemon "unistd.h" HAVE_DAEMON_UNISTD)
check_symbol_exists (daemon "stdlib.h" HAVE_DAEMON_STDLIB)
check_symbol_exists (kill "signal.h" HAVE_KILL)
check_symbol_exists (sigtimedwait "signal.h" HAVE_SIGTIMEDWAIT)
check_symbol_exists (getpwnam "pwd.h" HAVE_GETPWNAM)
check_symbol_exists (getgrnam "grp.h" HAVE_GETGRNAM)
check_symbol_exists (getpwuid "pwd.h" HAVE_GETPWUID)
check_symbol_exists (getuid "unistd.h" HAVE_GETUID)
check_symbol_exists (initgroups "grp.h" HAVE_INITGROUPS)
check_symbol_exists (setuid "unistd.h" HAVE_SETUID)
check_symbol_exists (setgid "unistd.h" HAVE_SETGID)
check_symbol_exists (SIGHUP "signal.h" HAVE_SIGHUP)
check_symbol_exists (SIGUSR1 "signal.h" HAVE_SIGUSR1)
check_symbol_exists (SIGUSR2 "signal.h" HAVE_SIGUSR2)
check_symbol_exists (dup "unistd.h" HAVE_DUP_UNISTD_H)
check_symbol_exists (getpid "unistd.h" HAVE_GETPID)
check_symbol_exists (getpass "unistd.h" HAVE_GETPASS)
check_symbol_exists (alarm "unistd.h" HAVE_ALARM)
check_symbol_exists (dup "io.h" HAVE_DUP_IO_H)
check_symbol_exists (shmget "sys/shm.h" HAVE_SHM)
check_c_source_compiles ("
#define _XOPEN_SOURCE
#define _BSD_SOURCE
#include <time.h>

int main(void) {
    struct tm timestruct;
    strptime(\"11:22\", \"%H:%M\", &timestruct);
    return 0;
}" HAVE_STRPTIME)
check_c_source_compiles ("
#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>

int main(void) {
    syslog(LOG_NOTICE, \"aaa %d\", 1);
    return 0;
}
"  HAVE_SYSLOG)
# Some compilers (eg. BCC) have this in ctype.h
if (NOT HAVE_TOWLOWER)
    check_symbol_exists (towlower "ctype.h" HAVE_TOWLOWER_CTYPE)
    if (HAVE_TOWLOWER_CTYPE)
        set (HAVE_TOWLOWER ${HAVE_TOWLOWER_CTYPE} CACHE INTERNAL "")
    endif (HAVE_TOWLOWER_CTYPE)
endif (NOT HAVE_TOWLOWER)

# Standard packages
macro_optional_find_package (Threads)

# Used (optionally for testing)
find_program(SH_BIN sh)
find_program(BASH_BIN bash)
find_program(SQLITE_BIN sqlite3)
find_program(SED_BIN sed)
if (WIN32)
find_program(MYSQL_BIN mysql $ENV{ProgramFiles}/MySQL/*/bin $ENV{SystemDrive}/MySQL/*/bin "c:/Program Files/MySQL/*/bin")
find_program(PSQL_BIN psql $ENV{ProgramFiles}/PostgreSQL/*/bin $ENV{SystemDrive}/PostgreSQL/*/bin "c:/Program Files/PostgreSQL/*/bin")
else()
find_program(MYSQL_BIN mysql)
find_program(PSQL_BIN psql)
endif()

find_package (Threads)

# Packages in sources
macro_optional_find_package (MySQL)
macro_optional_find_package (ODBC)
macro_optional_find_package (Postgres)
macro_optional_find_package (LibDBI)
macro_optional_find_package (Libintl)
macro_optional_find_package (Iconv)
macro_optional_find_package (CURL)
macro_optional_find_package (Glib)
macro_optional_find_package (GObject)
macro_optional_find_package (SystemD)

# Gudev stuff
pkg_check_modules(GUDEV gudev-1.0)

if (CMAKE_CROSSCOMPILING AND MINGW)
    set (HAVE_WINT_T True CACHE INTERNAL "")
    set (HAVE_WCHAR_T True CACHE INTERNAL "")
else (CMAKE_CROSSCOMPILING AND MINGW)
    # Search for needed includes and functions
    if (HAVE_WCHAR_H)
        set (CMAKE_EXTRA_INCLUDE_FILES wchar.h)
        check_type_size (wchar_t WCHAR_T)
        check_type_size (wint_t  WINT_T)
    endif (HAVE_WCHAR_H)
    set (CMAKE_EXTRA_INCLUDE_FILES)
endif (CMAKE_CROSSCOMPILING AND MINGW)

set (CMAKE_EXTRA_INCLUDE_FILES stdio.h)
check_type_size (ssize_t SSIZE_T)
set (CMAKE_EXTRA_INCLUDE_FILES)
if (HAVE_UNISTD_H)
    set (CMAKE_EXTRA_INCLUDE_FILES unistd.h)
    check_type_size(intptr_t INTPTR_T)
    set (CMAKE_EXTRA_INCLUDE_FILES)
else(HAVE_UNISTD_H)
    check_type_size(intptr_t INTPTR_T)
endif (HAVE_UNISTD_H)

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
#include <time.h>

int main(void) {
    struct tm tm;
	tm.tm_isdst	= daylight;
    return 0;
}
"  HAVE_DAYLIGHT)

check_c_source_compiles ("
#include <stdio.h>

int main(void) {
    printf( __FUNC__);
    return 0;
}
"  HAVE_MACRO_FUNC)


OPTION(WITH_BLUETOOTH "Bluetooth support" ON)
if (WITH_BLUETOOTH)
    if (WIN32 AND NOT CYGWIN)
        # FIXME: This is currently hardcoded here, maybe there is test?
        set(BLUETOOTH_FOUND ON)
        message(STATUS "Using Windows native Bluetooth")
        set(BLUETOOTH_SEARCH TRUE)
    elseif(CYGWIN)
        message(STATUS "Bluetooth is currently not supported in Cygwin")
        message(STATUS "If you need it, contact us")
    else (WIN32 AND NOT CYGWIN)
        find_package (Bluez)
        if (BLUEZ_FOUND)
            set(BLUETOOTH_FOUND ON)
            set(BLUETOOTH_SEARCH TRUE)
            message(STATUS "Using BlueZ stack")
        endif (BLUEZ_FOUND)
        find_package (BSDBluetooth)
        if (BSD_BLUE_FOUND)
            set(BLUETOOTH_FOUND ON)
            message(STATUS "Using BSD Bluetooth stack")
	    check_library_exists(bluetooth sdp_service_search_attribute "" HAVE_SDP_SERVICE_SEARCH_ATTRIBUTE)
	    check_library_exists(bluetooth bt_devinquiry "" HAVE_BT_DEVINQUIRY)
	    if (HAVE_SDP_SERVICE_SEARCH_ATTRIBUTE AND HAVE_BT_DEVINQUIRY)
		set(BLUETOOTH_SEARCH TRUE)
	    endif (HAVE_SDP_SERVICE_SEARCH_ATTRIBUTE AND HAVE_BT_DEVINQUIRY)
        endif (BSD_BLUE_FOUND)
        find_package (OSXBluetooth)
        if (OSX_BLUE_FOUND)
            set(BLUETOOTH_FOUND ON)
            set(BLUETOOTH_SEARCH FALSE)
            message(STATUS "Using OSX Bluetooth stack")
        endif (OSX_BLUE_FOUND)
    endif (WIN32 AND NOT CYGWIN)
else (WITH_BLUETOOTH)
    set(BLUETOOTH_FOUND FALSE)
    set(BLUETOOTH_INCLUDE_DIR)
    set(BLUETOOTH_INCLUDES)
    set(BLUETOOTH_LIBRARY)
    set(BLUETOOTH_LIBRARIES)
endif (WITH_BLUETOOTH)

if (BLUETOOTH_FOUND)
    message(STATUS "Bluetooth support enabled")
else (BLUETOOTH_FOUND)
    if (WITH_BLUETOOTH AND NOT CYGWIN)
        message("Bluetooth support disabled, please install libbluetooth-dev or equivalent to enable Bluetooth.")
    endif (WITH_BLUETOOTH AND NOT CYGWIN)
endif (BLUETOOTH_FOUND)

macro_gammu_option (BLUETOOTH_RF_SEARCHING "Searching for RF channels with Bluetooth stack" ON BLUETOOTH_FOUND BLUETOOTH_SEARCH)
if (WITH_BLUETOOTH_RF_SEARCHING)
    set (BLUETOOTH_RF_SEARCHING ON)
endif (WITH_BLUETOOTH_RF_SEARCHING)

OPTION(WITH_USB "Native USB support" ON)
if (WITH_USB)
    find_package (LibUSB)
endif (WITH_USB)

OPTION(WITH_IRDA "IrDA support" ON)
if (WITH_IRDA)
    if (WIN32 AND NOT CYGWIN)
        # FIXME: This is currently hardcoded here, maybe there is test?
        set (IRDA_FOUND TRUE)
        message(STATUS "Using Windows native IrDA")
    elseif(CYGWIN)
        message(STATUS "IrDA is currently not supported in Cygwin")
        message(STATUS "If you need it, contact us")
        set (IRDA_FOUND FALSE)
    else (WIN32 AND NOT CYGWIN)
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
        if (IRDA_FOUND)
            message(STATUS "Using Linux native IrDA")
        endif (IRDA_FOUND)
    endif (WIN32 AND NOT CYGWIN)
else (WITH_IRDA)
    set (IRDA_FOUND FALSE)
endif (WITH_IRDA)

if (IRDA_FOUND)
    message(STATUS "IrDA support enabled")
endif (IRDA_FOUND)

if (MYSQL_FOUND)
    set (HAVE_MYSQL_MYSQL_H TRUE)
    if (WIN32)
        set(WIN_LIB_MYSQL libmysql.dll)
    endif ()
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
    set (HAVE_POSTGRESQL_LIBPQ_FE_H TRUE)
    if (WIN32)
        set(WIN_LIB_PGSQL libpq.dll SSLEAY32.DLL LIBEAY32.DLL LIBINTL-8.DLL)
    endif ()
endif (POSTGRES_FOUND)

set (GAMMU_LIBS "")

if (BLUEZ_FOUND)
    if (NOT "${BLUEZ_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${BLUEZ_LIBRARIES}")
    endif (NOT "${BLUEZ_LIBRARIES}" STREQUAL "")
endif (BLUEZ_FOUND)

if (BSD_BLUE_FOUND)
    if (NOT "${BSD_BLUE_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${BSD_BLUE_LIBRARIES}")
    endif (NOT "${BSD_BLUE_LIBRARIES}" STREQUAL "")
endif (BSD_BLUE_FOUND)

if (ICONV_FOUND)
    if (NOT "${ICONV_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${ICONV_LIBRARIES}")
    endif (NOT "${ICONV_LIBRARIES}" STREQUAL "")
endif (ICONV_FOUND)

if (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
    set (GAMMU_LIBS "${GAMMU_LIBS} -l${LIBINTL_LIBRARIES}")
endif (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)

if (LIBUSB_FOUND)
    if (NOT "${LIBUSB_LIBRARIES}" STREQUAL "")
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${LIBUSB_LIBRARIES}")
    endif (NOT "${LIBUSB_LIBRARIES}" STREQUAL "")
endif (LIBUSB_FOUND)

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
    # we use old runtime
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    # we use strcpy instead of strcpy_s
    add_definitions(-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1)
    # we use ansi API for winsock
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS=1)
    # The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS=1)

    # Generate PDB data even for release build
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(MATH_LIBS "")

   # Call cmake with "-DMSVC_RUNTIME=static" to link with static runtime libraries
   configure_msvc_runtime()
else(MSVC)
    set(MATH_LIBS "-lm")
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_MINGW OR
    CMAKE_COMPILER_IS_CYGWIN OR CMAKE_COMPILER_IS_GNUCXX OR CLANG_COMPILER)
    # Check for extra compiler flags we want to use
    if (NOT GAMMU_VERSION_PATCH LESS 90 OR "$ENV{TRAVIS}" STREQUAL "true")
        MACRO_TUNE_COMPILER("-Werror")
    endif (NOT GAMMU_VERSION_PATCH LESS 90 OR "$ENV{TRAVIS}" STREQUAL "true")

    #
    # Warnings related flags
    #
    MACRO_TUNE_COMPILER("-O2")
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
    MACRO_TUNE_COMPILER("-Wshadow")
    MACRO_TUNE_COMPILER("-Wstrict-prototypes")
    MACRO_TUNE_COMPILER("-Wsign-compare")
#   We need some typecasting (especially for iconv)
#    MACRO_TUNE_COMPILER("-Wcast-qual")

    # Does not work well with default cases for enums
    MACRO_TUNE_COMPILER("-Wunreachable-code")
    MACRO_TUNE_COMPILER("-Wno-error=unreachable-code")

    # Just silent this for now, too much code to fix here
    MACRO_TUNE_COMPILER("-Wno-pointer-sign")

    MACRO_TUNE_COMPILER("-Wwrite-strings")
    MACRO_TUNE_COMPILER("-Wredundant-decls")
    if (DEBUG)
        MACRO_TUNE_COMPILER("-fstrict-aliasing")
    endif (DEBUG)

    # Check format strings
    MACRO_TUNE_COMPILER("-Wshadow")
    MACRO_TUNE_COMPILER("-Wformat=2")
    MACRO_TUNE_COMPILER("-Wno-format-y2k")
    MACRO_TUNE_COMPILER("-Wno-format-nonliteral")
    # Win32 implementation of gettext does not allow us to use so strict warnings
    if (WIN32 AND NOT CYGWIN)
        MACRO_TUNE_COMPILER("-Wno-format-nonliteral")
        MACRO_TUNE_COMPILER("-Wno-format-security")
    endif (WIN32 AND NOT CYGWIN)

    # Enable extra warnings
    MACRO_TUNE_COMPILER("-Wextra")
    if (NOT CLANG_COMPILER)
        MACRO_TUNE_COMPILER("-Wunused-but-set-variable")
    endif (NOT CLANG_COMPILER)
    # Just silent this for now, too much code to fix here
    MACRO_TUNE_COMPILER("-Wno-unused-parameter")

    #
    # Security related flags
    #
    if (NOT CLANG_COMPILER)
        set (ENABLE_PROTECTION ON CACHE BOOL "Enable compile time protections (stack, read only code, fortify in libc...)")
    endif (NOT CLANG_COMPILER)
    if (ENABLE_PROTECTION)
        # Need to enable -O to make fortify work
        set (CMAKE_C_FLAGS_DEBUG "-g -O")
        add_definitions(-D_FORTIFY_SOURCE=2)

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

    if (WIN32 AND NOT CYGWIN AND BUILD_SHARED_LIBS)
        MACRO_TUNE_LINKER("-Wl,--enable-auto-import")
    endif (WIN32 AND NOT CYGWIN AND BUILD_SHARED_LIBS)

    MACRO_TUNE_LINKER("-Wl,--as-needed")
    if (NOT WIN32)
        set (PIC ON CACHE BOOL "Compile PIC code")
        if (PIC)
            MACRO_TUNE_COMPILER("-fPIC")
        endif (PIC)
    endif (NOT WIN32)
    # Specs for MinGW to change msvcrt
    # This is not supported by MinGW and is broken currently
#    if (CMAKE_COMPILER_IS_MINGW)
#        MACRO_TUNE_COMPILER("-specs=${CMAKE_CURRENT_SOURCE_DIR}/cmake/mingw.spec")
#        add_definitions(-D_MSVCRT_VERSION__=0x090)
#    endif (CMAKE_COMPILER_IS_MINGW)
endif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_MINGW OR
    CMAKE_COMPILER_IS_CYGWIN OR CMAKE_COMPILER_IS_GNUCXX OR CLANG_COMPILER)

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
    string(REGEX REPLACE "-l(/usr)?(/local)?/lib(64)?/lib([^ ]*).so" "-l\\4" G_LIB "${GAMMU_LIBS}")
    string(REGEX REPLACE "-l(/usr)?(/local)?/lib(64)?/lib([^ ]*).so" "-l\\4" S_LIB "${SMSD_LIBS}")
    set (GAMMU_PRIVATE_LIBS "${G_LIB}" CACHE INTERNAL "Private libraries for gammu library")
    set (SMSD_PRIVATE_LIBS "${S_LIB}" CACHE INTERNAL "Private libraries for gammu smsd library")
endif (BUILD_SHARED_LIBS)

set (DOCS
    README.rst
    ChangeLog
    COPYING
    )

if (CMAKE_USE_PTHREADS_INIT)
    set (HAVE_PTHREAD ON)
endif (CMAKE_USE_PTHREADS_INIT)

if (UNIX)
    find_library (MATH_LIBRARIES m)

    # Install paths
    if ("${MATH_LIBRARIES}" MATCHES "/lib64/")
        set (LIB_SUFFIX "64" CACHE STRING "Suffix of library directory (usually 64 or empty)")
    else ("${MATH_LIBRARIES}" MATCHES "/lib64/")
        set (LIB_SUFFIX "" CACHE STRING "Suffix of library directory (usually 64 or empty)")
    endif ("${MATH_LIBRARIES}" MATCHES "/lib64/")
endif (UNIX)


set (INSTALL_BIN_DIR "bin" CACHE STRING "Path for binaries installation")
mark_as_advanced (INSTALL_BIN_DIR)

set (INSTALL_LIB_DIR "lib${LIB_SUFFIX}" CACHE STRING "Path for libraries installation")
mark_as_advanced (INSTALL_LIB_DIR)

set (INSTALL_LIBDATA_DIR "lib${LIB_SUFFIX}" CACHE STRING "Path for libraries data (eg. pkgconfig data) installation")
mark_as_advanced (INSTALL_LIBDATA_DIR)

set (INSTALL_INC_DIR "include/gammu" CACHE STRING "Path for includes installation")
mark_as_advanced (INSTALL_INC_DIR)

set (INSTALL_LOC_DIR "share/locale" CACHE STRING "Path for locales installation")
mark_as_advanced (INSTALL_LOC_DIR)

set (INSTALL_DATA_DIR "share/gammu" CACHE STRING "Path for data installation")
mark_as_advanced (INSTALL_DATA_DIR)

set (INSTALL_DOC_DIR "share/doc/gammu" CACHE STRING "Path for documentation installation")
mark_as_advanced (INSTALL_DOC_DIR)

set (INSTALL_MAN_DIR "share/man" CACHE STRING "Path for man pages installation")
mark_as_advanced (INSTALL_MAN_DIR)

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
macro_gammu_option (ATOBEX_AUTO_MODE "Automatic switching to OBEX for AT+MODE capable phones" OFF WITH_ATOBEX_SUPPORT ON)

macro_gammu_option (S60_SUPPORT "Symbian phones support (S60)" ON ON ON)
macro_gammu_option (GNAPPLET_SUPPORT "Symbian phones support (gnapplet)" ON WITH_NOKIA_SUPPORT ON)
macro_gammu_option (BLUEGNAPBUS "GNAPBUS protocol over Bluetooth" ON WITH_GNAPPLET_SUPPORT BLUETOOTH_FOUND)
macro_gammu_option (IRDAGNAPBUS "GNAPBUS protocol over IrDA" ON WITH_GNAPPLET_SUPPORT IRDA_FOUND)

macro_gammu_option (GNAPGEN "Gnapplet phones support" ON WITH_GNAPPLET_SUPPORT ON)

macro_gammu_option (S60 "S60 phones support" ON WITH_S60_SUPPORT ON)

# Some generic configurations
macro_gammu_option (CELLBROADCAST "Cell Broadcast messages support" ON ON ON)
macro_gammu_option (BACKUP "Backup/Restore functions" ON ON ON)

# Generate Doxygen file
set (DOXYGEN_INPUT "${CMAKE_CURRENT_BINARY_DIR}/include")
set (DOXYGEN_INTERNAL_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/libgammu")
set (DOXYGEN_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gammu-doc")
set (DOXYGEN_INTERNAL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gammu-internal-doc")
if ("${DOXYGEN_DOT_EXECUTABLE}" STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "NO")
    set (DOXYGEN_DOT_PATH "")
else ("${DOXYGEN_DOT_EXECUTABLE}" STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
    set (HAVE_DOT "YES")
    # Strip binary name from variable
    string (REGEX REPLACE "/dot$" "" DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}")
endif ("${DOXYGEN_DOT_EXECUTABLE}" STREQUAL DOXYGEN_DOT_EXECUTABLE-NOTFOUND)
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/Doxyfile.cmake" "${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/Doxyfile-internal.cmake" "${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile-internal")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/api.desc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/doxygen/api.desc")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/internals.desc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/doxygen/internals.desc")

# Target for generating API documentation
add_custom_target (apidoc ${DOXYGEN_EXECUTABLE} doxygen/Doxyfile
    COMMENT "Generating API documentation")

add_custom_target (interndoc ${DOXYGEN_EXECUTABLE} doxygen/Doxyfile-internal
    COMMENT "Generating internal documentation")

# CPP check
find_program(CPPCHECK_EXECUTABLE NAMES cppcheck
    DOC "cppcheck - code checking tool")
add_custom_target (cppcheck ${CPPCHECK_EXECUTABLE} -a -f -q "${CMAKE_CURRENT_SOURCE_DIR}/gammu" "${CMAKE_CURRENT_SOURCE_DIR}/smsd" "${CMAKE_CURRENT_SOURCE_DIR}/helper" "${CMAKE_CURRENT_SOURCE_DIR}/libgammu" "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}/docs/manual/examples" "${CMAKE_CURRENT_SOURCE_DIR}/contrib/smscgi"
    COMMENT "Checking C code")


# Generate pkgconfig file
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gammu.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/cfg/gammu.pc" @ONLY)
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gammu-smsd.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/cfg/gammu-smsd.pc" @ONLY)
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gammu-uninstalled.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/gammu.pc" @ONLY)
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/gammu-smsd-uninstalled.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/gammu-smsd.pc" @ONLY)

# Install instructions

# Install Windows libraries
if (WIN32 AND NOT CYGWIN)
    # MySQL
    if (MYSQL_FOUND)
        foreach(lib ${WIN_LIB_MYSQL})
            ADD_DLL("${lib}" "${MYSQL_LIBRARIES}")
        endforeach(lib ${WIN_LIB_MYSQL})
    endif (MYSQL_FOUND)

    # PostgreSQL and dependencies
    if (POSTGRES_FOUND)
        foreach(lib ${WIN_LIB_PGSQL})
            ADD_DLL("${lib}" "${POSTGRES_LIBRARY}")
        endforeach(lib ${WIN_LIB_PGSQL})
    endif (POSTGRES_FOUND)

    # iconv and dependencies
    if (ICONV_FOUND)
        foreach(lib ${WIN_LIB_ICONV})
            ADD_DLL("${lib}" "${ICONV_LIBRARIES}")
        endforeach(lib ${WIN_LIB_ICONV})
    endif (ICONV_FOUND)

    # Gettext and dependencies
    if (LIBINTL_LIB_FOUND)
        foreach(lib ${WIN_LIB_INTL})
            ADD_DLL("${lib}" "${LIBINTL_LIBRARY}")
        endforeach(lib ${WIN_LIB_INTL})
    endif (LIBINTL_LIB_FOUND)

    # Curl and dependencies
    if (CURL_FOUND)
        foreach(lib ${WIN_LIB_CURL})
            ADD_DLL("${lib}" "${CURL_LIBRARIES}")
        endforeach(lib ${WIN_LIB_CURL})
    endif (CURL_FOUND)

    # Glib and dependencies
    if (Glib_FOUND)
        foreach(lib ${WIN_LIB_GLIB})
            ADD_DLL("${lib}" "${Glib_LIBRARIES}")
        endforeach(lib ${WIN_LIB_GLIB})
    endif (Glib_FOUND)

    install (FILES ${WIN32_INSTALL_DLL}
            DESTINATION "${INSTALL_BIN_DIR}"
            COMPONENT dlls
            )
endif (WIN32 AND NOT CYGWIN)

install (
    FILES "${CMAKE_CURRENT_BINARY_DIR}/cfg/gammu.pc" "${CMAKE_CURRENT_BINARY_DIR}/cfg/gammu-smsd.pc"
    DESTINATION "${INSTALL_LIBDATA_DIR}/pkgconfig"
    COMPONENT "pkgconfig"
    )

# We want some tests. Even when cross compiling for Windows tests can be
# done in Wine. This is better than nothing, even though Wine is a bit
# more fault tolerant when linking DLLs.

# Testing and dashboard
include(CTest)
enable_testing()
include(Dart)

# Packaging support
set (CPACK_PACKAGE_NAME "Gammu")
set (CPACK_PACKAGE_VERSION "${GAMMU_VERSION}")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gammu All Mobile Management Utilities ${GAMMU_VERSION}")
if (WIN32)
    set (CPACK_PACKAGE_VENDOR "Michal Cihar")
    set (CPACK_PACKAGE_CONTACT "Michal Cihar <michal@cihar.com>")
else ()
    set (CPACK_PACKAGE_VENDOR "Michal Čihař")
    set (CPACK_PACKAGE_CONTACT "Michal Čihař <michal@cihar.com>")
endif ()
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/installer/description.txt")
# On Mac OS, the file needs .txt extension, give it to them
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/README.rst" "${CMAKE_CURRENT_BINARY_DIR}/readme.txt" COPYONLY)
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_BINARY_DIR}/readme.txt")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/installer/license.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${GAMMU_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${GAMMU_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${GAMMU_VERSION_PATCH}")

if (WIN32 AND NOT CYGWIN)
    set (CPACK_SYSTEM_NAME "Windows" CACHE STRING "Windows package name")
endif (WIN32 AND NOT CYGWIN)

# RPM
set (CPACK_RPM_PACKAGE_LICENSE "GPLv2")
if (EXISTS "/etc/SuSE-release")
set (CPACK_RPM_PACKAGE_GROUP "Hardware/Mobile")
else (EXISTS "/etc/SuSE-release")
set (CPACK_RPM_PACKAGE_GROUP "Applications/Communications")
endif (EXISTS "/etc/SuSE-release")

# Components
set (CPACK_COMPONENTS_ALL gammu library smsd headers utils manpages docs pkgconfig symbian examples manual)
if (WIN32 AND NOT CYGWIN AND WIN32_INSTALL_DLL)
    list (APPEND CPACK_COMPONENTS_ALL dlls)
endif (WIN32 AND NOT CYGWIN AND WIN32_INSTALL_DLL)

set (CPACK_COMPONENT_GAMMU_DISPLAY_NAME "Gammu Application")
set (CPACK_COMPONENT_SMSD_DISPLAY_NAME "Gammu SMS Daemon")
set (CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "Gammu Library")
set (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C Headers")
set (CPACK_COMPONENT_UTILS_DISPLAY_NAME "Utility Scripts")
set (CPACK_COMPONENT_MANPAGES_DISPLAY_NAME "Manual Pages")
set (CPACK_COMPONENT_DOCS_DISPLAY_NAME "User Documentation")
set (CPACK_COMPONENT_PKGCONFIG_DISPLAY_NAME "Pkg-config Data")
set (CPACK_COMPONENT_SYMBIAN_DISPLAY_NAME "Applet for Symbian Phones")
set (CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Using Gammu")
set (CPACK_COMPONENT_MANUAL_DISPLAY_NAME "Gammu Manual")
set (CPACK_COMPONENT_MEDIA_DISPLAY_NAME "Media Files")

set (CPACK_COMPONENT_DLLS_HIDDEN TRUE)
set (CPACK_COMPONENT_DLLS_REQUIRED TRUE)
set (CPACK_COMPONENT_LIBRARY_REQUIRED TRUE)

set (CPACK_COMPONENT_GAMMU_DESCRIPTION "The gammu.exe program.")
set (CPACK_COMPONENT_SMSD_DESCRIPTION "Gammu SMS Daemon which automatically sends and receives messages.")
set (CPACK_COMPONENT_LIBRARY_DESCRIPTION "Gammu library.")
set (CPACK_COMPONENT_HEADERS_DESCRIPTION "C headers for development own programs using Gammu library.")
set (CPACK_COMPONENT_UTILS_DESCRIPTION "Utility scripts, mostly written using bourne shell, which you need to install on your own.")
set (CPACK_COMPONENT_MANPAGES_DESCRIPTION "Manual pages for programs and configuration files.")
set (CPACK_COMPONENT_DOCS_DESCRIPTION "User documentation in text files.")
set (CPACK_COMPONENT_PKGCONFIG_DESCRIPTION "Data for pkg-config program (automatic finding of compiler parameters for libraries).")
set (CPACK_COMPONENT_SYMBIAN_DESCRIPTION "Applet for Symbian phones, install it to phone to communicate with them using Gammu.")
set (CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Examples showing how to use Gammu, in PHP, Perl or C.")
set (CPACK_COMPONENT_MANUAL_DESCRIPTION "User and Developer Manual")
set (CPACK_COMPONENT_MEDIA_DESCRIPTION "Various media files which can be used with Gammu.")

set (CPACK_COMPONENT_GROUP_RUNTIME_DESCRIPTION "Programs and libraries for general usage.")
set (CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "Headers and documentation used for development using Gammu.")
set (CPACK_COMPONENT_GROUP_DOCUMENTATION_DESCRIPTION "Gammu documentation and usage examples.")

set(CPACK_COMPONENT_GAMMU_DEPENDS library)
if (WIN32 AND NOT CYGWIN AND WIN32_INSTALL_DLL)
set(CPACK_COMPONENT_LIBRARY_DEPENDS dlls)
endif (WIN32 AND NOT CYGWIN AND WIN32_INSTALL_DLL)
set(CPACK_COMPONENT_SMSD_DEPENDS library)
set(CPACK_COMPONENT_HEADERS_DEPENDS library)
set(CPACK_COMPONENT_UTILS_DEPENDS gammu)
set(CPACK_COMPONENT_PKGCONFIG_DEPENDS headers)

set (CPACK_COMPONENT_GAMMU_GROUP "Runtime")
set (CPACK_COMPONENT_SMSD_GROUP "Runtime")
set (CPACK_COMPONENT_UTILS_GROUP "Runtime")
set (CPACK_COMPONENT_LIBRARY_GROUP "Runtime")

set (CPACK_COMPONENT_HEADERS_GROUP "Development")
set (CPACK_COMPONENT_PKGCONFIG_GROUP "Development")

set (CPACK_COMPONENT_MANPAGES_GROUP "Documentation")
set (CPACK_COMPONENT_DOCS_GROUP "Documentation")
set (CPACK_COMPONENT_EXAMPLES_GROUP "Documentation")
set (CPACK_COMPONENT_MANUAL_GROUP "Documentation")
set (CPACK_COMPONENT_MEDIA_GROUP "Documentation")

set (CPACK_ALL_INSTALL_TYPES Full Standard Developer)

set (CPACK_COMPONENT_GAMMU_INSTALL_TYPES Standard Developer Full)
set (CPACK_COMPONENT_SMSD_INSTALL_TYPES Standard Developer Full)
set (CPACK_COMPONENT_LIBRARY_INSTALL_TYPES Standard Developer Full)
set (CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full)
set (CPACK_COMPONENT_UTILS_INSTALL_TYPES Full)
set (CPACK_COMPONENT_MANPAGES_INSTALL_TYPES Standard Developer Full)
set (CPACK_COMPONENT_DOCS_INSTALL_TYPES Standard Developer Full)
set (CPACK_COMPONENT_PKGCONFIG_INSTALL_TYPES Developer Full)
set (CPACK_COMPONENT_SYMBIAN_INSTALL_TYPES Standard Developer Full)
set (CPACK_COMPONENT_EXAMPLES_INSTALL_TYPES Developer Full)
set (CPACK_COMPONENT_MANUAL_INSTALL_TYPES Developer Full)
set (CPACK_COMPONENT_MEDIA_INSTALL_TYPES Full)

if (WIN32 AND NOT CYGWIN)
  set (CPACK_PACKAGE_INSTALL_DIRECTORY "Gammu ${GAMMU_VERSION}")
  set (CPACK_NSIS_DISPLAY_NAME "Gammu ${GAMMU_VERSION}")
  set (CPACK_NSIS_HELP_LINK "https://wammu.eu/support/")
  set (CPACK_NSIS_URL_INFO_ABOUT "https://wammu.eu/gammu/")
  set (CPACK_NSIS_CONTACT "michal@cihar.com")
  set (CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		    ExecWait  \\\"$INSTDIR\\\\bin\\\\gammu-smsd.exe -e \\\"
            ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
		    ExecWait  \\\"$INSTDIR\\\\bin\\\\gammu-smsd.exe -E \\\"
            ")
else (WIN32 AND NOT CYGWIN)
  set (CPACK_STRIP_FILES "gammu")
  set (CPACK_SOURCE_STRIP_FILES "")
endif (WIN32 AND NOT CYGWIN)
include (CPack)

# Test files for Gammu
set (RINGTONE_TEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/contrib/media/axelf.txt)
set (LOGO_TEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/contrib/media/aliens.nlm)

# Transverse to dirs
add_subdirectory(include)

add_subdirectory(libgammu)

add_subdirectory(helper)

add_subdirectory(tests)

add_subdirectory(smsd)

add_subdirectory(gammu)

add_subdirectory(gammu-detect)

add_subdirectory(locale)

add_subdirectory(utils)

option(INSTALL_DOC "Install documentation" ON)

if(INSTALL_DOC)
    install (
        FILES ${DOCS}
        DESTINATION "${INSTALL_DOC_DIR}"
        COMPONENT "docs"
        )

    add_subdirectory(docs/config)
    add_subdirectory(docs/manual)
    add_subdirectory(docs/examples)
    add_subdirectory(docs/sql)
    add_subdirectory(docs/man)
endif(INSTALL_DOC)

add_subdirectory(contrib)

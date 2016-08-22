# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-helper C)

include(GammuTuneFlags)

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}/../include"
    )

add_library(string STATIC string.c)
add_coverage (string)
GAMMU_TUNE_SHARED(string)

add_library(array STATIC array.c)
add_coverage (array)
GAMMU_TUNE_SHARED(array)

add_library (printing STATIC printing.c)
add_coverage (printing)
GAMMU_TUNE_SHARED(printing)

add_library (cmdline STATIC cmdline.c)
add_coverage (cmdline)
GAMMU_TUNE_SHARED(cmdline)
target_link_libraries (cmdline printing)

add_library (memorydisplay STATIC memory-display.c)
add_coverage (memorydisplay)
GAMMU_TUNE_SHARED(memorydisplay)
target_link_libraries (memorydisplay libGammu)

add_library (messagecmdline STATIC message-cmdline.c)
add_coverage (messagecmdline)
GAMMU_TUNE_SHARED(messagecmdline)
target_link_libraries (messagecmdline libGammu)
target_link_libraries (messagecmdline printing)
target_link_libraries (messagecmdline string)
target_link_libraries (messagecmdline cmdline)

add_library (messagedisplay STATIC message-display.c)
add_coverage (messagedisplay)
GAMMU_TUNE_SHARED(messagedisplay)
target_link_libraries (messagedisplay libGammu)
target_link_libraries (messagedisplay memorydisplay)

# Tune options for found libraries
if (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
    target_link_libraries (messagedisplay ${LIBINTL_LIBRARIES})
    target_link_libraries (printing ${LIBINTL_LIBRARIES})
    include_directories (${LIBINTL_INCLUDE_DIR})
endif (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)

if(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)
    add_library(win32dirent STATIC win32-dirent.c)
    add_coverage (win32dirent)
endif(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)

if (NOT HAVE_STRPTIME)
    add_library(strptime STATIC strptime.c)
    add_coverage (strptime)
    GAMMU_TUNE_SHARED(strptime)
    target_link_libraries (strptime string)
endif (NOT HAVE_STRPTIME)

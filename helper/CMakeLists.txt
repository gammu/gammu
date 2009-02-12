# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-helper C)

include(GammuTuneCompiler)

include_directories (
    "${Gammu_BINARY_DIR}/include"
    )

add_library (printing STATIC printing.c)

add_library (memorydisplay STATIC memory-display.c)
target_link_libraries (memorydisplay libGammu)

add_library (messagecmdline STATIC message-cmdline.c)
target_link_libraries (messagecmdline libGammu)
target_link_libraries (messagecmdline printing)

add_library (messagedisplay STATIC message-display.c)
target_link_libraries (messagedisplay libGammu)
target_link_libraries (messagedisplay memorydisplay)

# Tune options for found libraries
if (GETTEXTLIBS_FOUND)
    target_link_libraries (messagedisplay ${GETTEXT_LIBRARIES})
    target_link_libraries (printing ${GETTEXT_LIBRARIES})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

if(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)
    add_library(win32dirent STATIC win32-dirent.c)
endif(WIN32 AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)

if (NOT HAVE_STRPTIME)
    add_library(strptime STATIC strptime.c)
endif (NOT HAVE_STRPTIME)

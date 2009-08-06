# - Finds Bluetooth library on BSD
# This module defines
#  BSD_BLUE_INCLUDE_DIR, where to find bluetooth.h
#  BSD_BLUE_LIBRARIES, the libraries needed to use BSD Bluetooth.
#  BSD_BLUE_FOUND, If false, do not try to use BSD Bluetooth.
#
# Copyright (c) 2007, Michal Cihar, <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:

if (NOT DEFINED BSD_BLUE_FOUND)
    if (NOT CROSS_MINGW)
        find_path(BSD_BLUE_INCLUDE_DIR NAMES bluetooth.h
           PATHS
           /usr/include
           /usr/local/include
        )

        find_library(BSD_BLUE_LIBRARIES NAMES bluetooth
           PATHS
           /usr/lib
           /usr/local/lib
        )

        if(BSD_BLUE_INCLUDE_DIR AND BSD_BLUE_LIBRARIES)
           set(BSD_BLUE_FOUND TRUE CACHE INTERNAL "BSD Bluetooth found")
           message(STATUS "Found BSD Bluetooth: ${BSD_BLUE_INCLUDE_DIR}, ${BSD_BLUE_LIBRARIES}")
        else(BSD_BLUE_INCLUDE_DIR AND BSD_BLUE_LIBRARIES)
           set(BSD_BLUE_FOUND FALSE CACHE INTERNAL "BSD Bluetooth found")
           message(STATUS "BSD Bluetooth not found.")
        endif(BSD_BLUE_INCLUDE_DIR AND BSD_BLUE_LIBRARIES)

        mark_as_advanced(BSD_BLUE_INCLUDE_DIR BSD_BLUE_LIBRARIES)
    endif (NOT CROSS_MINGW)
endif (NOT DEFINED BSD_BLUE_FOUND)

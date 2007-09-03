# - Finds Bluetooth library on FreeBSD
# This module defines
#  FBSD_BLUE_INCLUDE_DIR, where to find bluetooth.h
#  FBSD_BLUE_LIBRARIES, the libraries needed to use FreeBSD Bluetooth.
#  FBSD_BLUE_FOUND, If false, do not try to use FreeBSD Bluetooth.
#
# Copyright (c) 2007, Michal Cihar, <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:

if (NOT DEFINED FBSD_BLUE_FOUND)
    if (NOT CROSS_MINGW)
        find_path(FBSD_BLUE_INCLUDE_DIR NAMES bluetooth.h
           PATHS
           /usr/include
           /usr/local/include
        )

        find_library(FBSD_BLUE_LIBRARIES NAMES bluetooth
           PATHS
           /usr/lib
           /usr/local/lib
        )

        if(FBSD_BLUE_INCLUDE_DIR AND FBSD_BLUE_LIBRARIES)
           set(FBSD_BLUE_FOUND TRUE CACHE INTERNAL "FreeBSD Bluetooth found")
           message(STATUS "Found FreeBSD Bluetooth: ${FBSD_BLUE_INCLUDE_DIR}, ${FBSD_BLUE_LIBRARIES}")
        else(FBSD_BLUE_INCLUDE_DIR AND FBSD_BLUE_LIBRARIES)
           set(FBSD_BLUE_FOUND FALSE CACHE INTERNAL "FreeBSD Bluetooth found")
           message(STATUS "FreeBSD Bluetooth not found.")
        endif(FBSD_BLUE_INCLUDE_DIR AND FBSD_BLUE_LIBRARIES)

        mark_as_advanced(FBSD_BLUE_INCLUDE_DIR FBSD_BLUE_LIBRARIES)
    endif (NOT CROSS_MINGW)
endif (NOT DEFINED FBSD_BLUE_FOUND)

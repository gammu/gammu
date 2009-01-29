# - Find Bluez library
# This module defines
#  BLUEZ_INCLUDE_DIR, where to find bluetooth.h
#  BLUEZ_LIBRARIES, the libraries needed to use Bluez.
#  BLUEZ_FOUND, If false, do not try to use Bluez.
#
# Copyright (c) 2007, Michal Cihar, <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:

if (NOT BLUEZ_FOUND)
    pkg_check_modules (BLUEZ_PKG bluez)

    find_path(BLUEZ_INCLUDE_DIR NAMES bluetooth/bluetooth.h
       PATHS
       ${BLUEZ_PKG_INCLUDE_DIRS}
       /usr/include
       /usr/local/include
    )

    find_library(BLUEZ_LIBRARIES NAMES bluetooth
       PATHS
       ${BLUEZ_PKG_LIBBRARY_DIRS}
       /usr/lib
       /usr/local/lib
    )

    if(BLUEZ_INCLUDE_DIR AND BLUEZ_LIBRARIES)
       set(BLUEZ_FOUND TRUE CACHE INTERNAL "Bluez found")
       message(STATUS "Found Bluez: ${BLUEZ_INCLUDE_DIR}, ${BLUEZ_LIBRARIES}")
    else(BLUEZ_INCLUDE_DIR AND BLUEZ_LIBRARIES)
       set(BLUEZ_FOUND FALSE CACHE INTERNAL "Bluez found")
       message(STATUS "Bluez not found.")
    endif(BLUEZ_INCLUDE_DIR AND BLUEZ_LIBRARIES)

    mark_as_advanced(BLUEZ_INCLUDE_DIR BLUEZ_LIBRARIES)
endif (NOT BLUEZ_FOUND)

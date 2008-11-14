# - Finds Bluetooth library on OS X
# This module defines
#  OSX_BLUE_INCLUDE_DIR, where to find bluetooth.h
#  OSX_BLUE_LIBRARIES, the libraries needed to use OS X Bluetooth.
#  OSX_BLUE_FOUND, If false, do not try to use OS X Bluetooth.
#
# Copyright (c) 2007, Michal Cihar, <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:

if (NOT DEFINED OSX_BLUE_FOUND)
    if (NOT CROSS_MINGW)
        find_path(OSX_BLUE_INCLUDE_DIR NAMES IOBluetooth/IOBluetoothUserLib.h
           PATHS
           /usr/include
           /usr/local/include
        )

        find_library(OSX_COREFOUNDATION_LIB NAMES CoreFoundation
           PATHS
           /usr/lib
           /usr/local/lib
        )
        find_library(OSX_BLUETOOTH_LIB NAMES IOBluetooth
           PATHS
           /usr/lib
           /usr/local/lib
        )
        find_library(OSX_FOUNDATION_LIB NAMES Foundation
           PATHS
           /usr/lib
           /usr/local/lib
        )

        if (OSX_BLUETOOTH_LIB AND OSX_COREFOUNDATION_LIB AND OSX_FOUNDATION_LIB) 
            set (OSX_BLUE_LIBRARIES "${OSX_BLUETOOTH_LIB} ${OSX_COREFOUNDATION_LIB} ${OSX_FOUNDATION_LIB}" CACHE INTERNAL "OS X Bluetooth libraries")
        endif (OSX_BLUETOOTH_LIB AND OSX_COREFOUNDATION_LIB AND OSX_FOUNDATION_LIB) 


        if(OSX_BLUE_INCLUDE_DIR AND OSX_BLUE_LIBRARIES)
           set(OSX_BLUE_FOUND TRUE CACHE INTERNAL "OS X Bluetooth found")
           message(STATUS "Found OS X Bluetooth: ${OSX_BLUE_INCLUDE_DIR}, ${OSX_BLUE_LIBRARIES}")
        else(OSX_BLUE_INCLUDE_DIR AND OSX_BLUE_LIBRARIES)
           set(OSX_BLUE_FOUND FALSE CACHE INTERNAL "OS X Bluetooth found")
           message(STATUS "OS X Bluetooth not found.")
        endif(OSX_BLUE_INCLUDE_DIR AND OSX_BLUE_LIBRARIES)

        CHECK_LIBRARY_EXISTS(${OSX_BLUETOOTH_LIB} "IOBluetoothDeviceOpenRFCOMMChannelSync" "" OSX_BLUE_2_0)

        mark_as_advanced(OSX_BLUE_INCLUDE_DIR OSX_BLUE_LIBRARIES)
    endif (NOT CROSS_MINGW)
endif (NOT DEFINED OSX_BLUE_FOUND)

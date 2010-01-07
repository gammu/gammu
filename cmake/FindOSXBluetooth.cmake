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
            /System/Library/Frameworks/IOBluetooth.framework/Headers/
            )

        find_library(OSX_BLUETOOTH_LIB NAMES IOBluetooth
            PATHS
            /System/Library/Frameworks/IOBluetooth.framework/
            )

        INCLUDE(CMakeFindFrameworks)

        CMAKE_FIND_FRAMEWORKS(CoreFoundation)
        CMAKE_FIND_FRAMEWORKS(IOBluetooth)
        CMAKE_FIND_FRAMEWORKS(Foundation)

        if (CoreFoundation_FRAMEWORKS)
           set(OSX_COREFOUNDATION_LIB "-framework CoreFoundation" CACHE FILEPATH "CoreFoundation framework" FORCE)
           set(OSX_COREFOUNDATION_FOUND 1)
        endif (CoreFoundation_FRAMEWORKS)

        if (Foundation_FRAMEWORKS)
           set(OSX_FOUNDATION_LIB "-framework Foundation" CACHE FILEPATH "Foundation framework" FORCE)
           set(OSX_FOUNDATION_FOUND 1)
        endif (Foundation_FRAMEWORKS)

        if (IOBluetooth_FRAMEWORKS)
           set(OSX_IOBLUETOOTH_LIB "-framework IOBluetooth" CACHE FILEPATH "IOBluetooth framework" FORCE)
           set(OSX_IOBLUETOOTH_FOUND 1)
        endif (IOBluetooth_FRAMEWORKS)

        if (CoreFoundation_FRAMEWORKS AND Foundation_FRAMEWORKS AND IOBluetooth_FRAMEWORKS)
            set (OSX_BLUE_LIBS "${OSX_IOBLUETOOTH_LIB} ${OSX_COREFOUNDATION_LIB} ${OSX_FOUNDATION_LIB}" CACHE INTERNAL "OS X Bluetooth libraries")
        endif (CoreFoundation_FRAMEWORKS AND Foundation_FRAMEWORKS AND IOBluetooth_FRAMEWORKS)

        if(OSX_BLUE_INCLUDE_DIR AND OSX_BLUETOOTH_LIB)
           set(OSX_BLUE_FOUND TRUE CACHE INTERNAL "OS X Bluetooth found")
           message(STATUS "Found OS X Bluetooth: ${OSX_BLUE_INCLUDE_DIR}, ${OSX_BLUETOOTH_LIB}")
        else(OSX_BLUE_INCLUDE_DIR AND OSX_BLUETOOTH_LIB)
           set(OSX_BLUE_FOUND FALSE CACHE INTERNAL "OS X Bluetooth found")
           message(STATUS "OS X Bluetooth not found.")
        endif(OSX_BLUE_INCLUDE_DIR AND OSX_BLUETOOTH_LIB)

        if(OSX_BLUETOOTH_LIB)
            CHECK_LIBRARY_EXISTS(${OSX_BLUETOOTH_LIB} "IOBluetoothDeviceOpenRFCOMMChannelSync" "" OSX_BLUE_2_0)
        endif(OSX_BLUETOOTH_LIB)

        mark_as_advanced(OSX_BLUE_INCLUDE_DIR OSX_BLUETOOTH_LIB)
    endif (NOT CROSS_MINGW)
endif (NOT DEFINED OSX_BLUE_FOUND)

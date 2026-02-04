# Install script for directory: /home/runner/work/gammu/gammu/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "headers" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gammu" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/include/gammu-backup.h"
    "/home/runner/work/gammu/gammu/include/gammu-bitmap.h"
    "/home/runner/work/gammu/gammu/include/gammu-calendar.h"
    "/home/runner/work/gammu/gammu/include/gammu-call.h"
    "/home/runner/work/gammu/gammu/include/gammu-callback.h"
    "/home/runner/work/gammu/gammu/include/gammu-category.h"
    "/home/runner/work/gammu/gammu/include/gammu-datetime.h"
    "/home/runner/work/gammu/gammu/include/gammu-debug.h"
    "/home/runner/work/gammu/gammu/include/gammu-error.h"
    "/home/runner/work/gammu/gammu/include/gammu-file.h"
    "/home/runner/work/gammu/gammu/include/gammu-info.h"
    "/home/runner/work/gammu/gammu/include/gammu-inifile.h"
    "/home/runner/work/gammu/gammu/include/gammu-keys.h"
    "/home/runner/work/gammu/gammu/include/gammu-limits.h"
    "/home/runner/work/gammu/gammu/include/gammu-memory.h"
    "/home/runner/work/gammu/gammu/include/gammu-message.h"
    "/home/runner/work/gammu/gammu/include/gammu-misc.h"
    "/home/runner/work/gammu/gammu/include/gammu-nokia.h"
    "/home/runner/work/gammu/gammu/include/gammu-ringtone.h"
    "/home/runner/work/gammu/gammu/include/gammu-security.h"
    "/home/runner/work/gammu/gammu/include/gammu-settings.h"
    "/home/runner/work/gammu/gammu/include/gammu-smsd.h"
    "/home/runner/work/gammu/gammu/include/gammu-statemachine.h"
    "/home/runner/work/gammu/gammu/include/gammu-types.h"
    "/home/runner/work/gammu/gammu/include/gammu-unicode.h"
    "/home/runner/work/gammu/gammu/include/gammu-wap.h"
    "/home/runner/work/gammu/gammu/include/gammu.h"
    "/home/runner/work/gammu/gammu/_build/include/gammu-config.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/gammu/gammu/_build/include/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

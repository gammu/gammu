# Install script for directory: /home/runner/work/gammu/gammu/docs/man

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "manpages" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/docs/man/gammu-config.1"
    "/home/runner/work/gammu/gammu/docs/man/gammu-detect.1"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-inject.1"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-monitor.1"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd.1"
    "/home/runner/work/gammu/gammu/docs/man/gammu.1"
    "/home/runner/work/gammu/gammu/docs/man/jadmaker.1"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "manpages" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man5" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/docs/man/gammu-backup.5"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsbackup.5"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsdrc.5"
    "/home/runner/work/gammu/gammu/docs/man/gammurc.5"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "manpages" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man7" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-dbi.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-files.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-mysql.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-null.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-odbc.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-pgsql.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-run.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-sql.7"
    "/home/runner/work/gammu/gammu/docs/man/gammu-smsd-tables.7"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/gammu/gammu/_codeql_build_dir/docs/man/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

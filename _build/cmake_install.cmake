# Install script for directory: /home/runner/work/gammu/gammu

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

if(CMAKE_INSTALL_COMPONENT STREQUAL "pkgconfig" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/_build/cfg/gammu.pc"
    "/home/runner/work/gammu/gammu/_build/cfg/gammu-smsd.pc"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "docs" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/gammu" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/README.rst"
    "/home/runner/work/gammu/gammu/ChangeLog"
    "/home/runner/work/gammu/gammu/COPYING"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/runner/work/gammu/gammu/_build/include/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/libgammu/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/helper/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/tests/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/smsd/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/gammu/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/gammu-detect/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/locale/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/utils/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/docs/config/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/docs/manual/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/docs/examples/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/docs/sql/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/docs/man/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_build/contrib/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/gammu/gammu/_build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/gammu/gammu/_build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

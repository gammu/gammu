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

if(CMAKE_INSTALL_COMPONENT STREQUAL "pkgconfig" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES
    "/home/runner/work/gammu/gammu/_codeql_build_dir/cfg/gammu.pc"
    "/home/runner/work/gammu/gammu/_codeql_build_dir/cfg/gammu-smsd.pc"
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
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/include/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/libgammu/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/helper/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/tests/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/smsd/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/gammu/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/gammu-detect/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/locale/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/utils/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/docs/config/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/docs/manual/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/docs/examples/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/docs/sql/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/docs/man/cmake_install.cmake")
  include("/home/runner/work/gammu/gammu/_codeql_build_dir/contrib/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/gammu/gammu/_codeql_build_dir/install_local_manifest.txt"
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
  file(WRITE "/home/runner/work/gammu/gammu/_codeql_build_dir/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

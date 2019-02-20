# cmake <http://www.cmake.org> build file for Gammu
# Copyright © 2007 - 2009 Michal Čihař
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-smsd C)

set (LIBRARY_SRC
    core.c
    services/files.c
    services/null.c
    )

if (HAVE_MYSQL_MYSQL_H OR LIBDBI_FOUND OR HAVE_POSTGRESQL_LIBPQ_FE_H OR ODBC_FOUND)
    list(APPEND LIBRARY_SRC services/sql.c)
endif (HAVE_MYSQL_MYSQL_H OR LIBDBI_FOUND OR HAVE_POSTGRESQL_LIBPQ_FE_H OR ODBC_FOUND)

if (HAVE_MYSQL_MYSQL_H)
    list(APPEND LIBRARY_SRC services/mysql.c)
endif (HAVE_MYSQL_MYSQL_H)

if (HAVE_POSTGRESQL_LIBPQ_FE_H)
    list(APPEND LIBRARY_SRC services/pgsql.c)
endif (HAVE_POSTGRESQL_LIBPQ_FE_H)

if (LIBDBI_FOUND)
    list(APPEND LIBRARY_SRC services/dbi.c)
endif (LIBDBI_FOUND)

if (ODBC_FOUND)
    list(APPEND LIBRARY_SRC services/odbc.c)
endif (ODBC_FOUND)

if (WIN32)
    list(APPEND LIBRARY_SRC log-event.c)
endif (WIN32)

set (DAEMON_SRC
    main.c
    )

if (WIN32)
    list(APPEND DAEMON_SRC winservice.c)
endif (WIN32)

if (HAVE_KILL)
    list(APPEND DAEMON_SRC pidfile.c)
endif (HAVE_KILL)

if (HAVE_GETPWNAM OR HAVE_GETGRNAM OR HAVE_SETUID OR HAVE_SETGID)
    list(APPEND DAEMON_SRC uid.c)
endif (HAVE_GETPWNAM OR HAVE_GETGRNAM OR HAVE_SETUID OR HAVE_SETGID)

set (INJECT_SRC
    inject.c
    )

set (MONITOR_SRC
    monitor.c
    )

# Hides default case in switch, to allow checking whether all cases are handled.
if(DEBUG)
    add_definitions(-DCHECK_CASES)
endif(DEBUG)

# Windows Event Log support
if (WIN32)
    enable_language(RC)

# Find the Microsoft mc.exe message compiler
#
#  CMAKE_MC_COMPILER - where to find mc.exe
  # cmake has CMAKE_RC_COMPILER, but no message compiler
  if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    # this path is only present for 2008+, but we currently require PATH to
    # be set up anyway
    get_filename_component(sdk_dir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]" REALPATH)
    get_filename_component(kit_dir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot]" REALPATH)
    if (X64)
      set(sdk_bindir "${sdk_dir}/bin/x64")
      set(kit_bindir "${kit_dir}/bin/x64")
    else (X64)
      set(sdk_bindir "${sdk_dir}/bin")
      set(kit_bindir "${kit_dir}/bin/x86")
    endif (X64)
  endif ()
  find_program(CMAKE_MC_COMPILER mc.exe HINTS "${sdk_bindir}" "${kit_bindir}"
    DOC "path to message compiler")
  if (NOT CMAKE_MC_COMPILER)
    message(FATAL_ERROR "message compiler not found: required to build")
  endif (NOT CMAKE_MC_COMPILER)
  message(STATUS "Found message compiler: ${CMAKE_MC_COMPILER}")
  mark_as_advanced(CMAKE_MC_COMPILER)

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/smsd-event.h ${CMAKE_CURRENT_BINARY_DIR}/smsd-event.rc ${CMAKE_CURRENT_BINARY_DIR}/MSG00001.bin
        COMMAND ${CMAKE_MC_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/smsd-event.mc -r ${CMAKE_CURRENT_BINARY_DIR} -h ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/smsd-event.mc)
    set(EXTRA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/smsd-event.h)
    set(SMSD_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/smsd-event.rc)
else (WIN32)
    set(EXTRA_SOURCES )
    set(SMSD_RESOURCES )
endif (WIN32)

# SMSD library
if(MSVC)
    add_library (gsmsd STATIC ${LIBRARY_SRC} ${EXTRA_SOURCES})
else(MSVC)
    add_library (gsmsd ${LIBRARY_SRC} ${EXTRA_SOURCES})
endif(MSVC)
add_coverage (gsmsd)
set_target_properties (gsmsd PROPERTIES
    VERSION ${GAMMU_SOVERSION}.${GAMMU_VERSION}
    SOVERSION ${GAMMU_SOVERSION})
if(CMAKE_COMPILER_IS_MINGW AND BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES(gsmsd PROPERTIES LINK_FLAGS
        "-Wl,--output-def,${CMAKE_CURRENT_BINARY_DIR}/libgsmsd.def")
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libgsmsd.def
            DESTINATION "${INSTALL_LIB_DIR}"
            COMPONENT smsd)
endif(CMAKE_COMPILER_IS_MINGW AND BUILD_SHARED_LIBS)

target_link_libraries (gsmsd libGammu)

# Gammu-smsd program
add_executable (gammu-smsd ${DAEMON_SRC} ${SMSD_RESOURCES})
add_coverage (gammu-smsd)

target_link_libraries (gammu-smsd libGammu)
target_link_libraries (gammu-smsd gsmsd)

# Gammu-smsd-inject program
add_executable (gammu-smsd-inject ${INJECT_SRC})
add_coverage (gammu-smsd-inject)

target_link_libraries (gammu-smsd-inject libGammu)
target_link_libraries (gammu-smsd-inject gsmsd)
target_link_libraries (gammu-smsd-inject messagecmdline)

if (HAVE_SHM OR WIN32)
# Gammu-smsd-monitor program
    add_executable (gammu-smsd-monitor ${MONITOR_SRC})
    add_coverage (gammu-smsd-monitor)

    target_link_libraries (gammu-smsd-monitor libGammu)
    target_link_libraries (gammu-smsd-monitor gsmsd)
endif (HAVE_SHM OR WIN32)

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}/../include"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    )

# Tune options for found libraries
if (LIBDBI_FOUND)
    target_link_libraries (gsmsd ${LIBDBI_LIBRARIES})
    include_directories (${LIBDBI_INCLUDE_DIR})
endif (LIBDBI_FOUND)

if (ODBC_FOUND)
    target_link_libraries (gsmsd ${ODBC_LIBRARIES})
    include_directories (${ODBC_INCLUDE_DIR})
endif (ODBC_FOUND)

if (MYSQL_FOUND)
    target_link_libraries (gsmsd ${MYSQL_LIBRARIES})
    include_directories (${MYSQL_INCLUDE_DIR})
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
    target_link_libraries (gsmsd ${POSTGRES_LIBRARY})
    include_directories (${POSTGRES_INCLUDE_DIR})
endif (POSTGRES_FOUND)

install (TARGETS gsmsd gammu-smsd gammu-smsd-inject
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT smsd
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        COMPONENT smsd
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        COMPONENT smsd
        )
if (HAVE_SHM OR WIN32)
    install (TARGETS gammu-smsd-monitor
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            COMPONENT smsd
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
            COMPONENT smsd
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            COMPONENT smsd
            )
endif (HAVE_SHM OR WIN32)

# Testing
include(CMakeTests.txt)

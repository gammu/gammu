# cmake <http://www.cmake.org> build file for Gammu
# Copyright © 2007 - 2009 Michal Čihař
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-smsd C)

include(GammuTuneCompiler)

set (LIBRARY_SRC
    smsdcore.c
    s_files.c
    )

if (HAVE_MYSQL_MYSQL_H)
    list(APPEND LIBRARY_SRC s_mysql.c)
endif (HAVE_MYSQL_MYSQL_H)

if (HAVE_POSTGRESQL_LIBPQ_FE_H)
    list(APPEND LIBRARY_SRC s_pgsql.c)
endif (HAVE_POSTGRESQL_LIBPQ_FE_H)

if (WIN32)
    list(APPEND LIBRARY_SRC log-event.c)
endif (WIN32)

set (DAEMON_SRC
    main.c
    )

if (WIN32)
    list(APPEND DAEMON_SRC service.c)
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

# Hides default case in switch, to allow checking whether all cases are handled.
if(DEBUG)
    add_definitions(-DCHECK_CASES)
endif(DEBUG)

# SMSD library
add_library (gsmsd ${LIBRARY_SRC})
set_target_properties (gsmsd PROPERTIES
    VERSION ${SOVERSION}.${VERSION}
    SOVERSION ${SOVERSION})
if (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)
    set_target_properties (gsmsd PROPERTIES COMPILE_FLAGS -fPIC)
endif (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_MINGW AND NOT CMAKE_COMPILER_IS_CYGWIN)

target_link_libraries (gsmsd libGammu)

# Gammu-smsd program
add_executable (gammu-smsd ${DAEMON_SRC})

target_link_libraries (gammu-smsd libGammu)
target_link_libraries (gammu-smsd gsmsd)

# Gammu-smsd-inject program
add_executable (gammu-smsd-inject ${INJECT_SRC})

target_link_libraries (gammu-smsd-inject libGammu)
target_link_libraries (gammu-smsd-inject gsmsd)
target_link_libraries (gammu-smsd-inject messagecmdline)

include_directories (
    "${Gammu_BINARY_DIR}/include"
    )

# Tune options for found libraries
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

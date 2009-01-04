# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-smsd C)

if (HAVE_WNOUNUSEDPARAMETER)
    set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-unused-parameter")
endif (HAVE_WNOUNUSEDPARAMETER)

set (LIBRARY_SRC
    smsdcore.c
    s_files.c
    s_mysql.c
    s_pgsql.c
    )

set (BINARY_SRC
    main.c
    )

if (WIN32)
    list(APPEND BINARY_SRC service.c)
endif (WIN32)

if (HAVE_KILL)
    list(APPEND BINARY_SRC pidfile.c)
endif (HAVE_KILL)

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
add_executable (gammu-smsd ${BINARY_SRC})

target_link_libraries (gammu-smsd libGammu)
target_link_libraries (gammu-smsd gsmsd)

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

# Add libraries needed for windows build
if (WIN32)
    # Magic needed for SHGetFolderPathA
    target_link_libraries (gammu-smsd shfolder shell32)
    target_link_libraries (gsmsd shfolder shell32)
    # Win sock
    target_link_libraries (gammu-smsd ws2_32)
    target_link_libraries (gsmsd ws2_32)
    # These libraries are needed for serial driver
    target_link_libraries (gammu-smsd setupapi advapi32)
    target_link_libraries (gsmsd setupapi advapi32)
endif (WIN32)

install (TARGETS gammu-smsd
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        )
install (TARGETS gsmsd
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        )

# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 - 2018 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-binary C)

if (HAVE_WNOUNUSEDPARAMETER)
    set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-unused-parameter")
endif (HAVE_WNOUNUSEDPARAMETER)

set (BINARY_SRC
    depend/nokia/dct3.c
    depend/nokia/dct3trac/wmx.c
    depend/nokia/dct3trac/wmx-gsm.c
    depend/nokia/dct3trac/wmx-list.c
    depend/nokia/dct3trac/wmx-sim.c
    depend/nokia/dct3trac/wmx-util.c
    depend/nokia/dct4.c
    depend/siemens/dsiemens.c
    depend/siemens/chiffre.c
    common.c
    memory.c
    message.c
    mms.c
    search.c
    nokia.c
    files.c
    calendar.c
    misc.c
    gammu.c)

if (WITH_BACKUP)
    list(APPEND BINARY_SRC backup.c backupsms.c)
endif (WITH_BACKUP)

if (DEBUG)
    list(APPEND BINARY_SRC sniff.c)
endif()


# Hides default case in switch, to allow checking whether all cases are handled.
if(DEBUG)
    add_definitions(-DCHECK_CASES)
endif(DEBUG)

# Gammu program
add_executable (gammu ${BINARY_SRC})
add_coverage (gammu)

target_link_libraries (gammu libGammu)
target_link_libraries (gammu messagedisplay)
target_link_libraries (gammu messagecmdline)
target_link_libraries (gammu memorydisplay)
target_link_libraries (gammu printing)
target_link_libraries (gammu cmdline)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}/../include"
    )
# Tune options for found libraries
if (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
    target_link_libraries (gammu ${LIBINTL_LIBRARIES})
    include_directories (${LIBINTL_INCLUDE_DIR})
endif (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)

if (CURL_FOUND)
    target_link_libraries (gammu ${CURL_LIBRARIES})
    include_directories (${CURL_INCLUDE_DIR})
endif (CURL_FOUND)

install (TARGETS gammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT gammu
        )

# Testing
include(CMakeTests.txt)

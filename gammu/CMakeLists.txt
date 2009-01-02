# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
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
    sniff.c
    common.c
    memory.c
    message.c
    search.c
    nokia.c
    backup.c
    files.c
    calendar.c
    misc.c
    gammu.c)


# Hides default case in switch, to allow checking whether all cases are handled.
if(DEBUG)
    add_definitions(-DCHECK_CASES)
endif(DEBUG)

# Gammu program
add_executable (gammu ${BINARY_SRC})

target_link_libraries (gammu libGammu)
target_link_libraries (gammu gsmsd)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

include_directories (
    "${Gammu_BINARY_DIR}/include"
    )
# Tune options for found libraries
if (GETTEXTLIBS_FOUND)
    target_link_libraries (gammu ${GETTEXT_LIBRARY})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

if (CURL_FOUND)
    target_link_libraries (gammu ${CURL_LIBRARIES})
    include_directories (${CURL_INCLUDE_DIR})
endif (CURL_FOUND)

# Add libraries needed for windows build
if (WIN32)
    # Magic needed for SHGetFolderPathA
    target_link_libraries (gammu shfolder shell32)
    # Win sock
    target_link_libraries (gammu ws2_32)
    # These libraries are needed for serial driver
    target_link_libraries (gammu setupapi advapi32)
endif (WIN32)

install (TARGETS gammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        )

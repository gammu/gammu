# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-binary C)

include(GammuTuneCompiler)

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
target_link_libraries (gammu messagedisplay)
target_link_libraries (gammu messagecmdline)
target_link_libraries (gammu memorydisplay)
target_link_libraries (gammu printing)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

include_directories (
    "${Gammu_BINARY_DIR}/include"
    )
# Tune options for found libraries
if (GETTEXTLIBS_FOUND)
    target_link_libraries (gammu ${GETTEXT_LIBRARIES})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

if (CURL_FOUND)
    target_link_libraries (gammu ${CURL_LIBRARIES})
    include_directories (${CURL_INCLUDE_DIR})
endif (CURL_FOUND)

install (TARGETS gammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT gammu
        )

# Binary testing
add_test(gammu-help "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}" help)

foreach(EXT nlm ngg nol xpm nsl wbmp bmp)
    add_test(gammu-copybitmap-${EXT} "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
        copybitmap ${Gammu_SOURCE_DIR}/docs/examples/media/aliens.nlm "${CMAKE_CURRENT_BINARY_DIR}/test-bmp.${EXT}" PICTURE)
    add_test(gammu-displaybitmap-${EXT} "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
        copybitmap "${CMAKE_CURRENT_BINARY_DIR}/test-bmp.${EXT}")
    # Reading of xpm is not supported
    if (${EXT} STREQUAL xpm)
        set_tests_properties(
            gammu-displaybitmap-${EXT}
            PROPERTIES WILL_FAIL TRUE)
    endif (${EXT} STREQUAL xpm)
endforeach(EXT)

add_test(gammu-copyringtone-binary "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
    copyringtone ${Gammu_SOURCE_DIR}/docs/examples/media/axelf.txt "${CMAKE_CURRENT_BINARY_DIR}/test-rng.bin" BINARY)
add_test(gammu-copyringtone-binary-binary "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
    copyringtone "${CMAKE_CURRENT_BINARY_DIR}/test-rng.bin" "${CMAKE_CURRENT_BINARY_DIR}/test-rng-bin.bin" BINARY)

foreach(EXT ott rng mid imy ime wav rttl)
    add_test(gammu-copyringtone-${EXT} "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
        copyringtone ${Gammu_SOURCE_DIR}/docs/examples/media/axelf.txt "${CMAKE_CURRENT_BINARY_DIR}/test-rng.${EXT}" RTTL)
    add_test(gammu-copyringtone-binary-${EXT} "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
        copyringtone "${CMAKE_CURRENT_BINARY_DIR}/test-rng.bin" "${CMAKE_CURRENT_BINARY_DIR}/test-rng-bin.${EXT}" RTTL)
    add_test(gammu-copyringtone-${EXT}-binary "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
        copyringtone ${Gammu_SOURCE_DIR}/docs/examples/media/axelf.txt "${CMAKE_CURRENT_BINARY_DIR}/test-rng-${EXT}.bin" BINARY)

    foreach(EXT2 ott rng mid imy ime wav rttl)
        add_test(gammu-copyringtone-${EXT}-${EXT} "${CMAKE_CURRENT_BINARY_DIR}/gammu${GAMMU_TEST_SUFFIX}"
            copyringtone "${CMAKE_CURRENT_BINARY_DIR}/test-rng.${EXT}" "${CMAKE_CURRENT_BINARY_DIR}/test-rng-${EXT}.${EXT2}" RTTL)
        # Reading midi is not supported
        if (${EXT} STREQUAL mid OR ${EXT} STREQUAL wav)
            set_tests_properties(
                gammu-copyringtone-${EXT}-${EXT}
                PROPERTIES WILL_FAIL TRUE)
        endif (${EXT} STREQUAL mid OR ${EXT} STREQUAL wav)
    endforeach(EXT2)
endforeach(EXT)

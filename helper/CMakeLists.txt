# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-helper C)

if (HAVE_WNOUNUSEDPARAMETER)
    set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-unused-parameter")
endif (HAVE_WNOUNUSEDPARAMETER)

include_directories (
    "${Gammu_BINARY_DIR}/include"
    )

add_library (memorydisplay STATIC memory-display.c)
target_link_libraries (memorydisplay libGammu)

add_library (messagecmdline STATIC message-cmdline.c)
target_link_libraries (messagecmdline libGammu)

add_library (messagedisplay STATIC message-display.c)
target_link_libraries (messagedisplay libGammu)
target_link_libraries (messagedisplay memorydisplay)

# Tune options for found libraries
if (GETTEXTLIBS_FOUND)
    target_link_libraries (gammu ${GETTEXT_LIBRARY})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

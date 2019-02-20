# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-helper C)

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}/../include"
    )

add_library (printing STATIC printing.c)
add_coverage (printing)

add_library (cmdline STATIC cmdline.c)
add_coverage (cmdline)
target_link_libraries (cmdline printing)

add_library (memorydisplay STATIC memory-display.c)
add_coverage (memorydisplay)
target_link_libraries (memorydisplay libGammu)

add_library (messagecmdline STATIC message-cmdline.c)
add_coverage (messagecmdline)
target_link_libraries (messagecmdline libGammu)
target_link_libraries (messagecmdline printing)
target_link_libraries (messagecmdline cmdline)

add_library (messagedisplay STATIC message-display.c)
add_coverage (messagedisplay)
target_link_libraries (messagedisplay libGammu)
target_link_libraries (messagedisplay memorydisplay)

# Tune options for found libraries
if (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
    target_link_libraries (messagedisplay ${LIBINTL_LIBRARIES})
    target_link_libraries (printing ${LIBINTL_LIBRARIES})
    include_directories (${LIBINTL_INCLUDE_DIR})
endif (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)

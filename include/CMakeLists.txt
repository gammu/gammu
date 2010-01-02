# cmake <http://www.cmake.org> build file for Gammu headers
# Copyright (c) 2007 - 2010 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-headers C)

file(GLOB HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

# Fill in include dir for build
foreach (HEADER ${HEADER_FILES})
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}" "${CMAKE_CURRENT_BINARY_DIR}/${HEADER}" COPY_ONLY)
endforeach (HEADER)

# Generate config.h
configure_file ("${Gammu_SOURCE_DIR}/cmake/templates/gammu-config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gammu-config.h" ESCAPE_QUOTES)

install (
    FILES ${HEADER_FILES} ${CMAKE_CURRENT_BINARY_DIR}/gammu-config.h
    DESTINATION "${INSTALL_INC_DIR}"
    COMPONENT "headers"
    )


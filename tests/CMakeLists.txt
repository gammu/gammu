# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-tests C)

# We use Gammu
include_directories("${Gammu_BINARY_DIR}/include")

# Basic check for statemachine allocation
add_executable(statemachine-alloc statemachine-alloc.c)
target_link_libraries(statemachine-alloc Gammu)
add_test(statemachine-alloc ${EXECUTABLE_OUTPUT_PATH}/statemachine-alloc)

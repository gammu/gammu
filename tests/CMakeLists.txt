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

# vCard parsing
add_executable(vcard-read vcard-read.c)
target_link_libraries(vcard-read Gammu)

# List test cases
file(GLOB VCARDS 
    RELATIVE "${Gammu_SOURCE_DIR}/tests/vcards" 
    "${Gammu_SOURCE_DIR}/tests/vcards/*.vcf")

foreach(TESTVCARD ${VCARDS})
    string(REPLACE vcf backup TESTBACKUP ${TESTVCARD})
    string(REPLACE .vcf "" TESTNAME ${TESTVCARD})
    add_test("vcard-read-${TESTNAME}" 
        "${EXECUTABLE_OUTPUT_PATH}/vcard-read" 
        "${Gammu_SOURCE_DIR}/tests/vcards/${TESTVCARD}" 
        "${Gammu_SOURCE_DIR}/tests/vcards/${TESTBACKUP}")
endforeach(TESTVCARD $VCARDS)

# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-tests C)

# Define path for cmake tests
if (EXECUTABLE_OUTPUT_PATH)
    set (GAMMU_TEST_PATH "${EXECUTABLE_OUTPUT_PATH}/")
else (EXECUTABLE_OUTPUT_PATH)
    set (GAMMU_TEST_PATH "${CMAKE_CURRENT_BINARY_DIR}/")
endif (EXECUTABLE_OUTPUT_PATH)
if (CROSS_MINGW)
    set (GAMMU_TEST_SUFFIX ".exe")
else (CROSS_MINGW)
    set (GAMMU_TEST_SUFFIX "")
endif (CROSS_MINGW)

# We use Gammu
include_directories("${Gammu_BINARY_DIR}/include")

# Basic check for statemachine allocation
add_executable(statemachine-alloc statemachine-alloc.c)
if (CROSS_MINGW)
    set_target_properties (statemachine-alloc PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)

target_link_libraries(statemachine-alloc libGammu)
add_test(statemachine-alloc "${GAMMU_TEST_PATH}/statemachine-alloc${GAMMU_TEST_SUFFIX}")

# vCard parsing
add_executable(vcard-read vcard-read.c)
if (CROSS_MINGW)
    set_target_properties (vcard-read PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(vcard-read libGammu)

# List test cases
file(GLOB VCARDS 
    RELATIVE "${Gammu_SOURCE_DIR}/tests/vcards" 
    "${Gammu_SOURCE_DIR}/tests/vcards/*.vcf")
list(SORT VCARDS)

foreach(TESTVCARD ${VCARDS})
    string(REPLACE vcf backup TESTBACKUP ${TESTVCARD})
    string(REPLACE .vcf "" TESTNAME ${TESTVCARD})
    add_test("vcard-read-${TESTNAME}" 
        "${GAMMU_TEST_PATH}/vcard-read${GAMMU_TEST_SUFFIX}" 
        "${Gammu_SOURCE_DIR}/tests/vcards/${TESTVCARD}" 
        "${Gammu_SOURCE_DIR}/tests/vcards/${TESTBACKUP}")
endforeach(TESTVCARD $VCARDS)

# SMS parsing
add_executable(sms-nokia-01 sms-nokia-01.c)
if (CROSS_MINGW)
    set_target_properties (sms-nokia-01 PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(sms-nokia-01 libGammu)
add_test(sms-nokia-01 "${GAMMU_TEST_PATH}/sms-nokia-01${GAMMU_TEST_SUFFIX}")

# AT SMS parsing
add_executable(sms-at-parse sms-at-parse.c)
if (CROSS_MINGW)
    set_target_properties (sms-at-parse PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(sms-at-parse libGammu)

# List test cases
file(GLOB MESSAGES 
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms" 
    "${Gammu_SOURCE_DIR}/tests/at-sms/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
    add_test("sms-at-parse-${TESTNAME}" 
        "${GAMMU_TEST_PATH}/sms-at-parse${GAMMU_TEST_SUFFIX}" 
        "${Gammu_SOURCE_DIR}/tests/at-sms/${TESTMESSAGE}")
endforeach(TESTMESSAGE $MESSAGES)

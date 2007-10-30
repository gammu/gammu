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
add_executable(sms-nokia-01 sms-nokia-01.c sms-printing.c)
if (CROSS_MINGW)
    set_target_properties (sms-nokia-01 PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(sms-nokia-01 libGammu)
add_test(sms-nokia-01 "${GAMMU_TEST_PATH}/sms-nokia-01${GAMMU_TEST_SUFFIX}")

# AT SMS parsing
add_executable(sms-at-parse sms-at-parse.c sms-printing.c)
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

# AT SMSC parsing
add_executable(get-smsc-at get-smsc-at.c)
if (CROSS_MINGW)
    set_target_properties (get-smsc-at PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(get-smsc-at libGammu)

# List test cases
file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-smsc"
    "${Gammu_SOURCE_DIR}/tests/at-smsc/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
    add_test("get-smsc-at-${TESTNAME}"
        "${GAMMU_TEST_PATH}/get-smsc-at${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-smsc/${TESTMESSAGE}")
endforeach(TESTMESSAGE $MESSAGES)

# SMS encoding
add_executable(sms-encode-decode sms-encode-decode.c sms-printing.c)
if (CROSS_MINGW)
    set_target_properties (sms-encode-decode PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(sms-encode-decode libGammu)
add_test(sms-encode-decode "${GAMMU_TEST_PATH}/sms-encode-decode${GAMMU_TEST_SUFFIX}")

# Just try to compile example whether it is still correct
add_executable(phone-info "${Gammu_SOURCE_DIR}/docs/develop/examples/phone-info.c")
if (CROSS_MINGW)
    set_target_properties (phone-info PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(phone-info libGammu)

# Auto generated include tests begin
# Do not modify this section, change gen-include-test.sh instead

# Test for header gammu-backup.h
add_executable(include-backup include-backup.c)
if (CROSS_MINGW)
    set_target_properties (include-backup PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-backup libGammu)
add_test(include-backup "${GAMMU_TEST_PATH}/include-backup${GAMMU_TEST_SUFFIX}")

# Test for header gammu-bitmap.h
add_executable(include-bitmap include-bitmap.c)
if (CROSS_MINGW)
    set_target_properties (include-bitmap PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-bitmap libGammu)
add_test(include-bitmap "${GAMMU_TEST_PATH}/include-bitmap${GAMMU_TEST_SUFFIX}")

# Test for header gammu-calendar.h
add_executable(include-calendar include-calendar.c)
if (CROSS_MINGW)
    set_target_properties (include-calendar PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-calendar libGammu)
add_test(include-calendar "${GAMMU_TEST_PATH}/include-calendar${GAMMU_TEST_SUFFIX}")

# Test for header gammu-callback.h
add_executable(include-callback include-callback.c)
if (CROSS_MINGW)
    set_target_properties (include-callback PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-callback libGammu)
add_test(include-callback "${GAMMU_TEST_PATH}/include-callback${GAMMU_TEST_SUFFIX}")

# Test for header gammu-call.h
add_executable(include-call include-call.c)
if (CROSS_MINGW)
    set_target_properties (include-call PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-call libGammu)
add_test(include-call "${GAMMU_TEST_PATH}/include-call${GAMMU_TEST_SUFFIX}")

# Test for header gammu-category.h
add_executable(include-category include-category.c)
if (CROSS_MINGW)
    set_target_properties (include-category PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-category libGammu)
add_test(include-category "${GAMMU_TEST_PATH}/include-category${GAMMU_TEST_SUFFIX}")

# Test for header gammu-datetime.h
add_executable(include-datetime include-datetime.c)
if (CROSS_MINGW)
    set_target_properties (include-datetime PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-datetime libGammu)
add_test(include-datetime "${GAMMU_TEST_PATH}/include-datetime${GAMMU_TEST_SUFFIX}")

# Test for header gammu-debug.h
add_executable(include-debug include-debug.c)
if (CROSS_MINGW)
    set_target_properties (include-debug PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-debug libGammu)
add_test(include-debug "${GAMMU_TEST_PATH}/include-debug${GAMMU_TEST_SUFFIX}")

# Test for header gammu-error.h
add_executable(include-error include-error.c)
if (CROSS_MINGW)
    set_target_properties (include-error PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-error libGammu)
add_test(include-error "${GAMMU_TEST_PATH}/include-error${GAMMU_TEST_SUFFIX}")

# Test for header gammu-file.h
add_executable(include-file include-file.c)
if (CROSS_MINGW)
    set_target_properties (include-file PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-file libGammu)
add_test(include-file "${GAMMU_TEST_PATH}/include-file${GAMMU_TEST_SUFFIX}")

# Test for header gammu-info.h
add_executable(include-info include-info.c)
if (CROSS_MINGW)
    set_target_properties (include-info PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-info libGammu)
add_test(include-info "${GAMMU_TEST_PATH}/include-info${GAMMU_TEST_SUFFIX}")

# Test for header gammu-inifile.h
add_executable(include-inifile include-inifile.c)
if (CROSS_MINGW)
    set_target_properties (include-inifile PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-inifile libGammu)
add_test(include-inifile "${GAMMU_TEST_PATH}/include-inifile${GAMMU_TEST_SUFFIX}")

# Test for header gammu-keys.h
add_executable(include-keys include-keys.c)
if (CROSS_MINGW)
    set_target_properties (include-keys PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-keys libGammu)
add_test(include-keys "${GAMMU_TEST_PATH}/include-keys${GAMMU_TEST_SUFFIX}")

# Test for header gammu-limits.h
add_executable(include-limits include-limits.c)
if (CROSS_MINGW)
    set_target_properties (include-limits PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-limits libGammu)
add_test(include-limits "${GAMMU_TEST_PATH}/include-limits${GAMMU_TEST_SUFFIX}")

# Test for header gammu-memory.h
add_executable(include-memory include-memory.c)
if (CROSS_MINGW)
    set_target_properties (include-memory PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-memory libGammu)
add_test(include-memory "${GAMMU_TEST_PATH}/include-memory${GAMMU_TEST_SUFFIX}")

# Test for header gammu-message.h
add_executable(include-message include-message.c)
if (CROSS_MINGW)
    set_target_properties (include-message PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-message libGammu)
add_test(include-message "${GAMMU_TEST_PATH}/include-message${GAMMU_TEST_SUFFIX}")

# Test for header gammu-misc.h
add_executable(include-misc include-misc.c)
if (CROSS_MINGW)
    set_target_properties (include-misc PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-misc libGammu)
add_test(include-misc "${GAMMU_TEST_PATH}/include-misc${GAMMU_TEST_SUFFIX}")

# Test for header gammu-nokia.h
add_executable(include-nokia include-nokia.c)
if (CROSS_MINGW)
    set_target_properties (include-nokia PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-nokia libGammu)
add_test(include-nokia "${GAMMU_TEST_PATH}/include-nokia${GAMMU_TEST_SUFFIX}")

# Test for header gammu-ringtone.h
add_executable(include-ringtone include-ringtone.c)
if (CROSS_MINGW)
    set_target_properties (include-ringtone PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-ringtone libGammu)
add_test(include-ringtone "${GAMMU_TEST_PATH}/include-ringtone${GAMMU_TEST_SUFFIX}")

# Test for header gammu-security.h
add_executable(include-security include-security.c)
if (CROSS_MINGW)
    set_target_properties (include-security PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-security libGammu)
add_test(include-security "${GAMMU_TEST_PATH}/include-security${GAMMU_TEST_SUFFIX}")

# Test for header gammu-settings.h
add_executable(include-settings include-settings.c)
if (CROSS_MINGW)
    set_target_properties (include-settings PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-settings libGammu)
add_test(include-settings "${GAMMU_TEST_PATH}/include-settings${GAMMU_TEST_SUFFIX}")

# Test for header gammu-statemachine.h
add_executable(include-statemachine include-statemachine.c)
if (CROSS_MINGW)
    set_target_properties (include-statemachine PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-statemachine libGammu)
add_test(include-statemachine "${GAMMU_TEST_PATH}/include-statemachine${GAMMU_TEST_SUFFIX}")

# Test for header gammu-types.h
add_executable(include-types include-types.c)
if (CROSS_MINGW)
    set_target_properties (include-types PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-types libGammu)
add_test(include-types "${GAMMU_TEST_PATH}/include-types${GAMMU_TEST_SUFFIX}")

# Test for header gammu-unicode.h
add_executable(include-unicode include-unicode.c)
if (CROSS_MINGW)
    set_target_properties (include-unicode PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-unicode libGammu)
add_test(include-unicode "${GAMMU_TEST_PATH}/include-unicode${GAMMU_TEST_SUFFIX}")

# Test for header gammu-wap.h
add_executable(include-wap include-wap.c)
if (CROSS_MINGW)
    set_target_properties (include-wap PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)
target_link_libraries(include-wap libGammu)
add_test(include-wap "${GAMMU_TEST_PATH}/include-wap${GAMMU_TEST_SUFFIX}")

# Auto generated include tests end

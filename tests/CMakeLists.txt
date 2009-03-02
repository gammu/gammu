# cmake <http://www.cmake.org> build file for Gammu
# Copyright © 2007 - 2009 Michal Čihař
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-tests C)

include(GammuTuneCompiler)

# Define path for cmake tests
if (EXECUTABLE_OUTPUT_PATH)
    set (GAMMU_TEST_PATH "${EXECUTABLE_OUTPUT_PATH}/")
else (EXECUTABLE_OUTPUT_PATH)
    set (GAMMU_TEST_PATH "${CMAKE_CURRENT_BINARY_DIR}/")
endif (EXECUTABLE_OUTPUT_PATH)

# We use Gammu
include_directories("${Gammu_BINARY_DIR}/include")

# Basic check for statemachine allocation
add_executable(statemachine-alloc statemachine-alloc.c)
target_link_libraries(statemachine-alloc libGammu)
add_test(statemachine-alloc "${GAMMU_TEST_PATH}/statemachine-alloc${GAMMU_TEST_SUFFIX}")

# Basic check for statemachine functionality
add_executable(statemachine-init statemachine-init.c)
target_link_libraries(statemachine-init libGammu)
add_test(statemachine-init "${GAMMU_TEST_PATH}/statemachine-init${GAMMU_TEST_SUFFIX}")

# Debug testing
add_executable(debug debug.c)
target_link_libraries(debug libGammu)
add_test(debug "${GAMMU_TEST_PATH}/debug${GAMMU_TEST_SUFFIX}")

# Check error code  descriptions
add_executable(dump-error-codes dump-error-codes.c)
target_link_libraries(dump-error-codes libGammu)
add_test(dump-error-codes "${GAMMU_TEST_PATH}/dump-error-codes${GAMMU_TEST_SUFFIX}" "-n")

# Check feature code names
add_executable(dump-features dump-features.c)
target_link_libraries(dump-features libGammu)
add_test(dump-features "${GAMMU_TEST_PATH}/dump-features${GAMMU_TEST_SUFFIX}" "-n")

# Check feature parsing
add_executable(features-parsing features-parsing.c)
target_link_libraries(features-parsing libGammu)
add_test(features-parsing "${GAMMU_TEST_PATH}/features-parsing${GAMMU_TEST_SUFFIX}" "-n")

# Check network codes decoding
add_executable(network-codes network-codes.c)
target_link_libraries(network-codes libGammu)
add_test(network-codes "${GAMMU_TEST_PATH}/network-codes${GAMMU_TEST_SUFFIX}" "-n")

# vCard parsing
add_executable(vcard-read vcard-read.c)
target_link_libraries(vcard-read memorydisplay)
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

file(GLOB VCARDS
    RELATIVE "${Gammu_SOURCE_DIR}/tests/vcards-failing"
    "${Gammu_SOURCE_DIR}/tests/vcards-failing/*.vcf")
list(SORT VCARDS)

foreach(TESTVCARD ${VCARDS})
    string(REPLACE vcf backup TESTBACKUP ${TESTVCARD})
    string(REPLACE .vcf "" TESTNAME ${TESTVCARD})
    add_test("vcard-read-${TESTNAME}"
        "${GAMMU_TEST_PATH}/vcard-read${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/vcards-failing/${TESTVCARD}"
        "${Gammu_SOURCE_DIR}/tests/vcards-failing/${TESTBACKUP}")
    set_tests_properties(
        "vcard-read-${TESTNAME}"
        PROPERTIES WILL_FAIL TRUE)
endforeach(TESTVCARD $VCARDS)

# [iv]Calendar parsing
add_executable(cal-read cal-read.c)
target_link_libraries(cal-read libGammu)

# List test cases
file(GLOB VCALS
    RELATIVE "${Gammu_SOURCE_DIR}/tests/vcal"
    "${Gammu_SOURCE_DIR}/tests/vcal/*.vcs")
list(SORT VCALS)

foreach(TESTVCAL ${VCALS})
    string(REPLACE vcs backup TESTBACKUP ${TESTVCAL})
    string(REPLACE .vcs "" TESTNAME ${TESTVCAL})
    add_test("cal-read-${TESTNAME}"
        "${GAMMU_TEST_PATH}/cal-read${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/vcal/${TESTVCAL}"
        "${Gammu_SOURCE_DIR}/tests/vcal/${TESTBACKUP}")
endforeach(TESTVCAL $VCALS)

# List test cases
file(GLOB VCALS
    RELATIVE "${Gammu_SOURCE_DIR}/tests/vcal"
    "${Gammu_SOURCE_DIR}/tests/vcal/*.ics")
list(SORT VCALS)

foreach(TESTVCAL ${VCALS})
    string(REPLACE ics backup TESTBACKUP ${TESTVCAL})
    string(REPLACE .ics "" TESTNAME ${TESTVCAL})
    add_test("cal-read-${TESTNAME}"
        "${GAMMU_TEST_PATH}/cal-read${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/vcal/${TESTVCAL}"
        "${Gammu_SOURCE_DIR}/tests/vcal/${TESTBACKUP}")
endforeach(TESTVCAL $VCALS)

# SMS parsing
add_executable(sms-nokia-01 sms-nokia-01.c)
target_link_libraries(sms-nokia-01 libGammu)
target_link_libraries(sms-nokia-01 messagedisplay)
add_test(sms-nokia-01 "${GAMMU_TEST_PATH}/sms-nokia-01${GAMMU_TEST_SUFFIX}")

add_executable(sms-nokia-02 sms-nokia-02.c)
target_link_libraries(sms-nokia-02 libGammu)
target_link_libraries(sms-nokia-02 messagedisplay)
add_test(sms-nokia-02 "${GAMMU_TEST_PATH}/sms-nokia-02${GAMMU_TEST_SUFFIX}")

add_executable(sms-nokia-03 sms-nokia-03.c)
target_link_libraries(sms-nokia-03 libGammu)
target_link_libraries(sms-nokia-03 messagedisplay)
add_test(sms-nokia-03 "${GAMMU_TEST_PATH}/sms-nokia-03${GAMMU_TEST_SUFFIX}")

add_executable(sms-nokia-04 sms-nokia-04.c)
target_link_libraries(sms-nokia-04 libGammu)
target_link_libraries(sms-nokia-04 messagedisplay)
add_test(sms-nokia-04 "${GAMMU_TEST_PATH}/sms-nokia-04${GAMMU_TEST_SUFFIX}")

add_executable(sms-nokia-05 sms-nokia-05.c)
target_link_libraries(sms-nokia-05 libGammu)
target_link_libraries(sms-nokia-05 messagedisplay)
add_test(sms-nokia-05 "${GAMMU_TEST_PATH}/sms-nokia-05${GAMMU_TEST_SUFFIX}")

add_executable(sms-nokia-06 sms-nokia-06.c)
target_link_libraries(sms-nokia-06 libGammu)
target_link_libraries(sms-nokia-06 messagedisplay)
add_test(sms-nokia-06 "${GAMMU_TEST_PATH}/sms-nokia-06${GAMMU_TEST_SUFFIX}")

add_executable(sms-nokia-07 sms-nokia-07.c)
target_link_libraries(sms-nokia-07 libGammu)
add_test(sms-nokia-07 "${GAMMU_TEST_PATH}/sms-nokia-07${GAMMU_TEST_SUFFIX}")

# Nokia ringtone
add_executable(nokia-6110-ringtone nokia-6110-ringtone.c)
target_link_libraries(nokia-6110-ringtone libGammu)
add_test(nokia-6110-ringtone "${GAMMU_TEST_PATH}/nokia-6110-ringtone${GAMMU_TEST_SUFFIX}")

# AT SMS parsing
add_executable(sms-at-parse sms-at-parse.c)
target_link_libraries(sms-at-parse libGammu)
target_link_libraries(sms-at-parse messagedisplay)

# AT SMS parsing - simple PDU decoder
add_executable(pdu-decode pdu-decode.c)

# List test cases for success
file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms"
    "${Gammu_SOURCE_DIR}/tests/at-sms/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
    add_test("sms-at-parse-${TESTNAME}"
        "${GAMMU_TEST_PATH}/sms-at-parse${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-sms/${TESTMESSAGE}")
    add_test("pdu-decode-${TESTNAME}"
        "${GAMMU_TEST_PATH}/pdu-decode${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-sms/${TESTMESSAGE}")
endforeach(TESTMESSAGE $MESSAGES)

# List test cases for failure
file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms-failing"
    "${Gammu_SOURCE_DIR}/tests/at-sms-failing/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
    add_test("sms-at-parse-fail-${TESTNAME}"
        "${GAMMU_TEST_PATH}/sms-at-parse${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-sms-failing/${TESTMESSAGE}")
    add_test("pdu-decode-fail-${TESTNAME}"
        "${GAMMU_TEST_PATH}/pdu-decode${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-sms-failing/${TESTMESSAGE}")
    set_tests_properties(
        "pdu-decode-fail-${TESTNAME}"
        "sms-at-parse-fail-${TESTNAME}"
        PROPERTIES WILL_FAIL TRUE)
endforeach(TESTMESSAGE $MESSAGES)

# List test cases
file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms"
    "${Gammu_SOURCE_DIR}/tests/at-sms/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
endforeach(TESTMESSAGE $MESSAGES)

# AT model parsing
add_executable(get-model-at get-model-at.c)
target_link_libraries(get-model-at libGammu)

# List test cases
file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-model"
    "${Gammu_SOURCE_DIR}/tests/at-model/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
    add_test("get-model-at-${TESTNAME}"
        "${GAMMU_TEST_PATH}/get-model-at${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-model/${TESTMESSAGE}")
endforeach(TESTMESSAGE $MESSAGES)

# AT SMSC parsing
add_executable(get-smsc-at get-smsc-at.c)
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

file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-smsc-failing"
    "${Gammu_SOURCE_DIR}/tests/at-smsc-failing/*.dump")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
    add_test("get-smsc-at-${TESTNAME}"
        "${GAMMU_TEST_PATH}/get-smsc-at${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-smsc-failing/${TESTMESSAGE}")
    set_tests_properties(
        "get-smsc-at-${TESTNAME}"
        PROPERTIES WILL_FAIL TRUE)
endforeach(TESTMESSAGE $MESSAGES)

# AT SMS encoding
add_executable(sms-at-encode sms-at-encode.c)
target_link_libraries(sms-at-encode libGammu)

# List test cases
file(GLOB MESSAGES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms-encode"
    "${Gammu_SOURCE_DIR}/tests/at-sms-encode/*.backup")
list(SORT MESSAGES)

foreach(TESTMESSAGE ${MESSAGES})
    string(REPLACE .backup "" TESTNAME ${TESTMESSAGE})
    string(REPLACE backup dump TESTDUMP ${TESTMESSAGE})
    add_test("sms-at-encode-${TESTNAME}"
        "${GAMMU_TEST_PATH}/sms-at-encode${GAMMU_TEST_SUFFIX}"
        "${Gammu_SOURCE_DIR}/tests/at-sms-encode/${TESTMESSAGE}"
        "${Gammu_SOURCE_DIR}/tests/at-sms-encode/${TESTDUMP}")
endforeach(TESTMESSAGE $MESSAGES)

# SMS encoding
add_executable(sms-encode-decode sms-encode-decode.c)
target_link_libraries(sms-encode-decode libGammu)
target_link_libraries(sms-encode-decode messagedisplay)
add_test(sms-encode-decode "${GAMMU_TEST_PATH}/sms-encode-decode${GAMMU_TEST_SUFFIX}")

# SMS encoding from commandline
add_executable(sms-cmdline sms-cmdline.c)
target_link_libraries(sms-cmdline libGammu)
target_link_libraries(sms-cmdline messagedisplay)
target_link_libraries(sms-cmdline messagecmdline)

add_test(sms-cmdline-TEXT "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    TEXT 213 -text aaafdsfdfasd)
set_tests_properties(sms-cmdline-TEXT PROPERTIES
    PASS_REGULAR_EXPRESSION "Remote number        : \"213\"")

add_test(sms-cmdline-TEXT-smsc "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    TEXT 213 -text aaafdsfdfasd -smscnumber 132465)
set_tests_properties(sms-cmdline-TEXT-smsc PROPERTIES
PASS_REGULAR_EXPRESSION "Remote number        : \"213\"")


add_test(sms-cmdline-TEXT-16 "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    TEXT 213
        -text fdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdd
        -autolen 600 -16bit)
set_tests_properties(sms-cmdline-TEXT-16 PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(16 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-TEXT-unicode "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    TEXT 213 -text aaafdsfdfasd -unicode)
set_tests_properties(sms-cmdline-TEXT-unicode PROPERTIES
    PASS_REGULAR_EXPRESSION "Coding               : Unicode \\(no compression\\)")

add_test(sms-cmdline-TEXT-voice "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    text 213 -text aaafdsfdfasd -unicode -enablevoice)
set_tests_properties(sms-cmdline-TEXT-voice PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Enables voice indicator")

add_test(sms-cmdline-EMS "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    EMS 213 -text "aaafdsfdfasddsadsadsadsadijfdsalkfjla sdjflkjlfk fdsaf dsa fdsaf dsafafdasfsadfdsaf dsafdsa fdsaf dasfdsafdsafdsaf dsafdsaf daaafdsfdfasddsadsadsadsadijfdsalkfjla sdjflkjlfk fdsaf dsa fdsaf dsafafdasfsadfdsaf dsafdsa fdsaf dasfdsafdsafdsaf dsafdsaf dsafdsaf")
set_tests_properties(sms-cmdline-EMS PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(8 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-EMS-empty "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    EMS 213)
set_tests_properties(sms-cmdline-EMS-empty PROPERTIES
    FAIL_REGULAR_EXPRESSION "Entry is empty"
    WILL_FAIL TRUE)

add_test(sms-cmdline-EMS-misc "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    EMS 213
        -defsound 99
        -defanimation 1
        -protected 0
        -text "foo" -format l
        -text bar -format r
        -text moo -format b
        -text mee -format i
        -text bee -format u
        -text geee -format t)
set_tests_properties(sms-cmdline-EMS-misc PROPERTIES
    PASS_REGULAR_EXPRESSION "EMS sound ID         : 99"
    )

add_test(sms-cmdline-EMS-ring "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    EMS 54654654 -protected 1 -toneSElong  ${RINGTONE_TEST_FILE})

add_test(sms-cmdline-RINGTONE "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    RINGTONE 54654654 ${RINGTONE_TEST_FILE})
set_tests_properties(sms-cmdline-RINGTONE PROPERTIES
    PASS_REGULAR_EXPRESSION "Ringtone \"AxelF\"")

add_test(sms-cmdline-MMS "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    MMSINDICATOR 800123456 http://wammu.eu/ Wammu Gammu)
set_tests_properties(sms-cmdline-MMS PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : MMS indicator, ID \\(8 bit\\) [0-9]*, part 1 of 1")

add_test(sms-cmdline-WAP "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    WAPINDICATOR 800123456 http://wammu.eu/ Wammu)
set_tests_properties(sms-cmdline-WAP PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : MMS indicator, ID \\(8 bit\\) [0-9]*, part 1 of 1")

add_test(sms-cmdline-TODO "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    TODO 132132  ${Gammu_SOURCE_DIR}/tests/vcal/02.vcs  1)
set_tests_properties(sms-cmdline-TODO PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia calendar note, ID \\(8 bit\\) [0-9]*, part 2 of 2")

add_test(sms-cmdline-CALENDAR "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    CALENDAR 132132  ${Gammu_SOURCE_DIR}/tests/vcal/UK32Holidays.ics  30)
set_tests_properties(sms-cmdline-CALENDAR PROPERTIES
    PASS_REGULAR_EXPRESSION "SUMMARY:Bank Holiday")

add_test(sms-cmdline-VCARD "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    VCARD21 132132  ${Gammu_SOURCE_DIR}/tests/vcards/yahoo.vcf ME 1)
set_tests_properties(sms-cmdline-VCARD PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(8 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-SMSTEMPLATE "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    SMSTEMPLATE 213 -text aaafdsfdfasd)
# Not sure if this is really correct...
set_tests_properties(sms-cmdline-SMSTEMPLATE PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : User UDH")

add_test(sms-cmdline-PICTURE "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    PICTURE 213 ${LOGO_TEST_FILE})
set_tests_properties(sms-cmdline-PICTURE PROPERTIES
    PASS_REGULAR_EXPRESSION "###################   ##    ##     #     #     ##   ## ### ##   ###   ##")

add_test(sms-cmdline-OPERATOR "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    OPERATOR 213 ${LOGO_TEST_FILE} -netcode "230 03")
set_tests_properties(sms-cmdline-OPERATOR PROPERTIES
    PASS_REGULAR_EXPRESSION "Operator logo for 230 03 network \\(Vodafone CZ, Czech Republic\\)")

add_test(sms-cmdline-CALLER "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    CALLER 213 ${LOGO_TEST_FILE})
set_tests_properties(sms-cmdline-CALLER PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia caller logo")

add_test(sms-cmdline-ANIMATION "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    ANIMATION 213 6
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE})
set_tests_properties(sms-cmdline-ANIMATION PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of messages: 14")

add_test(sms-cmdline-PROFILE "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    PROFILE 213
        -bitmap ${LOGO_TEST_FILE}
        -ringtone ${RINGTONE_TEST_FILE})
set_tests_properties(sms-cmdline-PROFILE PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia profile, ID \\(8 bit\\) [0-9]*, 3 parts")

add_test(sms-cmdline-BOOKMARK "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    BOOKMARK 213 ${Gammu_SOURCE_DIR}/tests/misc/wap-bookmark.backup 1)
set_tests_properties(sms-cmdline-BOOKMARK PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia WAP bookmark")

add_test(sms-cmdline-WAPSETTINGS "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    WAPSETTINGS 213 ${Gammu_SOURCE_DIR}/tests/misc/wap-settings.backup 1 GPRS)
set_tests_properties(sms-cmdline-WAPSETTINGS PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia WAP bookmark or WAP/MMS settings, ID \\(8 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-MMSSETTINGS "${GAMMU_TEST_PATH}/sms-cmdline${GAMMU_TEST_SUFFIX}"
    MMSSETTINGS 213 ${Gammu_SOURCE_DIR}/tests/misc/mms-settings.backup 1)
set_tests_properties(sms-cmdline-MMSSETTINGS PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia WAP bookmark or WAP/MMS settings, ID \\(8 bit\\) [0-9]*, 2 parts")

# AT parser tests
add_executable(at-parser at-parser.c)
target_link_libraries(at-parser libGammu)
add_test(at-parser "${GAMMU_TEST_PATH}/at-parser${GAMMU_TEST_SUFFIX}")

# AT text encoding/decoding
add_executable(at-charset at-charset.c)
target_link_libraries(at-charset libGammu)
add_test(at-charset "${GAMMU_TEST_PATH}/at-charset${GAMMU_TEST_SUFFIX}")

# Line parser tests
add_executable(line-splitting line-splitting.c)
target_link_libraries(line-splitting libGammu)
add_test(line-splitting "${GAMMU_TEST_PATH}/line-splitting${GAMMU_TEST_SUFFIX}")

# Just try to compile example whether it is still correct
add_executable(phone-info "${Gammu_SOURCE_DIR}/docs/develop/examples/phone-info.c")
target_link_libraries(phone-info libGammu)
add_executable(sms-send "${Gammu_SOURCE_DIR}/docs/develop/examples/sms-send.c")
target_link_libraries(sms-send libGammu)


# Auto generated include tests begin
# Do not modify this section, change gen-include-test.sh instead

# Test for header gammu-backup.h
add_executable(include-backup include-backup.c)
target_link_libraries(include-backup libGammu)
add_test(include-backup "${GAMMU_TEST_PATH}/include-backup${GAMMU_TEST_SUFFIX}")

# Test for header gammu-bitmap.h
add_executable(include-bitmap include-bitmap.c)
target_link_libraries(include-bitmap libGammu)
add_test(include-bitmap "${GAMMU_TEST_PATH}/include-bitmap${GAMMU_TEST_SUFFIX}")

# Test for header gammu-calendar.h
add_executable(include-calendar include-calendar.c)
target_link_libraries(include-calendar libGammu)
add_test(include-calendar "${GAMMU_TEST_PATH}/include-calendar${GAMMU_TEST_SUFFIX}")

# Test for header gammu-callback.h
add_executable(include-callback include-callback.c)
target_link_libraries(include-callback libGammu)
add_test(include-callback "${GAMMU_TEST_PATH}/include-callback${GAMMU_TEST_SUFFIX}")

# Test for header gammu-call.h
add_executable(include-call include-call.c)
target_link_libraries(include-call libGammu)
add_test(include-call "${GAMMU_TEST_PATH}/include-call${GAMMU_TEST_SUFFIX}")

# Test for header gammu-category.h
add_executable(include-category include-category.c)
target_link_libraries(include-category libGammu)
add_test(include-category "${GAMMU_TEST_PATH}/include-category${GAMMU_TEST_SUFFIX}")

# Test for header gammu-datetime.h
add_executable(include-datetime include-datetime.c)
target_link_libraries(include-datetime libGammu)
add_test(include-datetime "${GAMMU_TEST_PATH}/include-datetime${GAMMU_TEST_SUFFIX}")

# Test for header gammu-debug.h
add_executable(include-debug include-debug.c)
target_link_libraries(include-debug libGammu)
add_test(include-debug "${GAMMU_TEST_PATH}/include-debug${GAMMU_TEST_SUFFIX}")

# Test for header gammu-error.h
add_executable(include-error include-error.c)
target_link_libraries(include-error libGammu)
add_test(include-error "${GAMMU_TEST_PATH}/include-error${GAMMU_TEST_SUFFIX}")

# Test for header gammu-file.h
add_executable(include-file include-file.c)
target_link_libraries(include-file libGammu)
add_test(include-file "${GAMMU_TEST_PATH}/include-file${GAMMU_TEST_SUFFIX}")

# Test for header gammu-info.h
add_executable(include-info include-info.c)
target_link_libraries(include-info libGammu)
add_test(include-info "${GAMMU_TEST_PATH}/include-info${GAMMU_TEST_SUFFIX}")

# Test for header gammu-inifile.h
add_executable(include-inifile include-inifile.c)
target_link_libraries(include-inifile libGammu)
add_test(include-inifile "${GAMMU_TEST_PATH}/include-inifile${GAMMU_TEST_SUFFIX}")

# Test for header gammu-keys.h
add_executable(include-keys include-keys.c)
target_link_libraries(include-keys libGammu)
add_test(include-keys "${GAMMU_TEST_PATH}/include-keys${GAMMU_TEST_SUFFIX}")

# Test for header gammu-limits.h
add_executable(include-limits include-limits.c)
target_link_libraries(include-limits libGammu)
add_test(include-limits "${GAMMU_TEST_PATH}/include-limits${GAMMU_TEST_SUFFIX}")

# Test for header gammu-memory.h
add_executable(include-memory include-memory.c)
target_link_libraries(include-memory libGammu)
add_test(include-memory "${GAMMU_TEST_PATH}/include-memory${GAMMU_TEST_SUFFIX}")

# Test for header gammu-message.h
add_executable(include-message include-message.c)
target_link_libraries(include-message libGammu)
add_test(include-message "${GAMMU_TEST_PATH}/include-message${GAMMU_TEST_SUFFIX}")

# Test for header gammu-misc.h
add_executable(include-misc include-misc.c)
target_link_libraries(include-misc libGammu)
add_test(include-misc "${GAMMU_TEST_PATH}/include-misc${GAMMU_TEST_SUFFIX}")

# Test for header gammu-nokia.h
add_executable(include-nokia include-nokia.c)
target_link_libraries(include-nokia libGammu)
add_test(include-nokia "${GAMMU_TEST_PATH}/include-nokia${GAMMU_TEST_SUFFIX}")

# Test for header gammu-ringtone.h
add_executable(include-ringtone include-ringtone.c)
target_link_libraries(include-ringtone libGammu)
add_test(include-ringtone "${GAMMU_TEST_PATH}/include-ringtone${GAMMU_TEST_SUFFIX}")

# Test for header gammu-security.h
add_executable(include-security include-security.c)
target_link_libraries(include-security libGammu)
add_test(include-security "${GAMMU_TEST_PATH}/include-security${GAMMU_TEST_SUFFIX}")

# Test for header gammu-settings.h
add_executable(include-settings include-settings.c)
target_link_libraries(include-settings libGammu)
add_test(include-settings "${GAMMU_TEST_PATH}/include-settings${GAMMU_TEST_SUFFIX}")

# Test for header gammu-smsd.h
add_executable(include-smsd include-smsd.c)
target_link_libraries(include-smsd libGammu)
add_test(include-smsd "${GAMMU_TEST_PATH}/include-smsd${GAMMU_TEST_SUFFIX}")

# Test for header gammu-statemachine.h
add_executable(include-statemachine include-statemachine.c)
target_link_libraries(include-statemachine libGammu)
add_test(include-statemachine "${GAMMU_TEST_PATH}/include-statemachine${GAMMU_TEST_SUFFIX}")

# Test for header gammu-types.h
add_executable(include-types include-types.c)
target_link_libraries(include-types libGammu)
add_test(include-types "${GAMMU_TEST_PATH}/include-types${GAMMU_TEST_SUFFIX}")

# Test for header gammu-unicode.h
add_executable(include-unicode include-unicode.c)
target_link_libraries(include-unicode libGammu)
add_test(include-unicode "${GAMMU_TEST_PATH}/include-unicode${GAMMU_TEST_SUFFIX}")

# Test for header gammu-wap.h
add_executable(include-wap include-wap.c)
target_link_libraries(include-wap libGammu)
add_test(include-wap "${GAMMU_TEST_PATH}/include-wap${GAMMU_TEST_SUFFIX}")

# Auto generated include tests end

# Test for locking, only on !WIN32 and if we can write to lock dir
if (NOT WIN32)
    execute_process(COMMAND test -w /var/lock/ RESULT_VARIABLE VAR_LOCK_WRITABLE)
    if (VAR_LOCK_WRITABLE EQUAL 0)
        add_executable(locking locking.c)
        target_link_libraries(locking libGammu)
        add_test(locking "${GAMMU_TEST_PATH}/locking${GAMMU_TEST_SUFFIX}")
    else (VAR_LOCK_WRITABLE EQUAL 0)
        message("/var/lock is not writable, skipping locking tests!")
    endif (VAR_LOCK_WRITABLE EQUAL 0)
endif (NOT WIN32)

# cmake <http://www.cmake.org> build file for Gammu
# Copyright © 2007 - 2009 Michal Čihař
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu-tests C)

include(SetupDummyPhone)

# Define path for cmake tests
if (EXECUTABLE_OUTPUT_PATH)
    set (GAMMU_TEST_PATH "${EXECUTABLE_OUTPUT_PATH}/")
else (EXECUTABLE_OUTPUT_PATH)
    set (GAMMU_TEST_PATH "${CMAKE_CURRENT_BINARY_DIR}/")
endif (EXECUTABLE_OUTPUT_PATH)

# We use Gammu
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../include")

# ATGEN function tests
add_subdirectory(atgen)

# Basic check for statemachine allocation
add_executable(statemachine-alloc statemachine-alloc.c)
add_coverage(statemachine-alloc)
target_link_libraries(statemachine-alloc libGammu ${LIBINTL_LIBRARIES})
add_test(statemachine-alloc "${GAMMU_TEST_PATH}/statemachine-alloc${CMAKE_EXECUTABLE_SUFFIX}")

if (BLUETOOTH_RF_SEARCHING)
    add_executable(bluetooth_checkservicename bluetooth_checkservicename.c)
    add_coverage(bluetooth_checkservicename)
    target_link_libraries(bluetooth_checkservicename libGammu ${LIBINTL_LIBRARIES})
    add_test(bluetooth_checkservicename "${GAMMU_TEST_PATH}/bluetooth_checkservicename${CMAKE_EXECUTABLE_SUFFIX}")
endif (BLUETOOTH_RF_SEARCHING)

# Basic check for sizes
add_executable(sizes sizes.c)
add_coverage(sizes)
add_test(sizes "${GAMMU_TEST_PATH}/sizes${CMAKE_EXECUTABLE_SUFFIX}")
set_tests_properties(sizes PROPERTIES
    FAIL_REGULAR_EXPRESSION "[0-9][0-9] MiB")

add_executable(base64 base64.c)
add_coverage(base64)
target_link_libraries(base64 libGammu ${LIBINTL_LIBRARIES})
add_test(base64 "${GAMMU_TEST_PATH}/base64${CMAKE_EXECUTABLE_SUFFIX}")

# Array manipulation tests
add_executable(array-test array-test.c)
add_coverage(array-test)
target_link_libraries (array-test libGammu)
add_test(array "${GAMMU_TEST_PATH}/array-test${CMAKE_EXECUTABLE_SUFFIX}")

# UTF-8 manipulation tests
add_executable(utf-8 utf-8.c)
add_coverage(utf-8)
target_link_libraries (utf-8 libGammu)
add_test(utf-8 "${GAMMU_TEST_PATH}/utf-8${CMAKE_EXECUTABLE_SUFFIX}")

# SQL backend date parsing
if (HAVE_MYSQL_MYSQL_H OR LIBDBI_FOUND OR HAVE_POSTGRESQL_LIBPQ_FE_H)
    if (LIBDBI_FOUND)
        include_directories (${LIBDBI_INCLUDE_DIR})
    endif (LIBDBI_FOUND)

    if (MYSQL_FOUND)
        include_directories (${MYSQL_INCLUDE_DIR})
    endif (MYSQL_FOUND)

    if (POSTGRES_FOUND)
        include_directories (${POSTGRES_INCLUDE_DIR})
    endif (POSTGRES_FOUND)

    add_executable(sql-parse-date sql-parse-date.c)
    add_coverage(sql-parse-date)
    target_link_libraries (sql-parse-date gsmsd)
    add_test(sql-parse-date "${GAMMU_TEST_PATH}/sql-parse-date${CMAKE_EXECUTABLE_SUFFIX}")
endif (HAVE_MYSQL_MYSQL_H OR LIBDBI_FOUND OR HAVE_POSTGRESQL_LIBPQ_FE_H)

# Backup comments
if (WITH_BACKUP)
    add_executable(backup-comment backup-comment.c)
    add_coverage(backup-comment)
    target_link_libraries (backup-comment libGammu ${LIBINTL_LIBRARIES})
    add_test(backup-comment "${GAMMU_TEST_PATH}/backup-comment${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_BACKUP)

# Backup reading
if (WITH_BACKUP)
    add_executable(read-backup read-backup.c)
    add_coverage(read-backup)
    target_link_libraries (read-backup libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB BACKUPS
        RELATIVE "${Gammu_SOURCE_DIR}/tests/backups"
        "${Gammu_SOURCE_DIR}/tests/backups/*.backup")
    list(SORT BACKUPS)

    foreach(TESTBACKUP ${BACKUPS})
        string(REPLACE .backup "" TESTNAME ${TESTBACKUP})
        add_test("read-backup-${TESTNAME}"
            "${GAMMU_TEST_PATH}/read-backup${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/backups/${TESTBACKUP}")
    endforeach(TESTBACKUP $BACKUPS)
endif (WITH_BACKUP)

# Basic check for statemachine functionality
add_executable(statemachine-init statemachine-init.c)
add_coverage(statemachine-init)
target_link_libraries(statemachine-init libGammu ${LIBINTL_LIBRARIES})
add_test(statemachine-init "${GAMMU_TEST_PATH}/statemachine-init${CMAKE_EXECUTABLE_SUFFIX}")

# USB device parsing
if (LIBUSB_FOUND AND WITH_NOKIA_SUPPORT)
    add_executable(usb-device-parse usb-device-parse.c)
    add_coverage(usb-device-parse)
    target_link_libraries(usb-device-parse libGammu ${LIBINTL_LIBRARIES})
    add_test(usb-device-parse "${GAMMU_TEST_PATH}/usb-device-parse${CMAKE_EXECUTABLE_SUFFIX}")
endif (LIBUSB_FOUND AND WITH_NOKIA_SUPPORT)

# Debug testing
add_executable(debug debug.c)
add_coverage(debug)
target_link_libraries(debug libGammu ${LIBINTL_LIBRARIES})
add_test(debug "${GAMMU_TEST_PATH}/debug${CMAKE_EXECUTABLE_SUFFIX}")

# Check error code  descriptions
add_executable(dump-error-codes dump-error-codes.c)
add_coverage(dump-error-codes)
target_link_libraries(dump-error-codes libGammu ${LIBINTL_LIBRARIES})
add_test(dump-error-codes "${GAMMU_TEST_PATH}/dump-error-codes${CMAKE_EXECUTABLE_SUFFIX}" "-n")

# Check feature code names
add_executable(dump-features dump-features.c)
add_coverage(dump-features)
target_link_libraries(dump-features libGammu ${LIBINTL_LIBRARIES})
add_test(dump-features "${GAMMU_TEST_PATH}/dump-features${CMAKE_EXECUTABLE_SUFFIX}" "-n")

# Check feature parsing
add_executable(features-parsing features-parsing.c)
add_coverage(features-parsing)
target_link_libraries(features-parsing libGammu ${LIBINTL_LIBRARIES})
add_test(features-parsing "${GAMMU_TEST_PATH}/features-parsing${CMAKE_EXECUTABLE_SUFFIX}" "-n")

# Check network codes decoding
add_executable(network-codes network-codes.c)
add_coverage(network-codes)
target_link_libraries(network-codes libGammu ${LIBINTL_LIBRARIES})
add_test(network-codes "${GAMMU_TEST_PATH}/network-codes${CMAKE_EXECUTABLE_SUFFIX}" "-n")

if (WITH_BACKUP)
    # smsbackup parsing
    add_executable(smsbackup smsbackup.c)
    add_coverage(smsbackup)
    target_link_libraries(smsbackup messagedisplay)
    target_link_libraries(smsbackup libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB SMSBACKUPS
        RELATIVE "${Gammu_SOURCE_DIR}/tests/smsbackups"
        "${Gammu_SOURCE_DIR}/tests/smsbackups/*.smsbackup")
    list(SORT SMSBACKUPS)

    foreach(TESTSMSBACKUP ${SMSBACKUPS})
        string(REPLACE .smsbackup "" TESTNAME ${TESTSMSBACKUP})
        add_test("smsbackup-${TESTNAME}"
            "${GAMMU_TEST_PATH}/smsbackup${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/smsbackups/${TESTSMSBACKUP}")
    endforeach(TESTSMSBACKUP $SMSBACKUPS)
endif (WITH_BACKUP)

if (WITH_BACKUP)
    # vCard parsing
    add_executable(vcard-read vcard-read.c)
    add_coverage(vcard-read)
    target_link_libraries(vcard-read memorydisplay)
    target_link_libraries(vcard-read libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB VCARDS
        RELATIVE "${Gammu_SOURCE_DIR}/tests/vcards"
        "${Gammu_SOURCE_DIR}/tests/vcards/*.vcf")
    list(SORT VCARDS)

    foreach(TESTVCARD ${VCARDS})
        string(REPLACE vcf backup TESTBACKUP ${TESTVCARD})
        string(REPLACE .vcf "" TESTNAME ${TESTVCARD})
        add_test("vcard-read-${TESTNAME}"
            "${GAMMU_TEST_PATH}/vcard-read${CMAKE_EXECUTABLE_SUFFIX}"
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
            "${GAMMU_TEST_PATH}/vcard-read${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/vcards-failing/${TESTVCARD}"
            "${Gammu_SOURCE_DIR}/tests/vcards-failing/${TESTBACKUP}")
        set_tests_properties(
            "vcard-read-${TESTNAME}"
            PROPERTIES WILL_FAIL TRUE)
    endforeach(TESTVCARD $VCARDS)

    # LDIF parsing
    add_executable(ldif-read ldif-read.c)
    add_coverage(ldif-read)
    target_link_libraries(ldif-read memorydisplay)
    target_link_libraries(ldif-read libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB LDIFS
        RELATIVE "${Gammu_SOURCE_DIR}/tests/ldif"
        "${Gammu_SOURCE_DIR}/tests/ldif/*.ldif")
    list(SORT LDIFS)

    foreach(TESTLDIF ${LDIFS})
        string(REPLACE ldif backup TESTBACKUP ${TESTLDIF})
        string(REPLACE .ldif "" TESTNAME ${TESTLDIF})
        add_test("ldif-read-${TESTNAME}"
            "${GAMMU_TEST_PATH}/ldif-read${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/ldif/${TESTLDIF}"
            "${Gammu_SOURCE_DIR}/tests/ldif/${TESTBACKUP}")
    endforeach(TESTLDIF $LDIFS)

endif (WITH_BACKUP)

# [iv]Calendar parsing
if (WITH_BACKUP)
    add_executable(cal-read cal-read.c)
    add_coverage(cal-read)
    target_link_libraries(cal-read libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB VCALS
        RELATIVE "${Gammu_SOURCE_DIR}/tests/vcal"
        "${Gammu_SOURCE_DIR}/tests/vcal/*.vcs")
    list(SORT VCALS)

    foreach(TESTVCAL ${VCALS})
        string(REPLACE vcs backup TESTBACKUP ${TESTVCAL})
        string(REPLACE .vcs "" TESTNAME ${TESTVCAL})
        add_test("cal-read-${TESTNAME}"
            "${GAMMU_TEST_PATH}/cal-read${CMAKE_EXECUTABLE_SUFFIX}"
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
            "${GAMMU_TEST_PATH}/cal-read${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/vcal/${TESTVCAL}"
            "${Gammu_SOURCE_DIR}/tests/vcal/${TESTBACKUP}")
    endforeach(TESTVCAL $VCALS)
endif (WITH_BACKUP)

# SMS parsing
if (WITH_NOKIA6110)
    add_executable(sms-nokia-01 sms-nokia-01.c)
    add_coverage(sms-nokia-01)
    target_link_libraries(sms-nokia-01 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-01 messagedisplay)
    add_test(sms-nokia-01 "${GAMMU_TEST_PATH}/sms-nokia-01${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6110)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-02 sms-nokia-02.c)
    add_coverage(sms-nokia-02)
    target_link_libraries(sms-nokia-02 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-02 messagedisplay)
    add_test(sms-nokia-02 "${GAMMU_TEST_PATH}/sms-nokia-02${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-03 sms-nokia-03.c)
    add_coverage(sms-nokia-03)
    target_link_libraries(sms-nokia-03 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-03 messagedisplay)
    add_test(sms-nokia-03 "${GAMMU_TEST_PATH}/sms-nokia-03${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-04 sms-nokia-04.c)
    add_coverage(sms-nokia-04)
    target_link_libraries(sms-nokia-04 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-04 messagedisplay)
    add_test(sms-nokia-04 "${GAMMU_TEST_PATH}/sms-nokia-04${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-05 sms-nokia-05.c)
    add_coverage(sms-nokia-05)
    target_link_libraries(sms-nokia-05 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-05 messagedisplay)
    add_test(sms-nokia-05 "${GAMMU_TEST_PATH}/sms-nokia-05${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-06 sms-nokia-06.c)
    add_coverage(sms-nokia-06)
    target_link_libraries(sms-nokia-06 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-06 messagedisplay)
    add_test(sms-nokia-06 "${GAMMU_TEST_PATH}/sms-nokia-06${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-07 sms-nokia-07.c)
    add_coverage(sms-nokia-07)
    target_link_libraries(sms-nokia-07 libGammu ${LIBINTL_LIBRARIES})
    add_test(sms-nokia-07 "${GAMMU_TEST_PATH}/sms-nokia-07${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-08 sms-nokia-08.c)
    add_coverage(sms-nokia-08)
    target_link_libraries(sms-nokia-08 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-08 messagedisplay)
    add_test(sms-nokia-08 "${GAMMU_TEST_PATH}/sms-nokia-08${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-09 sms-nokia-09.c)
    add_coverage(sms-nokia-09)
    target_link_libraries(sms-nokia-09 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-09 messagedisplay)
    add_test(sms-nokia-09 "${GAMMU_TEST_PATH}/sms-nokia-09${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-10 sms-nokia-10.c)
    add_coverage(sms-nokia-10)
    target_link_libraries(sms-nokia-10 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-10 messagedisplay)
    add_test(sms-nokia-10 "${GAMMU_TEST_PATH}/sms-nokia-10${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-11 sms-nokia-11.c)
    add_coverage(sms-nokia-11)
    target_link_libraries(sms-nokia-11 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-11 messagedisplay)
    add_test(sms-nokia-11 "${GAMMU_TEST_PATH}/sms-nokia-11${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-12 sms-nokia-12.c)
    add_coverage(sms-nokia-12)
    target_link_libraries(sms-nokia-12 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-12 messagedisplay)
    add_test(sms-nokia-12 "${GAMMU_TEST_PATH}/sms-nokia-12${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-13 sms-nokia-13.c)
    add_coverage(sms-nokia-13)
    target_link_libraries(sms-nokia-13 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-13 messagedisplay)
    add_test(sms-nokia-13 "${GAMMU_TEST_PATH}/sms-nokia-13${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-14 sms-nokia-14.c)
    add_coverage(sms-nokia-14)
    target_link_libraries(sms-nokia-14 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-14 messagedisplay)
    add_test(sms-nokia-14 "${GAMMU_TEST_PATH}/sms-nokia-14${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-15 sms-nokia-15.c)
    add_coverage(sms-nokia-15)
    target_link_libraries(sms-nokia-15 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-15 messagedisplay)
    add_test(sms-nokia-15 "${GAMMU_TEST_PATH}/sms-nokia-15${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-16 sms-nokia-16.c)
    add_coverage(sms-nokia-16)
    target_link_libraries(sms-nokia-16 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-16 messagedisplay)
    add_test(sms-nokia-16 "${GAMMU_TEST_PATH}/sms-nokia-16${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-17 sms-nokia-17.c)
    add_coverage(sms-nokia-17)
    target_link_libraries(sms-nokia-17 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-17 messagedisplay)
    add_test(sms-nokia-17 "${GAMMU_TEST_PATH}/sms-nokia-17${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6510)
    add_executable(sms-nokia-18 sms-nokia-18.c)
    add_coverage(sms-nokia-18)
    target_link_libraries(sms-nokia-18 libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(sms-nokia-18 messagedisplay)
    add_test(sms-nokia-18 "${GAMMU_TEST_PATH}/sms-nokia-18${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6510)

if (WITH_NOKIA6110)
# Nokia ringtone
    add_executable(nokia-6110-ringtone nokia-6110-ringtone.c)
    add_coverage(nokia-6110-ringtone)
    target_link_libraries(nokia-6110-ringtone libGammu ${LIBINTL_LIBRARIES})
    add_test(nokia-6110-ringtone "${GAMMU_TEST_PATH}/nokia-6110-ringtone${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_NOKIA6110)

if (WITH_ATGEN)
    # AT SMS parsing
    add_executable(sms-at-parse sms-at-parse.c)
    add_coverage(sms-at-parse)
    target_link_libraries(sms-at-parse libGammu ${LIBINTL_LIBRARIES} gsmsd)
    target_link_libraries(sms-at-parse messagedisplay)

    # List test cases for success
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms"
        "${Gammu_SOURCE_DIR}/tests/at-sms/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("sms-at-parse-${TESTNAME}"
            "${GAMMU_TEST_PATH}/sms-at-parse${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-sms/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    # List test cases for TXT mode
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms-txt"
        "${Gammu_SOURCE_DIR}/tests/at-sms-txt/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("sms-at-parse-txt-${TESTNAME}"
            "${GAMMU_TEST_PATH}/sms-at-parse${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-sms-txt/${TESTMESSAGE}"
            "TXT")
    endforeach(TESTMESSAGE $MESSAGES)

    # List test cases for TXT mode
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms-txt-detail"
        "${Gammu_SOURCE_DIR}/tests/at-sms-txt-detail/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("sms-at-parse-txtdetail-${TESTNAME}"
            "${GAMMU_TEST_PATH}/sms-at-parse${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-sms-txt-detail/${TESTMESSAGE}"
            "TXTDETAIL")
    endforeach(TESTMESSAGE $MESSAGES)

    # List test cases for failure
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms-failing"
        "${Gammu_SOURCE_DIR}/tests/at-sms-failing/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("sms-at-parse-fail-${TESTNAME}"
            "${GAMMU_TEST_PATH}/sms-at-parse${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-sms-failing/${TESTMESSAGE}")
        set_tests_properties(
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
    add_coverage(get-model-at)
    target_link_libraries(get-model-at libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-model"
        "${Gammu_SOURCE_DIR}/tests/at-model/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("get-model-at-${TESTNAME}"
            "${GAMMU_TEST_PATH}/get-model-at${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-model/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    # Samsung memory parsing
    add_executable(samsung-get-memory samsung-get-memory.c)
    add_coverage(samsung-get-memory)
    target_link_libraries(samsung-get-memory libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(samsung-get-memory memorydisplay)

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/samsung-memory"
        "${Gammu_SOURCE_DIR}/tests/samsung-memory/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("samsung-get-memory-${TESTNAME}"
            "${GAMMU_TEST_PATH}/samsung-get-memory${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/samsung-memory/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    # AT getting of available phone memories
    add_executable(at-get-smsmemories at-get-smsmemories.c)
    add_coverage(at-get-smsmemories)
    target_link_libraries(at-get-smsmemories libGammu ${LIBINTL_LIBRARIES})

    # List test cases for all memories
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-cpms"
        "${Gammu_SOURCE_DIR}/tests/at-cpms/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("at-get-smsmemories-${TESTNAME}"
            "${GAMMU_TEST_PATH}/at-get-smsmemories${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-cpms/${TESTMESSAGE}")
        set_tests_properties("at-get-smsmemories-${TESTNAME}" PROPERTIES
            PASS_REGULAR_EXPRESSION "read: ME : ok, SM : ok, SR : N/A save: ME : ok, SM : ok, SR : N/A")
    endforeach(TESTMESSAGE $MESSAGES)

    # List test cases for just SM
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-cpms-sm"
        "${Gammu_SOURCE_DIR}/tests/at-cpms-sm/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("at-get-smsmemories-sm-${TESTNAME}"
            "${GAMMU_TEST_PATH}/at-get-smsmemories${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-cpms-sm/${TESTMESSAGE}")
        set_tests_properties("at-get-smsmemories-sm-${TESTNAME}" PROPERTIES
            PASS_REGULAR_EXPRESSION "read: ME : N/A, SM : ok, SR : N/A save: ME : N/A, SM : ok, SR : N/A")
    endforeach(TESTMESSAGE $MESSAGES)

    # List test cases for just SR
    file(GLOB MESSAGES
            RELATIVE "${Gammu_SOURCE_DIR}/tests/at-cpms-sr"
            "${Gammu_SOURCE_DIR}/tests/at-cpms-sr/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("at-get-smsmemories-sr-${TESTNAME}"
                "${GAMMU_TEST_PATH}/at-get-smsmemories${CMAKE_EXECUTABLE_SUFFIX}"
                "${Gammu_SOURCE_DIR}/tests/at-cpms-sr/${TESTMESSAGE}")
        set_tests_properties("at-get-smsmemories-sr-${TESTNAME}" PROPERTIES
                PASS_REGULAR_EXPRESSION "read: ME : ok, SM : N/A, SR : ok save: ME : ok, SM : N/A, SR : ok")
    endforeach(TESTMESSAGE $MESSAGES)

    # AT CNMI parsing
    add_executable(at-cnmi-reply at-cnmi-reply.c)
    add_coverage(at-cnmi-reply)
    target_link_libraries(at-cnmi-reply libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-cnmi"
        "${Gammu_SOURCE_DIR}/tests/at-cnmi/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("at-cnmi-reply-${TESTNAME}"
            "${GAMMU_TEST_PATH}/at-cnmi-reply${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-cnmi/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    # AT CCFC parsing
    add_executable(at-ccfc-reply at-ccfc-reply.c)
    add_coverage(at-ccfc-reply)
    target_link_libraries(at-ccfc-reply libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-ccfc"
        "${Gammu_SOURCE_DIR}/tests/at-ccfc/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("at-ccfc-reply-${TESTNAME}"
            "${GAMMU_TEST_PATH}/at-ccfc-reply${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-ccfc/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    # AT CREG parsing
    add_executable(at-creg-reply at-creg-reply.c)
    add_coverage(at-creg-reply)
    target_link_libraries(at-creg-reply libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-creg"
        "${Gammu_SOURCE_DIR}/tests/at-creg/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("at-creg-reply-${TESTNAME}"
            "${GAMMU_TEST_PATH}/at-creg-reply${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-creg/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    # AT memory parsing
    add_executable(at-getmemory-reply at-getmemory-reply.c)
    add_coverage(at-getmemory-reply)
    target_link_libraries(at-getmemory-reply libGammu ${LIBINTL_LIBRARIES})
    target_link_libraries(at-getmemory-reply memorydisplay)

    macro(at_getmemory_reply_test _file _charset _test)

    add_test("at-getmemory-reply-${_file}"
            "${GAMMU_TEST_PATH}/at-getmemory-reply${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-getmemory/${_file}.dump"
            "${_charset}")
    set_tests_properties("at-getmemory-reply-${_file}"
        PROPERTIES PASS_REGULAR_EXPRESSION "${_test}")

    endmacro(at_getmemory_reply_test _file _charset _test)

    at_getmemory_reply_test(samsung PCCP437 AsistZahr)
    at_getmemory_reply_test(samsung-p900 PCCP437 "My Tempo")
    at_getmemory_reply_test(with-time UTF8 "Mama GSM")
    at_getmemory_reply_test(with-date-time UTF8 "60122256476")
    at_getmemory_reply_test(ucs2 UCS2 "Stanley Paul")
    at_getmemory_reply_test(ucs2-motorola UCS2 "Virchow Klinikum St. 31")
    at_getmemory_reply_test(nokia-2730 UCS2 "Steve  Vinson")

    # AT USSD replies parsing
    add_executable(at-ussd-reply at-ussd-reply.c)
    add_coverage(at-ussd-reply)
    target_link_libraries(at-ussd-reply libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-ussd"
        "${Gammu_SOURCE_DIR}/tests/at-ussd/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        string(REGEX REPLACE ".*-(.*)\\.dump" "\\1" PHONE_MODEL ${TESTMESSAGE})
        if (${TESTMESSAGE} MATCHES "^U")
            set(TEST_UNICODE_FLAG "U")
        else (${TESTMESSAGE} MATCHES "^U")
            set(TEST_UNICODE_FLAG "G")
        endif (${TESTMESSAGE} MATCHES "^U")
        add_test("at-ussd-reply-${TESTNAME}"
            "${GAMMU_TEST_PATH}/at-ussd-reply${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-ussd/${TESTMESSAGE}" "${PHONE_MODEL}" "${TEST_UNICODE_FLAG}")
        set_tests_properties("at-ussd-reply-${TESTNAME}"
            PROPERTIES FAIL_REGULAR_EXPRESSION "Response: (00[A-Fa-f0-0]*)?\n")
    endforeach(TESTMESSAGE $MESSAGES)

    # AT SMSC parsing
    add_executable(get-smsc-at get-smsc-at.c)
    add_coverage(get-smsc-at)
    target_link_libraries(get-smsc-at libGammu ${LIBINTL_LIBRARIES})

    # List test cases
    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-smsc"
        "${Gammu_SOURCE_DIR}/tests/at-smsc/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("get-smsc-at-${TESTNAME}"
            "${GAMMU_TEST_PATH}/get-smsc-at${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-smsc/${TESTMESSAGE}")
    endforeach(TESTMESSAGE $MESSAGES)

    file(GLOB MESSAGES
        RELATIVE "${Gammu_SOURCE_DIR}/tests/at-smsc-failing"
        "${Gammu_SOURCE_DIR}/tests/at-smsc-failing/*.dump")
    list(SORT MESSAGES)

    foreach(TESTMESSAGE ${MESSAGES})
        string(REPLACE .dump "" TESTNAME ${TESTMESSAGE})
        add_test("get-smsc-at-${TESTNAME}"
            "${GAMMU_TEST_PATH}/get-smsc-at${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/at-smsc-failing/${TESTMESSAGE}")
        set_tests_properties(
            "get-smsc-at-${TESTNAME}"
            PROPERTIES WILL_FAIL TRUE)
    endforeach(TESTMESSAGE $MESSAGES)

    if (WITH_BACKUP)
        # AT SMS encoding
        add_executable(sms-at-encode sms-at-encode.c)
        add_coverage(sms-at-encode)
        target_link_libraries(sms-at-encode libGammu ${LIBINTL_LIBRARIES})

        # List test cases
        file(GLOB MESSAGES
            RELATIVE "${Gammu_SOURCE_DIR}/tests/at-sms-encode"
            "${Gammu_SOURCE_DIR}/tests/at-sms-encode/*.backup")
        list(SORT MESSAGES)

        foreach(TESTMESSAGE ${MESSAGES})
            string(REPLACE .backup "" TESTNAME ${TESTMESSAGE})
            string(REPLACE backup dump TESTDUMP ${TESTMESSAGE})
            add_test("sms-at-encode-${TESTNAME}"
                "${GAMMU_TEST_PATH}/sms-at-encode${CMAKE_EXECUTABLE_SUFFIX}"
                "${Gammu_SOURCE_DIR}/tests/at-sms-encode/${TESTMESSAGE}"
                "${Gammu_SOURCE_DIR}/tests/at-sms-encode/${TESTDUMP}")
        endforeach(TESTMESSAGE $MESSAGES)
    endif (WITH_BACKUP)
endif (WITH_ATGEN)

if (WITH_OBEXGEN)
    # OBEX entries splitting
    add_executable(obex-vcard-split obex-vcard-split.c)
    add_coverage(obex-vcard-split)
    target_link_libraries(obex-vcard-split libGammu ${LIBINTL_LIBRARIES})

    # Tests
    add_test(obex-vcard-split-se-3
            "${GAMMU_TEST_PATH}/obex-vcard-split${CMAKE_EXECUTABLE_SUFFIX}"
            "${Gammu_SOURCE_DIR}/tests/vcards/se-3.vcf"
            499)

endif (WITH_OBEXGEN)

# SMS encoding
add_executable(sms-encode-decode sms-encode-decode.c)
add_coverage(sms-encode-decode)
target_link_libraries(sms-encode-decode libGammu ${LIBINTL_LIBRARIES})
target_link_libraries(sms-encode-decode messagedisplay)
add_test(sms-encode-decode "${GAMMU_TEST_PATH}/sms-encode-decode${CMAKE_EXECUTABLE_SUFFIX}")

# SMS encoding from commandline
add_executable(sms-cmdline sms-cmdline.c)
add_coverage(sms-cmdline)
target_link_libraries(sms-cmdline libGammu ${LIBINTL_LIBRARIES})
target_link_libraries(sms-cmdline messagedisplay)
target_link_libraries(sms-cmdline messagecmdline)

add_test(sms-cmdline-TEXT "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213 -text aaafdsfdfasd)
set_tests_properties(sms-cmdline-TEXT PROPERTIES
    PASS_REGULAR_EXPRESSION "Remote number        : \"213\"")

add_test(sms-cmdline-TEXT-smsc "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213 -text aaafdsfdfasd -smscnumber 132465)
set_tests_properties(sms-cmdline-TEXT-smsc PROPERTIES
PASS_REGULAR_EXPRESSION "Remote number        : \"213\"")

add_test(sms-cmdline-TEXT-textutf8 "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213 -unicode -textutf8 "Zkouška šíleně žlutého koně.")
set_tests_properties(sms-cmdline-TEXT-textutf8 PROPERTIES
    PASS_REGULAR_EXPRESSION "Coding               : Unicode \\(no compression\\)")


add_test(sms-cmdline-TEXT-16 "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213
        -text fdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdd
        -autolen 600 -16bit)
set_tests_properties(sms-cmdline-TEXT-16 PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(16 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-TEXT-autolen "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213
        -text fdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasddfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasddfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdd
        -autolen 200)
set_tests_properties(sms-cmdline-TEXT-autolen PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(8 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-TEXT-len "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213
        -text fdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdd
        -len 100)
set_tests_properties(sms-cmdline-TEXT-len PROPERTIES
    FAIL_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message")

add_test(sms-cmdline-TEXT-len-1 "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213
        -text fdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdfdasfasfasfdsafasdfasdffasdfasdfasaaafdsfdfasdd
        -len -1)
set_tests_properties(sms-cmdline-TEXT-len-1 PROPERTIES
    FAIL_REGULAR_EXPRESSION "Wrong message length \\(\"-1\"\\)"
    WILL_FAIL TRUE)

add_test(sms-cmdline-TEXT-unicode "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    TEXT 213 -text aaafdsfdfasd -unicode)
set_tests_properties(sms-cmdline-TEXT-unicode PROPERTIES
    PASS_REGULAR_EXPRESSION "Coding               : Unicode \\(no compression\\)")

add_test(sms-cmdline-TEXT-voice "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    text 213 -text aaafdsfdfasd -unicode -enablevoice)
set_tests_properties(sms-cmdline-TEXT-voice PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Enables voice indicator")

add_test(sms-cmdline-EMS "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    EMS 213 -text "aaafdsfdfasddsadsadsadsadijfdsalkfjla sdjflkjlfk fdsaf dsa fdsaf dsafafdasfsadfdsaf dsafdsa fdsaf dasfdsafdsafdsaf dsafdsaf daaafdsfdfasddsadsadsadsadijfdsalkfjla sdjflkjlfk fdsaf dsa fdsaf dsafafdasfsadfdsaf dsafdsa fdsaf dasfdsafdsafdsaf dsafdsaf dsafdsaf")
set_tests_properties(sms-cmdline-EMS PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(8 bit\\) [0-9]*, 2 parts")

add_test(sms-cmdline-EMS-empty "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    EMS 213)
set_tests_properties(sms-cmdline-EMS-empty PROPERTIES
    FAIL_REGULAR_EXPRESSION "Empty entry"
    WILL_FAIL TRUE)

add_test(sms-cmdline-EMS-misc "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
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

add_test(sms-cmdline-EMS-ring "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    EMS 54654654 -protected 1 -toneSElong  ${RINGTONE_TEST_FILE})

add_test(sms-cmdline-EMS-unicodefile "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    EMS 213 -unicodefiletext ${Gammu_SOURCE_DIR}/tests/text-unicode.txt -unicode)
set_tests_properties(sms-cmdline-EMS-unicodefile PROPERTIES
    PASS_REGULAR_EXPRESSION "Coding               : Unicode \\(no compression\\)")

add_test(sms-cmdline-RINGTONE "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    RINGTONE 54654654 ${RINGTONE_TEST_FILE})
set_tests_properties(sms-cmdline-RINGTONE PROPERTIES
    PASS_REGULAR_EXPRESSION "Ringtone \"AxelF\"")

add_test(sms-cmdline-MMS "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    MMSINDICATOR 800123456 https://wammu.eu/ Wammu Gammu)
set_tests_properties(sms-cmdline-MMS PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : MMS indicator, ID \\(8 bit\\) [0-9]*, part 1 of 1")

add_test(sms-cmdline-WAP "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    WAPINDICATOR 800123456 https://wammu.eu/ Wammu)
set_tests_properties(sms-cmdline-WAP PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : MMS indicator, ID \\(8 bit\\) [0-9]*, part 1 of 1")

if (WITH_BACKUP)
    add_test(sms-cmdline-TODO "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
        TODO 132132  ${Gammu_SOURCE_DIR}/tests/vcal/02.vcs  1)
    set_tests_properties(sms-cmdline-TODO PROPERTIES
        PASS_REGULAR_EXPRESSION "User Data Header     : Nokia calendar note, ID \\(8 bit\\) [0-9]*, part 2 of 2")

    add_test(sms-cmdline-CALENDAR "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
        CALENDAR 132132  ${Gammu_SOURCE_DIR}/tests/vcal/UK32Holidays.ics  30)
    set_tests_properties(sms-cmdline-CALENDAR PROPERTIES
        PASS_REGULAR_EXPRESSION "SUMMARY:Bank Holiday")

    add_test(sms-cmdline-VCARD "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
        VCARD21 132132  ${Gammu_SOURCE_DIR}/tests/vcards/yahoo.vcf ME 1)
    set_tests_properties(sms-cmdline-VCARD PROPERTIES
        PASS_REGULAR_EXPRESSION "User Data Header     : Concatenated \\(linked\\) message, ID \\(8 bit\\) [0-9]*, 2 parts")
endif (WITH_BACKUP)

add_test(sms-cmdline-SMSTEMPLATE "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    SMSTEMPLATE 213 -text aaafdsfdfasd)
# Not sure if this is really correct...
set_tests_properties(sms-cmdline-SMSTEMPLATE PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : User UDH")

add_test(sms-cmdline-PICTURE "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    PICTURE 213 ${LOGO_TEST_FILE})
set_tests_properties(sms-cmdline-PICTURE PROPERTIES
    PASS_REGULAR_EXPRESSION "###################   ##    ##     #     #     ##   ## ### ##   ###   ##")

add_test(sms-cmdline-OPERATOR "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    OPERATOR 213 ${LOGO_TEST_FILE} -netcode "230 03")
set_tests_properties(sms-cmdline-OPERATOR PROPERTIES
    PASS_REGULAR_EXPRESSION "Operator logo for 230 03 network \\(Vodafone, Czech Republic\\)")

add_test(sms-cmdline-CALLER "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    CALLER 213 ${LOGO_TEST_FILE})
set_tests_properties(sms-cmdline-CALLER PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia caller logo")

add_test(sms-cmdline-ANIMATION "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    ANIMATION 213 6
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE}
        ${LOGO_TEST_FILE})
set_tests_properties(sms-cmdline-ANIMATION PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of messages: 14")

add_test(sms-cmdline-PROFILE "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
    PROFILE 213
        -bitmap ${LOGO_TEST_FILE}
        -ringtone ${RINGTONE_TEST_FILE})
set_tests_properties(sms-cmdline-PROFILE PROPERTIES
    PASS_REGULAR_EXPRESSION "User Data Header     : Nokia profile, ID \\(8 bit\\) [0-9]*, 3 parts")

if (WITH_BACKUP)
    add_test(sms-cmdline-BOOKMARK "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
        BOOKMARK 213 ${Gammu_SOURCE_DIR}/tests/misc/wap-bookmark.backup 1)
    set_tests_properties(sms-cmdline-BOOKMARK PROPERTIES
        PASS_REGULAR_EXPRESSION "User Data Header     : Nokia WAP bookmark")

    add_test(sms-cmdline-WAPSETTINGS "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
        WAPSETTINGS 213 ${Gammu_SOURCE_DIR}/tests/misc/wap-settings.backup 1 GPRS)
    set_tests_properties(sms-cmdline-WAPSETTINGS PROPERTIES
        PASS_REGULAR_EXPRESSION "User Data Header     : Nokia WAP bookmark or WAP/MMS settings, ID \\(8 bit\\) [0-9]*, 2 parts")

    add_test(sms-cmdline-MMSSETTINGS "${GAMMU_TEST_PATH}/sms-cmdline${CMAKE_EXECUTABLE_SUFFIX}"
        MMSSETTINGS 213 ${Gammu_SOURCE_DIR}/tests/misc/mms-settings.backup 1)
    set_tests_properties(sms-cmdline-MMSSETTINGS PROPERTIES
        PASS_REGULAR_EXPRESSION "User Data Header     : Nokia WAP bookmark or WAP/MMS settings, ID \\(8 bit\\) [0-9]*, 2 parts")
endif (WITH_BACKUP)

if (WITH_ATGEN)
    # Samsung parser tests
    add_executable(at-samsung at-samsung.c)
    add_coverage(at-samsung)
    target_link_libraries(at-samsung libGammu ${LIBINTL_LIBRARIES})
    add_test(at-samsung "${GAMMU_TEST_PATH}/at-samsung${CMAKE_EXECUTABLE_SUFFIX}")

    # Motorola parser tests
    add_executable(at-motorola at-motorola.c)
    add_coverage(at-motorola)
    target_link_libraries(at-motorola libGammu ${LIBINTL_LIBRARIES})
    add_test(at-motorola "${GAMMU_TEST_PATH}/at-motorola${CMAKE_EXECUTABLE_SUFFIX}")

    # AT parser tests
    add_executable(at-parser at-parser.c)
    add_coverage(at-parser)
    target_link_libraries(at-parser libGammu ${LIBINTL_LIBRARIES})
    add_test(at-parser "${GAMMU_TEST_PATH}/at-parser${CMAKE_EXECUTABLE_SUFFIX}")

    # AT dispatch tests
    add_executable(at-dispatch at-dispatch.c)
    add_coverage(at-dispatch)
    target_link_libraries(at-dispatch libGammu ${LIBINTL_LIBRARIES})
    add_test(at-dispatch "${GAMMU_TEST_PATH}/at-dispatch${CMAKE_EXECUTABLE_SUFFIX}")

    # AT text encoding/decoding
    add_executable(at-charset at-charset.c)
    add_coverage(at-charset)
    target_link_libraries(at-charset libGammu ${LIBINTL_LIBRARIES})
    add_test(at-charset "${GAMMU_TEST_PATH}/at-charset${CMAKE_EXECUTABLE_SUFFIX}")

    # AT statemachine
    add_executable(at-statemachine at-statemachine.c)
    add_coverage(at-statemachine)
    target_link_libraries(at-statemachine libGammu ${LIBINTL_LIBRARIES})
    add_test(at-statemachine "${GAMMU_TEST_PATH}/at-statemachine${CMAKE_EXECUTABLE_SUFFIX}")
endif (WITH_ATGEN)

# Line parser tests
add_executable(line-splitting line-splitting.c)
add_coverage(line-splitting)
target_link_libraries(line-splitting libGammu ${LIBINTL_LIBRARIES})
add_test(line-splitting "${GAMMU_TEST_PATH}/line-splitting${CMAKE_EXECUTABLE_SUFFIX}")

# Config file parsing
add_executable(config config.c)
add_coverage(config)
target_link_libraries(config libGammu ${LIBINTL_LIBRARIES})
file(GLOB CONFIGS
    RELATIVE "${Gammu_SOURCE_DIR}/tests/configs"
    "${Gammu_SOURCE_DIR}/tests/configs/tabs-*")
list(SORT CONFIGS)

foreach(TESTCONFIG ${CONFIGS})
    add_test(config-${TESTCONFIG} "${GAMMU_TEST_PATH}/config${CMAKE_EXECUTABLE_SUFFIX}" "${Gammu_SOURCE_DIR}/tests/configs/${TESTCONFIG}")
    set_tests_properties(config-${TESTCONFIG} PROPERTIES
        PASS_REGULAR_EXPRESSION "DEBUG_LEVEL: 'textall'")
endforeach(TESTCONFIG $CONFIGS)


# INI file parsing
add_executable(inifile inifile.c)
add_coverage(inifile)
target_link_libraries(inifile libGammu ${LIBINTL_LIBRARIES})
file(GLOB INIFILES
    RELATIVE "${Gammu_SOURCE_DIR}/tests/inifiles"
    "${Gammu_SOURCE_DIR}/tests/inifiles/*.ini")
list(SORT INIFILES)

foreach(TESTINIFILE ${INIFILES})
    add_test(inifile-${TESTINIFILE} "${GAMMU_TEST_PATH}/inifile${CMAKE_EXECUTABLE_SUFFIX}" "${Gammu_SOURCE_DIR}/tests/inifiles/${TESTINIFILE}")
endforeach(TESTINIFILE $INIFILES)

# Examples
add_executable(phone-info "${Gammu_SOURCE_DIR}/docs/examples/phone-info.c")
add_coverage(phone-info)
target_link_libraries(phone-info libGammu ${LIBINTL_LIBRARIES})
add_executable(custom-config "${Gammu_SOURCE_DIR}/docs/examples/custom-config.c")
add_coverage(custom-config)
target_link_libraries(custom-config libGammu ${LIBINTL_LIBRARIES})
add_executable(sms-send "${Gammu_SOURCE_DIR}/docs/examples/sms-send.c")
add_coverage(sms-send)
target_link_libraries(sms-send libGammu ${LIBINTL_LIBRARIES})
add_executable(long-sms "${Gammu_SOURCE_DIR}/docs/examples/long-sms.c")
add_coverage(long-sms)
target_link_libraries(long-sms libGammu ${LIBINTL_LIBRARIES})
add_executable(sms-read "${Gammu_SOURCE_DIR}/docs/examples/sms-read.c")
add_coverage(sms-read)
target_link_libraries(sms-read libGammu ${LIBINTL_LIBRARIES})
add_executable(smsd "${Gammu_SOURCE_DIR}/docs/examples/smsd.c")
add_coverage(smsd)
target_link_libraries(smsd libGammu ${LIBINTL_LIBRARIES} gsmsd)

# Examples tests, works with dummy phone
if (WITH_BACKUP)
    add_test(phone-info "${GAMMU_TEST_PATH}/phone-info${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/.gammurc")
    add_test(custom-config "${GAMMU_TEST_PATH}/custom-config${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/.gammu-dummy" none dummy)
    add_test(sms-send "${GAMMU_TEST_PATH}/sms-send${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/.gammurc")
    add_test(long-sms "${GAMMU_TEST_PATH}/long-sms${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/.gammurc")
    add_test(sms-read "${GAMMU_TEST_PATH}/sms-read${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/.gammurc")
endif (WITH_BACKUP)


# Auto generated include tests begin
# Do not modify this section, change gen-include-test.sh instead

# Test for header gammu-backup.h
add_executable(include-backup include-backup.c)
add_coverage(include-backup)
target_link_libraries(include-backup libGammu ${LIBINTL_LIBRARIES})
add_test(include-backup "${GAMMU_TEST_PATH}/include-backup${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-bitmap.h
add_executable(include-bitmap include-bitmap.c)
add_coverage(include-bitmap)
target_link_libraries(include-bitmap libGammu ${LIBINTL_LIBRARIES})
add_test(include-bitmap "${GAMMU_TEST_PATH}/include-bitmap${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-calendar.h
add_executable(include-calendar include-calendar.c)
add_coverage(include-calendar)
target_link_libraries(include-calendar libGammu ${LIBINTL_LIBRARIES})
add_test(include-calendar "${GAMMU_TEST_PATH}/include-calendar${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-callback.h
add_executable(include-callback include-callback.c)
add_coverage(include-callback)
target_link_libraries(include-callback libGammu ${LIBINTL_LIBRARIES})
add_test(include-callback "${GAMMU_TEST_PATH}/include-callback${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-call.h
add_executable(include-call include-call.c)
add_coverage(include-call)
target_link_libraries(include-call libGammu ${LIBINTL_LIBRARIES})
add_test(include-call "${GAMMU_TEST_PATH}/include-call${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-category.h
add_executable(include-category include-category.c)
add_coverage(include-category)
target_link_libraries(include-category libGammu ${LIBINTL_LIBRARIES})
add_test(include-category "${GAMMU_TEST_PATH}/include-category${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-datetime.h
add_executable(include-datetime include-datetime.c)
add_coverage(include-datetime)
target_link_libraries(include-datetime libGammu ${LIBINTL_LIBRARIES})
add_test(include-datetime "${GAMMU_TEST_PATH}/include-datetime${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-debug.h
add_executable(include-debug include-debug.c)
add_coverage(include-debug)
target_link_libraries(include-debug libGammu ${LIBINTL_LIBRARIES})
add_test(include-debug "${GAMMU_TEST_PATH}/include-debug${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-error.h
add_executable(include-error include-error.c)
add_coverage(include-error)
target_link_libraries(include-error libGammu ${LIBINTL_LIBRARIES})
add_test(include-error "${GAMMU_TEST_PATH}/include-error${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-file.h
add_executable(include-file include-file.c)
add_coverage(include-file)
target_link_libraries(include-file libGammu ${LIBINTL_LIBRARIES})
add_test(include-file "${GAMMU_TEST_PATH}/include-file${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-info.h
add_executable(include-info include-info.c)
add_coverage(include-info)
target_link_libraries(include-info libGammu ${LIBINTL_LIBRARIES})
add_test(include-info "${GAMMU_TEST_PATH}/include-info${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-inifile.h
add_executable(include-inifile include-inifile.c)
add_coverage(include-inifile)
target_link_libraries(include-inifile libGammu ${LIBINTL_LIBRARIES})
add_test(include-inifile "${GAMMU_TEST_PATH}/include-inifile${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-keys.h
add_executable(include-keys include-keys.c)
add_coverage(include-keys)
target_link_libraries(include-keys libGammu ${LIBINTL_LIBRARIES})
add_test(include-keys "${GAMMU_TEST_PATH}/include-keys${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-limits.h
add_executable(include-limits include-limits.c)
add_coverage(include-limits)
target_link_libraries(include-limits libGammu ${LIBINTL_LIBRARIES})
add_test(include-limits "${GAMMU_TEST_PATH}/include-limits${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-memory.h
add_executable(include-memory include-memory.c)
add_coverage(include-memory)
target_link_libraries(include-memory libGammu ${LIBINTL_LIBRARIES})
add_test(include-memory "${GAMMU_TEST_PATH}/include-memory${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-message.h
add_executable(include-message include-message.c)
add_coverage(include-message)
target_link_libraries(include-message libGammu ${LIBINTL_LIBRARIES})
add_test(include-message "${GAMMU_TEST_PATH}/include-message${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-misc.h
add_executable(include-misc include-misc.c)
add_coverage(include-misc)
target_link_libraries(include-misc libGammu ${LIBINTL_LIBRARIES})
add_test(include-misc "${GAMMU_TEST_PATH}/include-misc${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-nokia.h
add_executable(include-nokia include-nokia.c)
add_coverage(include-nokia)
target_link_libraries(include-nokia libGammu ${LIBINTL_LIBRARIES})
add_test(include-nokia "${GAMMU_TEST_PATH}/include-nokia${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-ringtone.h
add_executable(include-ringtone include-ringtone.c)
add_coverage(include-ringtone)
target_link_libraries(include-ringtone libGammu ${LIBINTL_LIBRARIES})
add_test(include-ringtone "${GAMMU_TEST_PATH}/include-ringtone${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-security.h
add_executable(include-security include-security.c)
add_coverage(include-security)
target_link_libraries(include-security libGammu ${LIBINTL_LIBRARIES})
add_test(include-security "${GAMMU_TEST_PATH}/include-security${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-settings.h
add_executable(include-settings include-settings.c)
add_coverage(include-settings)
target_link_libraries(include-settings libGammu ${LIBINTL_LIBRARIES})
add_test(include-settings "${GAMMU_TEST_PATH}/include-settings${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-smsd.h
add_executable(include-smsd include-smsd.c)
add_coverage(include-smsd)
target_link_libraries(include-smsd libGammu ${LIBINTL_LIBRARIES})
add_test(include-smsd "${GAMMU_TEST_PATH}/include-smsd${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-statemachine.h
add_executable(include-statemachine include-statemachine.c)
add_coverage(include-statemachine)
target_link_libraries(include-statemachine libGammu ${LIBINTL_LIBRARIES})
add_test(include-statemachine "${GAMMU_TEST_PATH}/include-statemachine${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-types.h
add_executable(include-types include-types.c)
add_coverage(include-types)
target_link_libraries(include-types libGammu ${LIBINTL_LIBRARIES})
add_test(include-types "${GAMMU_TEST_PATH}/include-types${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-unicode.h
add_executable(include-unicode include-unicode.c)
add_coverage(include-unicode)
target_link_libraries(include-unicode libGammu ${LIBINTL_LIBRARIES})
add_test(include-unicode "${GAMMU_TEST_PATH}/include-unicode${CMAKE_EXECUTABLE_SUFFIX}")

# Test for header gammu-wap.h
add_executable(include-wap include-wap.c)
add_coverage(include-wap)
target_link_libraries(include-wap libGammu ${LIBINTL_LIBRARIES})
add_test(include-wap "${GAMMU_TEST_PATH}/include-wap${CMAKE_EXECUTABLE_SUFFIX}")

# Auto generated include tests end

# Test for GetInt
add_executable(getint getint.c)
add_coverage(getint)
target_link_libraries(getint cmdline)
set(GETINT_PASS 1 23213 3213213 -321312 -1)
set(GETINT_FAIL 1a 24432f xxx foo 3243243243243243232432432432432)
foreach(NUM ${GETINT_PASS})
    add_test(getint-${NUM} "${GAMMU_TEST_PATH}/getint${CMAKE_EXECUTABLE_SUFFIX}" "${NUM}")
    set_tests_properties(
        getint-${NUM}
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Result: ${NUM}")
endforeach(NUM ${GETINT_PASS})
foreach(NUM ${GETINT_FAIL})
    add_test(getint-${NUM} "${GAMMU_TEST_PATH}/getint${CMAKE_EXECUTABLE_SUFFIX}" "${NUM}")
    set_tests_properties(
        getint-${NUM}
        PROPERTIES WILL_FAIL TRUE)
endforeach(NUM ${GETINT_FAIL})

# Test for locking, only on !WIN32 and if we can write to lock dir
if (NOT WIN32)
    execute_process(COMMAND test -w /var/lock/ RESULT_VARIABLE VAR_LOCK_WRITABLE)
    if (VAR_LOCK_WRITABLE EQUAL 0)
        add_executable(locking locking.c)
        add_coverage(locking)
        target_link_libraries(locking libGammu ${LIBINTL_LIBRARIES})
        add_test(locking "${GAMMU_TEST_PATH}/locking${CMAKE_EXECUTABLE_SUFFIX}")
    else (VAR_LOCK_WRITABLE EQUAL 0)
        message("/var/lock is not writable, skipping locking tests!")
    endif (VAR_LOCK_WRITABLE EQUAL 0)
endif (NOT WIN32)

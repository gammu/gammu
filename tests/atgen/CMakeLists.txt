add_library(atgen-tests STATIC test_helper.c)
target_link_libraries(atgen-tests libGammu)

if(MYSQL_INCLUDE_DIR)
    include_directories(${MYSQL_INCLUDE_DIR})
endif()

if(POSTGRES_INCLUDE_DIR)
    include_directories(${POSTGRES_INCLUDE_DIR})
endif()

MACRO(atgen_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    add_coverage(${TEST_NAME})
    target_link_libraries(${TEST_NAME} atgen-tests ${LIBINTL_LIBRARIES})
    add_test(${TEST_NAME} "${GAMMU_TEST_PATH}/atgen/${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
ENDMACRO(atgen_test)

MACRO(smsd_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    add_coverage(${TEST_NAME})
    target_link_libraries(${TEST_NAME} atgen-tests gsmsd ${LIBINTL_LIBRARIES})
    add_test(${TEST_NAME} "${GAMMU_TEST_PATH}/atgen/${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
ENDMACRO(smsd_test)

atgen_test(is-memory-enabled)
atgen_test(is-memory-writeable)
atgen_test(set-requested-sms-memory)
atgen_test(incoming-sms-info)
atgen_test(incoming-sms)

atgen_test(gsm-set-cnmi-params)
atgen_test(get-sms-location)
atgen_test(get-sms)

smsd_test(smsd-incoming-cds)

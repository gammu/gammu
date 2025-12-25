project(atgen-tests C)

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

configure_file(smsd.cfg.in ${PROJECT_BINARY_DIR}/smsd.cfg)

add_custom_command(TARGET atgen-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/inbox
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/outbox
        )

atgen_test(is-memory-enabled)
atgen_test(is-memory-writeable)
atgen_test(set-requested-sms-memory)

atgen_test(deferred-event)
atgen_test(incoming-sms-info)
atgen_test(incoming-sms)

atgen_test(gsm-set-cnmi-params)
atgen_test(test-reply-get-cnmi)
atgen_test(get-sms-location)
atgen_test(get-sms)

if (HAVE_MYSQL_MYSQL_H OR LIBDBI_FOUND OR HAVE_POSTGRESQL_LIBPQ_FE_H OR ODBC_FOUND)
smsd_test(test_sql_time)
endif (HAVE_MYSQL_MYSQL_H OR LIBDBI_FOUND OR HAVE_POSTGRESQL_LIBPQ_FE_H OR ODBC_FOUND)

smsd_test(smsd-incoming-cds)
smsd_test(smsd-cancel-call)
smsd_test(smsd-incoming-ussd)
smsd_test(smsd-1-folder-getnextsms)

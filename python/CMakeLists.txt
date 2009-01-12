# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (python-gammu C)

# Python has defined these functions, but MinGW does also have them...
if (MINGW)
    check_symbol_exists (acosh "math.h" HAVE_ACOSH)
    if (HAVE_ACOSH)
        add_definitions(-DHAVE_ACOSH)
    endif (HAVE_ACOSH)
    check_symbol_exists (asinh "math.h" HAVE_ASINH)
    if (HAVE_ASINH)
        add_definitions(-DHAVE_ASINH)
    endif (HAVE_ASINH)
    check_symbol_exists (atanh "math.h" HAVE_ATANH)
    if (HAVE_ATANH)
        add_definitions(-DHAVE_ATANH)
    endif (HAVE_ATANH)
    check_symbol_exists (log1p "math.h" HAVE_LOG1P)
    if (HAVE_LOG1P)
        add_definitions(-DHAVE_LOG1P)
    endif (HAVE_LOG1P)
endif (MINGW)

set (PYTHON_GAMMU_SRC
    gammu.c
    misc.c
    errors.c
    convertors/backup.c
    convertors/base.c
    convertors/bitmap.c
    convertors/calendar.c
    convertors/call.c
    convertors/file.c
    convertors/memory.c
    convertors/misc.c
    convertors/ringtone.c
    convertors/sms.c
    convertors/string.c
    convertors/time.c
    convertors/todo.c
    convertors/wap.c
    )

execute_process(
    COMMAND
        ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print sysconfig.get_python_lib(plat_specific=1,prefix='${CMAKE_INSTALL_EXEC_PREFIX}')"
    OUTPUT_VARIABLE PYTHON_SITEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

include_directories (
    "${Gammu_BINARY_DIR}/include"
    ${PYTHON_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

add_library(python_gammu MODULE ${PYTHON_GAMMU_SRC})
set_target_properties (python_gammu PROPERTIES
    PREFIX ""
    OUTPUT_NAME gammu/_gammu)
if (WIN32 AND NOT CYGWIN)
    set_target_properties (python_gammu PROPERTIES
         SUFFIX ".pyd"
         )
endif (WIN32 AND NOT CYGWIN)
target_link_libraries (python_gammu libGammu)
target_link_libraries (python_gammu ${PYTHON_LIBRARIES})
install(TARGETS python_gammu LIBRARY DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")
file(GLOB PYFILES "gammu/*.py")
foreach(PYFILE ${PYFILES})
    get_filename_component(FILENAME "${PYFILE}" NAME)
    configure_file("${PYFILE}" "${CMAKE_CURRENT_BINARY_DIR}/gammu/${FILENAME}" COPY_ONLY)
endforeach(PYFILE)
install(FILES ${PYFILES} DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")

# Testing
# Scripts need to be in same directory as built gammu module!
configure_file("examples/backup-convertor.py" "${CMAKE_CURRENT_BINARY_DIR}/test-backup-convertor.py" COPY_ONLY)
file(GLOB VFORMATS "${CMAKE_CURRENT_SOURCE_DIR}/../tests/vcards/*.vcf"
    "${CMAKE_CURRENT_SOURCE_DIR}/../tests/vcal/*.ics"
    "${CMAKE_CURRENT_SOURCE_DIR}/../tests/vcal/*.vcs")
foreach(VCF ${VFORMATS})
    get_filename_component(TESTNAME "${VCF}" NAME_WE)
    get_filename_component(TYPE "${VCF}" EXT)
    add_test("py-backup-${TESTNAME}" ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/test-backup-convertor.py" "${VCF}" "${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.backup")
    if (${TYPE} STREQUAL .vcf)
        foreach (EXT lmb vcf ldif)
            add_test("py-backup-${TESTNAME}-${EXT}" ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/test-backup-convertor.py" "${VCF}" "${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.${EXT}")
        endforeach (EXT)
    else (${TYPE} STREQUAL .vcf)
        foreach (EXT vcs ics)
            add_test("py-backup-${TESTNAME}-${EXT}" ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/test-backup-convertor.py" "${VCF}" "${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.${EXT}")
        endforeach (EXT)
    endif (${TYPE} STREQUAL .vcf)
endforeach(VCF)

configure_file("examples/sms-encode-decode.py" "${CMAKE_CURRENT_BINARY_DIR}/test-sms-encode-decode.py" COPY_ONLY)
add_test("py-sms-encode-decode" ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/test-sms-encode-decode.py")
set_tests_properties("py-sms-encode-decode" PROPERTIES PASS_REGULAR_EXPRESSION "Comparsion: True")

configure_file("examples/sms-link.py" "${CMAKE_CURRENT_BINARY_DIR}/test-sms-link.py" COPY_ONLY)
add_test("py-sms-link" ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/test-sms-link.py")
set_tests_properties("py-sms-link" PROPERTIES PASS_REGULAR_EXPRESSION "Comparsion: True")

# Documentation
find_program(EPYDOC_EXECUTABLE NAMES epydoc
    DOC "Epydoc tool for generating Python API documentation <http://epydoc.sourceforge.net/>")
if (EPYDOC_EXECUTABLE)
    add_custom_target (pydoc ${EPYDOC_EXECUTABLE} --verbose --name python-gammu --url http://cihar.com/gammu/python/ gammu gammu._gammu
        DEPENDS ${PYFILES} ${PYTHON_GAMMU_SRC}
        COMMENT "Generating Python documentation")
endif (EPYDOC_EXECUTABLE)

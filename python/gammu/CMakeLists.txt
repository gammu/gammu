# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (python-gammu-module C)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_MINGW OR
    CMAKE_COMPILER_IS_CYGWIN OR CMAKE_COMPILER_IS_GNUCXX)
# Needed for Python at least for PyObject and PyTypeObject handling
MACRO_TUNE_COMPILER("-fno-strict-aliasing")
endif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_MINGW OR
    CMAKE_COMPILER_IS_CYGWIN OR CMAKE_COMPILER_IS_GNUCXX)

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

# Python headers include pragma comments for linking, but
# these are broken for debug builds and we do add library
# anyway, so it's safer to disable that.
if(MSVC)
    add_definitions(-DPy_NO_ENABLE_SHARED)
endif(MSVC)

execute_process(
    COMMAND
        ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print sysconfig.get_python_lib(plat_specific=1,prefix='${CMAKE_INSTALL_EXEC_PREFIX}')"
    OUTPUT_VARIABLE PYTHON_SITEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

set (PYTHON_GAMMU_SRC
    src/gammu.c
    src/smsd.c
    src/misc.c
    src/errors.c
    src/convertors/backup.c
    src/convertors/base.c
    src/convertors/bitmap.c
    src/convertors/calendar.c
    src/convertors/call.c
    src/convertors/file.c
    src/convertors/memory.c
    src/convertors/misc.c
    src/convertors/ringtone.c
    src/convertors/sms.c
    src/convertors/string.c
    src/convertors/time.c
    src/convertors/todo.c
    src/convertors/wap.c
    )

execute_process(
    COMMAND
        ${PYTHON_EXECUTABLE} -c "import sys
if hasattr(sys, 'pydebug'):
    print sys.pydebug"
    OUTPUT_VARIABLE PYTHON_DEBUG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

include_directories (
    "${Gammu_BINARY_DIR}/include"
    "${Gammu_SOURCE_DIR}/python/include"
    ${PYTHON_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

add_library(python_gammu MODULE ${PYTHON_GAMMU_SRC})
set_target_properties (python_gammu PROPERTIES
    PREFIX ""
    INSTALL_RPATH ""
    INSTALL_RPATH_USE_LINK_PATH OFF
    OUTPUT_NAME _gammu)

if (PYTHON_DEBUG STREQUAL True)
    set_target_properties (python_gammu PROPERTIES
        OUTPUT_NAME _gammu_d)
endif (PYTHON_DEBUG STREQUAL True)

if (WIN32 AND NOT CYGWIN)
    set_target_properties (python_gammu PROPERTIES
         SUFFIX ".pyd"
         )
endif (WIN32 AND NOT CYGWIN)
target_link_libraries (python_gammu libGammu)
target_link_libraries (python_gammu gsmsd)
target_link_libraries (python_gammu ${PYTHON_LIBRARIES})
install(TARGETS python_gammu LIBRARY DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")

file(GLOB PYFILES "*.py")
foreach(PYFILE ${PYFILES})
    get_filename_component(FILENAME "${PYFILE}" NAME)
    configure_file("${PYFILE}" "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}" COPY_ONLY)
endforeach(PYFILE)
install(FILES ${PYFILES} DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")

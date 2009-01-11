# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (python-gammu C)

find_package(PythonInterp)
find_package(PythonLibs)

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

if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
    set (ENABLE_PYTHON TRUE CACHE BOOL "Build Python bindings")
    set (CPACK_SYSTEM_NAME "Windows" CACHE STRING "Windows package name")
else (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
    set (ENABLE_PYTHON FALSE CACHE BOOL "Build Python bindings" FORCE)
endif (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)

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

if (ENABLE_PYTHON)
    execute_process(
        COMMAND
            ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print sysconfig.get_python_lib(1,0,prefix='${CMAKE_INSTALL_EXEC_PREFIX}')"
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
        OUTPUT_NAME _gammu)
    if (WIN32 AND NOT CYGWIN)
        set_target_properties (python_gammu PROPERTIES
             SUFFIX ".pyd"
             )
    endif (WIN32 AND NOT CYGWIN)
    target_link_libraries (python_gammu libGammu)
    target_link_libraries (python_gammu ${PYTHON_LIBRARIES})
    install(TARGETS python_gammu LIBRARY DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")
    file(GLOB PYFILES "gammu/*.py")
    install(FILES ${PYFILES} DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")
endif (ENABLE_PYTHON)

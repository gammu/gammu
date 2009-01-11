# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

# I didn't check older versions
# However 2.6 is needed for crosscompiling
cmake_minimum_required (VERSION 2.4.2)

project (python-gammu C)

find_package(PythonInterp)
find_package(PythonLibs)

if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
    set (ENABLE_PYTHON TRUE CACHE BOOL "Build Python bindings")
    set (CPACK_SYSTEM_NAME "Windows" CACHE STRING "Windows package name")
else (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
    set (ENABLE_PYTHON FALSE CACHE BOOL "Build Python bindings" FORCE)
endif (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)

set (PYTHON_GAMMU_SRC
    gammu.c
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
        OUTPUT_NAME Core)
    target_link_libraries (python_gammu libGammu)
    target_link_libraries (python_gammu ${PYTHON_LIBRARIES})
    install(TARGETS python_gammu LIBRARY DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")
    file(GLOB PYFILES "gammu/*.py")
    install(FILES ${PYFILES} DESTINATION "${PYTHON_SITEDIR}/gammu/" COMPONENT "python")
endif (ENABLE_PYTHON)

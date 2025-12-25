# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
# Lot of adustmens by Michal Cihar <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:
#
# Redistribution and use is allowed according to the terms of the BSD license.


if(UNIX)
    set(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
        "preferred path to MySQL (mysql_config)")
    find_program(MYSQL_CONFIG mysql_config
        ${MYSQL_CONFIG_PREFER_PATH}
        /usr/local/mysql/bin/
        /usr/local/bin/
        /usr/bin/
        )

    if(MYSQL_CONFIG)
        message(STATUS "Using mysql-config: ${MYSQL_CONFIG}")
        # set INCLUDE_DIR
        exec_program(${MYSQL_CONFIG}
            ARGS --include
            OUTPUT_VARIABLE MY_TMP)

        string(REGEX REPLACE "-I([^ ]+)( .*)?" "\\1" MY_TMP "${MY_TMP}")

        set(MYSQL_ADD_INCLUDE_DIR ${MY_TMP} CACHE FILEPATH INTERNAL)

        # set LIBRARY_DIR
        exec_program(${MYSQL_CONFIG}
            ARGS --libs
            OUTPUT_VARIABLE MY_TMP)

        set(MYSQL_ADD_LIBRARIES "")

        string(REGEX MATCHALL "(^| )-l[^ ]+" MYSQL_LIB_LIST "${MY_TMP}")
        foreach(LIB ${MYSQL_LIB_LIST})
            string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
            list(APPEND MYSQL_ADD_LIBRARIES "${LIB}")
        endforeach(LIB ${MYSQL_LIBS})

        set(MYSQL_ADD_LIBRARY_PATH "")

        string(REGEX MATCHALL "-L[^ ]+" MYSQL_LIBDIR_LIST "${MY_TMP}")
        foreach(LIB ${MYSQL_LIBDIR_LIST})
            string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
            list(APPEND MYSQL_ADD_LIBRARY_PATH "${LIB}")
        endforeach(LIB ${MYSQL_LIBS})

    else(MYSQL_CONFIG)
        set(MYSQL_ADD_LIBRARIES "")
        list(APPEND MYSQL_ADD_LIBRARIES "mysqlclient")
    endif(MYSQL_CONFIG)
else(UNIX)
    if (WIN32)
        set(MYSQL_ADD_LIBRARIES "")
        list(APPEND MYSQL_ADD_LIBRARIES "mysqlclient")
    endif (WIN32)
    set(MYSQL_ADD_INCLUDE_DIR "c:/msys/local/include" CACHE FILEPATH INTERNAL)
    set(MYSQL_ADD_LIBRARY_PATH "c:/msys/local/lib" CACHE FILEPATH INTERNAL)
ENDIF(UNIX)

if (WIN32)
    find_path(MYSQL_INCLUDE_DIR mysql.h
        /usr/local/include
        /usr/local/include/mysql
        /usr/local/mysql/include
        /usr/local/mysql/include/mysql
        /usr/include
        /usr/include/mysql
        $ENV{MYSQL_DIR}/include
        $ENV{ProgramFiles}/MySQL/*/include
        $ENV{SystemDrive}/MySQL/*/include
        "C:/Program Files/MySQL/*/include"
        ${MYSQL_ADD_INCLUDE_DIR}
    )
else()
    find_path(MYSQL_INCLUDE_DIR mysql.h
        /usr/local/include
        /usr/local/include/mysql
        /usr/local/mysql/include
        /usr/local/mysql/include/mysql
        /usr/include
        /usr/include/mysql
        ${MYSQL_ADD_INCLUDE_DIR}
    )
endif()

set(TMP_MYSQL_LIBRARIES "")

if (WIN32)
    foreach(LIB ${MYSQL_ADD_LIBRARIES})
        find_library("MYSQL_LIBRARIES_${LIB}" NAMES ${LIB}
            PATHS
            ${MYSQL_ADD_LIBRARY_PATH}
            /usr/lib/mysql
            /usr/local/lib
            /usr/local/lib/mysql
            /usr/local/mysql/lib
            $ENV{MYSQL_DIR}/lib/opt
            $ENV{ProgramFiles}/MySQL/*/lib
            $ENV{SystemDrive}/MySQL/*/lib
            "C:/Program Files/MySQL/*/lib"
            $ENV{ProgramFiles}/MySQL/*/lib/opt
            $ENV{SystemDrive}/MySQL/*/lib/opt
            "C:/Program Files/MySQL/*/lib/opt"
        )
        list(APPEND TMP_MYSQL_LIBRARIES "${MYSQL_LIBRARIES_${LIB}}")
    endforeach(LIB ${MYSQL_ADD_LIBRARIES})
else()
    find_library("MYSQL_LIBRARIES_mysqlclient" NAMES mysqlclient mariadbclient
        PATHS
        ${MYSQL_ADD_LIBRARY_PATH}
        /usr/lib/mysql
        /usr/local/lib
        /usr/local/lib/mysql
        /usr/local/mysql/lib
    )

    list(APPEND TMP_MYSQL_LIBRARIES "${MYSQL_LIBRARIES_mysqlclient}")
endif()

if (TMP_MYSQL_LIBRARIES)
    set(MYSQL_LIBRARIES ${TMP_MYSQL_LIBRARIES} CACHE FILEPATH "MySQL Libraries" FORCE)
else (TMP_MYSQL_LIBRARIES)
    set(MYSQL_LIBRARIES ${TMP_MYSQL_LIBRARIES} CACHE FILEPATH "MySQL Libraries")
endif (TMP_MYSQL_LIBRARIES)


if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    include(MacroCheckLibraryWorks)
    CHECK_LIBRARY_WORKS("mysql.h" "mysql_errno(0);" "${MYSQL_INCLUDE_DIR}" "${MYSQL_LIBRARIES}" "MYSQL_WORKS")

    set(MYSQL_FOUND ${MYSQL_WORKS} CACHE INTERNAL "MySQL found")
    message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND FALSE CACHE INTERNAL "MySQL found")
    message(STATUS "MySQL not found.")
endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)

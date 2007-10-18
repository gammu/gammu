# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF(UNIX) 
    SET(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
		"preferred path to MySQL (mysql_config)")
    FIND_PROGRAM(MYSQL_CONFIG mysql_config
      ${MYSQL_CONFIG_PREFER_PATH}
	  /usr/local/mysql/bin/
	  /usr/local/bin/
      /usr/bin/
      )
    
    IF (MYSQL_CONFIG) 
      # set INCLUDE_DIR
      EXEC_PROGRAM(${MYSQL_CONFIG}
        ARGS --include
        OUTPUT_VARIABLE MY_TMP)
	  string (REGEX REPLACE "-I([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")
      SET(MYSQL_ADD_INCLUDE_DIR ${MY_TMP} CACHE FILEPATH INTERNAL)

      # set LIBRARY_DIR
      EXEC_PROGRAM(${MYSQL_CONFIG}
        ARGS --libs
        OUTPUT_VARIABLE MY_TMP)
	  string (REGEX REPLACE "-L([^ ]*)( .*)?" "\\2" MYSQL_LIBS "${MY_TMP}")
	  set (MYSQL_ADD_LIBRARIES "")
	  foreach(LIB ${MYSQL_LIBS})
		string (REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
		list(APPEND MYSQL_ADD_LIBRARIES "${LIB}")
	  endforeach(LIB ${MYSQL_LIBS})
	  string (REGEX REPLACE "-L([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")
      SET(MYSQL_ADD_LIBRARY_PATH ${MY_TMP} CACHE FILEPATH INTERNAL)
    else(MYSQL_CONFIG)
        set (MYSQL_ADD_LIBRARIES "")
        list(APPEND MYSQL_ADD_LIBRARIES "mysqlclient")
    ENDIF(MYSQL_CONFIG)

ELSE(UNIX)

	set(MYSQL_ADD_INCLUDE_DIR "c:/msys/local/include" CACHE FILEPATH INTERNAL)
    set(MYSQL_ADD_LIBRARY_PATH "c:/msys/local/lib" CACHE FILEPATH INTERNAL)
ENDIF(UNIX)

if (NOT DEFINED MYSQL_FOUND)

	find_path(MYSQL_INCLUDE_DIR mysql.h
		/usr/local/include
		/usr/local/include/mysql 
		/usr/local/mysql/include
		/usr/include 
		${MYSQL_ADD_INCLUDE_DIR}
	)

	set(TMP_MYSQL_LIBRARIES "")
  	foreach(LIB ${MYSQL_ADD_LIBRARIES})
		find_library("MYSQL_LIBRARIES_${LIB}" NAMES ${LIB}
			PATHS
			${MYSQL_ADD_LIBRARY_PATH}
			/usr/lib/mysql
			/usr/local/lib
			/usr/local/lib/mysql
			/usr/local/mysql/lib
		)
		list(APPEND TMP_MYSQL_LIBRARIES "${MYSQL_LIBRARIES_${LIB}}")
  	endforeach(LIB ${MYSQL_ADD_LIBRARIES})
	set(MYSQL_LIBRARIES ${TMP_MYSQL_LIBRARIES} CACHE FILEPATH INTERNAL)

	if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
	   set(MYSQL_FOUND TRUE CACHE INTERNAL "MySQL found")
	   message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
	else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
	   set(MYSQL_FOUND FALSE CACHE INTERNAL "MySQL found")
	   message(STATUS "MySQL not found.")
	endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

	mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)

endif (NOT DEFINED MYSQL_FOUND)

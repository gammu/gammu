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


if (NOT DEFINED MYSQL_FOUND)

	find_path(MYSQL_INCLUDE_DIR mysql.h
	   /usr/include/mysql
	   /usr/local/include/mysql
	)

	find_library(MYSQL_LIBRARIES NAMES mysqlclient
	   PATHS
	   /usr/lib/mysql
	   /usr/local/lib/mysql
	)

	if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
	   set(MYSQL_FOUND TRUE CACHE INTERNAL "MySQL found")
	   message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
	else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
	   set(MYSQL_FOUND FALSE CACHE INTERNAL "MySQL found")
	   message(STATUS "MySQL not found.")
	endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

	mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)

endif (NOT DEFINED MYSQL_FOUND)

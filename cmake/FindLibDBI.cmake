# - Find libdbi library
# This module defines
#  LIBDBI_INCLUDE_DIR, where to find bluetooth.h
#  LIBDBI_LIBRARIES, the libraries needed to use libdbi.
#  LIBDBI_FOUND, If false, do not try to use libdbi.
#
# Copyright (c) 2009, Michal Cihar, <michal@cihar.com>
#
# vim: expandtab sw=4 ts=4 sts=4:

if (NOT LIBDBI_FOUND)
    find_path(LIBDBI_INCLUDE_DIR NAMES dbi/dbi.h
       PATHS
       /usr/include
       /usr/local/include
    )

    find_library(LIBDBI_LIBRARIES NAMES dbi
       PATHS
       /usr/lib
       /usr/local/lib
    )


    if(LIBDBI_INCLUDE_DIR AND LIBDBI_LIBRARIES)
        check_symbol_exists (DBI_INTEGER_SIZEMASK "${LIBDBI_INCLUDE_DIR}/dbi/dbi.h" HAVE_DBI_INTEGER_SIZEMASK)
        if (HAVE_DBI_INTEGER_SIZEMASK)
            message(STATUS "Found libdbi: ${LIBDBI_INCLUDE_DIR}, ${LIBDBI_LIBRARIES}")
            set(LIBDBI_FOUND TRUE CACHE INTERNAL "libdbi found")
        else (HAVE_DBI_INTEGER_SIZEMASK)
            message(STATUS "Found libdbi, but it is too old!")
            set(LIBDBI_FOUND FALSE CACHE INTERNAL "libdbi found")
            unset (LIBDBI_INCLUDE_DIR)
            unset (LIBDBI_LIBRARIES)
            unset (LIBDBI_INCLUDE_DIR CACHE)
            unset (LIBDBI_LIBRARIES CACHE)
        endif (HAVE_DBI_INTEGER_SIZEMASK)
    else(LIBDBI_INCLUDE_DIR AND LIBDBI_LIBRARIES)
        set(LIBDBI_FOUND FALSE CACHE INTERNAL "libdbi found")
        message(STATUS "libdbi not found.")
    endif(LIBDBI_INCLUDE_DIR AND LIBDBI_LIBRARIES)

    mark_as_advanced(LIBDBI_INCLUDE_DIR LIBDBI_LIBRARIES)
endif (NOT LIBDBI_FOUND)


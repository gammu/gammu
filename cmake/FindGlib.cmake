pkg_check_modules(GLIB glib-2.0)

# Find Glib even if pkg-config is not working (eg. cross compiling to Windows)
if (NOT GLIB_FOUND)
    find_library(GLIB_LIBRARIES NAMES glib-2.0)
    string (REGEX REPLACE "/[^/]*$" "" GLIB_LIBRARIES_DIR ${GLIB_LIBRARIES})

    find_path(GLIB_INCLUDE_DIR NAMES glib.h PATH_SUFFIXES glib-2.0)
    find_path(GLIB_CONFIG_INCLUDE_DIR NAMES glibconfig.h PATHS ${GLIB_LIBRARIES_DIR} PATH_SUFFIXES glib-2.0/include)

    if (GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARIES)
        set(GLIB_FOUND TRUE)
        set(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR} ${GLIB_CONFIG_INCLUDE_DIR})
    endif (GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARIES)
endif (NOT GLIB_FOUND)

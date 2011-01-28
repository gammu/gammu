pkg_check_modules(GOBJECT_PKG gobject-2.0)

if (GOBJECT_PKG_FOUND)
    find_path(GOBJECT_INCLUDE_DIR  NAMES glib-object.h PATH_SUFFIXES glib-2.0
       PATHS
       ${GOBJECT_PKG_INCLUDE_DIRS}
       /usr/include/glib-2.0
       /usr/include
       /usr/local/include
    )
    find_library(GOBJECT_LIBRARIES NAMES gobject-2.0
       PATHS
       ${GOBJECT_PKG_LIBRARY_DIRS}
       /usr/lib
       /usr/local/lib
    )
else (GOBJECT_PKG_FOUND)
    # Find Gobject even if pkg-config is not working (eg. cross compiling to Windows)
    find_path(GOBJECT_INCLUDE_DIRS NAMES glib-object.h PATH_SUFFIXES glib-2.0)

    find_library(GOBJECT_LIBRARIES NAMES gobject-2.0)
endif (GOBJECT_PKG_FOUND)

if(GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)
   set(GOBJECT_FOUND TRUE CACHE INTERNAL "gobject-2.0 found")
   message(STATUS "Found gobject-2.0: ${GOBJECT_INCLUDE_DIR}, ${GOBJECT_LIBRARIES}")
else(GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)
   set(GOBJECT_FOUND FALSE CACHE INTERNAL "gobject-2.0 found")
   message(STATUS "gobject-2.0 not found.")
endif(GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)

mark_as_advanced(GOBJECT_INCLUDE_DIR GOBJECT_LIBRARIES)

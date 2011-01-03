pkg_check_modules(GOBJECT gobject-2.0)

# Find Gobject even if pkg-config is not working (eg. cross compiling to Windows)
if (NOT GOBJECT_FOUND)
    find_path(GOBJECT_INCLUDE_DIRS NAMES glib-object.h PATH_SUFFIXES glib-2.0)

    find_library(GOBJECT_LIBRARIES NAMES gobject-2.0)
    if (GOBJECT_INCLUDE_DIRS AND GOBJECT_LIBRARIES)
        set(GOBJECT_FOUND TRUE)
    endif (GOBJECT_INCLUDE_DIRS AND GOBJECT_LIBRARIES)
endif (NOT GOBJECT_FOUND)


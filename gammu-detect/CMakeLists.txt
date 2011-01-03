project (Gammu-Detect C)

pkg_check_modules(GUDEV gudev-1.0)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(GOBJECT gobject-2.0)

# Find Glib/Gobject even if pkg-config is not working (eg. cross compiling to Windows)
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

if (NOT GOBJECT_FOUND)
    find_path(GOBJECT_INCLUDE_DIRS NAMES glib-object.h PATH_SUFFIXES glib-2.0)

    find_library(GOBJECT_LIBRARIES NAMES gobject-2.0)
    if (GOBJECT_INCLUDE_DIRS AND GOBJECT_LIBRARIES)
        set(GOBJECT_FOUND TRUE)
    endif (GOBJECT_INCLUDE_DIRS AND GOBJECT_LIBRARIES)
endif (NOT GOBJECT_FOUND)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if (GLIB_FOUND AND GOBJECT_FOUND)
    set(DETECT_SRC "main.c")

    if (GUDEV_FOUND)
        list(APPEND DETECT_SRC "udev.c")
    endif (GUDEV_FOUND)
    if (BLUEZ_FOUND)
        list(APPEND DETECT_SRC "bluez.c")
    endif (BLUEZ_FOUND)
    if (WIN32)
        list(APPEND DETECT_SRC "win32-serial.c")
    endif (WIN32)

    add_executable(gammu-detect ${DETECT_SRC})
    target_link_libraries (gammu-detect ${GLIB_LIBRARIES} ${GOBJECT_LIBRARIES} libGammu)
    include_directories(${GLIB_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS})
    if (GUDEV_FOUND)
        target_link_libraries (gammu-detect ${GUDEV_LIBRARIES})
        include_directories(${GUDEV_INCLUDE_DIRS})
    endif (GUDEV_FOUND)
    if (BLUEZ_FOUND)
        target_link_libraries (gammu-detect ${BLUEZ_LIBRARIES})
        include_directories (${BLUEZ_INCLUDE_DIR})
    endif (BLUEZ_FOUND)
    if (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
        target_link_libraries (gammu-detect ${LIBINTL_LIBRARIES})
        include_directories (${LIBINTL_INCLUDE_DIR})
    endif (LIBINTL_LIB_FOUND AND LIBINTL_LIBRARIES)
    if (WIN32)
        target_link_libraries (gammu-detect string)
    endif (WIN32)

    include_directories (
        "${Gammu_BINARY_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}"
        )
endif (GLIB_FOUND AND GOBJECT_FOUND)


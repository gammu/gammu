project (Gammu-Detect C)

pkg_check_modules(GUDEV gudev-1.0)
pkg_check_modules(GLIB glib-2.0)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if (GLIB_FOUND)
    set(DETECT_SRC "main.c")

    if (GUDEV_FOUND)
        list(APPEND DETECT_SRC "udev.c")
    endif (GUDEV_FOUND)
    if (BLUEZ_FOUND)
        list(APPEND DETECT_SRC "bluez.c")
    endif (BLUEZ_FOUND)

    add_executable(gammu-detect ${DETECT_SRC})
    target_link_libraries (gammu-detect ${GLIB_LIBRARIES} libGammu)
    include_directories(${GLIB_INCLUDE_DIRS})
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

    include_directories (
        "${Gammu_BINARY_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}"
        )
endif (GLIB_FOUND)


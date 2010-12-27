project (Gammu-Detect C)

pkg_check_modules(GUDEV gudev-1.0)
pkg_check_modules(GLIB glib-2.0)

if (GLIB_FOUND AND GUDEV_FOUND)
    add_executable(gammu-detect "main.c")
    target_link_libraries (gammu-detect ${GLIB_LIBRARIES} ${GUDEV_LIBRARIES})
    include_directories(${GLIB_INCLUDE_DIRS} ${GUDEV_INCLUDE_DIRS})
    include_directories (
        "${Gammu_BINARY_DIR}/include"
        )
endif (GLIB_FOUND AND GUDEV_FOUND)


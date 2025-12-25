project (Gammu-Detect C)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if (Glib_FOUND AND GObject_FOUND)
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
    add_coverage (gammu-detect)
    target_link_libraries (gammu-detect ${Glib_LIBRARIES} ${GObject_LIBRARIES} libGammu)
    include_directories(${Glib_INCLUDE_DIRS} ${GObject_INCLUDE_DIRS})
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
        "${CMAKE_CURRENT_BINARY_DIR}/../include"
        "${CMAKE_CURRENT_BINARY_DIR}"
        )
    install (TARGETS gammu-detect
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            COMPONENT gammu
            )

    add_test(gammu-detect-help "${CMAKE_CURRENT_BINARY_DIR}/gammu-detect${CMAKE_EXECUTABLE_SUFFIX}" --help)
    add_test(gammu-detect-version "${CMAKE_CURRENT_BINARY_DIR}/gammu-detect${CMAKE_EXECUTABLE_SUFFIX}" --version)
    add_test(gammu-detect-run "${CMAKE_CURRENT_BINARY_DIR}/gammu-detect${CMAKE_EXECUTABLE_SUFFIX}")

endif (Glib_FOUND AND GObject_FOUND)

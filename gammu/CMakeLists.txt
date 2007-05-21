# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (gammu C)

set (BINARY_SRC
    depend/nokia/dct3.c
    depend/nokia/dct3trac/wmx.c
    depend/nokia/dct3trac/wmx-gsm.c
    depend/nokia/dct3trac/wmx-list.c
    depend/nokia/dct3trac/wmx-sim.c
    depend/nokia/dct3trac/wmx-util.c
    depend/nokia/dct4.c
    depend/siemens/dsiemens.c
    depend/siemens/chiffre.c
    smsd/smsdcore.c
    smsd/s_files.c
    smsd/s_mysql.c
    smsd/s_pgsql.c
    sniff.c
    gammu.c)

# Gammu program
add_executable (gammu ${BINARY_SRC})
if (CROSS_MINGW)
    set_target_properties (gammu PROPERTIES PREFIX "" SUFFIX ".exe")
endif (CROSS_MINGW)

target_link_libraries (gammu Gammu)
target_link_libraries (gammu ${CMAKE_THREAD_LIBS_INIT})

include_directories (
    "${Gammu_BINARY_DIR}/include"
	"${Gammu_SOURCE_DIR}/include"
    )
# Tune options for found libraries
if (MYSQL_FOUND)
    target_link_libraries (gammu ${MYSQL_LIBRARIES})
    include_directories (${MYSQL_INCLUDE_DIR})
    set (HAVE_MYSQL_MYSQL_H TRUE)
endif (MYSQL_FOUND)

if (POSTGRES_FOUND)
    target_link_libraries (gammu ${POSTGRES_LIBRARY})
    include_directories (${POSTGRES_INCLUDE_DIR})

    set (HAVE_POSTGRESQL_LIBPQ_FE_H TRUE)
endif (POSTGRES_FOUND)

if (GETTEXTLIBS_FOUND)
    target_link_libraries (gammu ${GETTEXT_LIBRARY})
    include_directories (${GETTEXT_INCLUDE_DIR})
endif (GETTEXTLIBS_FOUND)

install (TARGETS gammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT "runtime"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT "runtime"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT "development"
        )

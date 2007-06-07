# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (libGammu C)

set (LIBRARY_SRC
    gsmcomon.c
    gsmphones.c
    gsmstate.c
    api.c
    misc/misc.c
    misc/cfg.c
    misc/coding/coding.c
    misc/coding/md5.c
    service/sms/gsmsms.c
    service/sms/gsmems.c
    service/sms/gsmmulti.c
    service/gsmcal.c
    service/gsmdata.c
    service/gsmpbk.c
    service/gsmring.c
    service/gsmlogo.c
    service/gsmmisc.c
    service/gsmnet.c
    service/backup/gsmback.c
    service/backup/backldif.c
    service/backup/backlmb.c
    service/backup/backtext.c
    service/backup/backvcs.c
    service/backup/backvcf.c
    service/backup/backics.c
    device/bluetoth/affix.c
    device/bluetoth/bluez.c
    device/bluetoth/blue_w32.c
    device/bluetoth/bluetoth.c
    device/serial/ser_unx.c
    device/serial/ser_djg.c
    device/serial/ser_w32.c
    device/irda/irda.c
    device/devfunc.c
    protocol/at/at.c
    protocol/alcatel/alcabus.c
    protocol/nokia/mbus2.c
    protocol/nokia/fbus2.c
    protocol/nokia/phonet.c
    protocol/obex/obex.c
    protocol/symbian/gnapbus.c
    phone/pfunc.c
    phone/at/atgen.c
    phone/at/siemens.c
    phone/at/samsung.c
    phone/alcatel/alcatel.c
    phone/sonyeric/sonyeric.c
    phone/nokia/dct3/n0650.c
    phone/nokia/dct3/n6110.c
    phone/nokia/dct3/n7110.c
    phone/nokia/dct3/n9210.c
    phone/nokia/dct3/dct3func.c
    phone/nokia/dct4s40/n3320.c
    phone/nokia/dct4s40/dct4func.c
    phone/nokia/dct4s40/6510/n6510.c
    phone/nokia/dct4s40/6510/6510cal.c
    phone/nokia/dct4s40/6510/6510file.c
    phone/nokia/wd2/n3650.c
    phone/nokia/nauto.c
    phone/nokia/nfunc.c
    phone/nokia/nfuncold.c
    phone/obex/obexgen.c
    phone/symbian/gnapgen.c)

# Library
add_library (Gammu ${LIBRARY_SRC})
if (CROSS_MINGW)
    if (ENABLE_SHARED)
        set_target_properties (Gammu PROPERTIES 
            PREFIX "" 
            SUFFIX ".dll"
            IMPORT_PREFIX ""
            IMPORT_SUFFIX ".lib"
            )
    else (ENABLE_SHARED)
        set_target_properties (Gammu PROPERTIES PREFIX "" SUFFIX ".lib")
    endif (ENABLE_SHARED)
else (CROSS_MINGW)
    # -fPIC on Linux
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        set_target_properties (Gammu PROPERTIES COMPILE_FLAGS "-fPIC")
    endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

endif (CROSS_MINGW)

set_target_properties (Gammu PROPERTIES VERSION 2.${VERSION} SOVERSION 2)

set (GAMMU_LIBS "")

if (BLUEZ_FOUND)
    target_link_libraries (Gammu ${BLUEZ_LIBRARIES})
    include_directories (${BLUEZ_INCLUDE_DIR})
    if (NOT "${BLUEZ_LIBRARIES}" STREQUAL "") 
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${BLUEZ_LIBRARIES}")
    endif (NOT "${BLUEZ_LIBRARIES}" STREQUAL "") 
endif (BLUEZ_FOUND)

if (NOT HAVE_SIN)
    target_link_libraries (Gammu m)
endif (NOT HAVE_SIN)

# Include search path
include_directories (
    "${Gammu_BINARY_DIR}/include"
	"${Gammu_SOURCE_DIR}/common"
    )

# Add libraries needed for windows build
if (WIN32)
    # Magic needed for SHGetFolderPathA
    target_link_libraries (Gammu shfolder shell32)
    # Win sock
    target_link_libraries (Gammu ws2_32)
    # These libraries are needed for serial driver
    target_link_libraries (Gammu setupapi advapi32)
endif (WIN32)

if (GETTEXTLIBS_FOUND)
    target_link_libraries (Gammu ${GETTEXT_LIBRARY})
    include_directories (${GETTEXT_INCLUDE_DIR})
    if (NOT "${GETTEXT_LIBRARIES}" STREQUAL "") 
        set (GAMMU_LIBS "${GAMMU_LIBS} -l${GETTEXT_LIBRARIES}")
    endif (NOT "${GETTEXT_LIBRARIES}" STREQUAL "") 
endif (GETTEXTLIBS_FOUND)

# Define required libraries for gammu library if it is static
if (ENABLE_SHARED)
    set (GAMMU_PRIVATE_LIBS "" CACHE INTERNAL "Private libraries for gammu library")
else (ENABLE_SHARED)
    set (GAMMU_PRIVATE_LIBS "${GAMMU_LIBS}" CACHE INTERNAL "Private libraries for gammu library")
endif (ENABLE_SHARED)

# Install instructions
install (TARGETS Gammu
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT "runtime"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT "runtime"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT "development"
        )

# cmake <http://www.cmake.org> build file for Gammu contrib files
# Copyright (c) 2007 - 2018 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-contrib C)

include("GNUInstallDirs")

option(INSTALL_GNAPPLET "Install Gnapplet binaries" ON)
option(INSTALL_S60 "Install Series 60 applet binaries" ON)
option(INSTALL_MEDIA "Install sample media files" ON)
option(INSTALL_PHP_EXAMPLES "Install PHP example scripts" ON)

if (BASH_BIN)
    option(INSTALL_BASH_COMPLETION "Install bash completion script for Gammu" ON)
else (BASH_BIN)
    option(INSTALL_BASH_COMPLETION "Install bash completion script for Gammu" OFF)
endif (BASH_BIN)

option(INSTALL_LSB_INIT "Install LSB compatible init script for Gammu" OFF)
option(INSTALL_UDEV_RULES "Install udev rules for Gammu" OFF)

set (GNAPPLET_FILES
    symbian/gnapplet.ini
    symbian/gnapplet.sis
    )

set (S60_FILES
    s60/gammu-s60-remote.sis
    )

file(GLOB MEDIA_FILES "media/*.txt" "media/*.nlm")

set (PHP_EXAMPLES
    php/class_gammu/class.gammu.php
    php/class_gammu/class.sms.gammu.php
    php/class_gammu/README
    php/smsd-mysql-admin/admin.php
    php/smsd-mysql-linked/linked.php
    php/smsd-mysql-linked/linked.sql
    php/smsd-mysql-intergammu/config.php
    php/smsd-mysql-intergammu/funcoes/func.gammu.php
    php/smsd-mysql-intergammu/funcoes/func.sql.php
    php/smsd-mysql-intergammu/index.php
    php/smsd-mysql-intergammu/intergammu.txt
    php/smsd-mysql-intergammu/proc/admin.php
    php/smsd-mysql-intergammu/proclast.sql
    php/smsd-mysql-list/sms.php
    )

if (INSTALL_GNAPPLET)
    install (
        FILES ${GNAPPLET_FILES}
        DESTINATION "${INSTALL_DATA_DIR}"
        COMPONENT "symbian"
        )
endif (INSTALL_GNAPPLET)

if (INSTALL_S60)
    install (
        FILES ${S60_FILES}
        DESTINATION "${INSTALL_DATA_DIR}"
        COMPONENT "symbian"
        )
endif (INSTALL_S60)

if (INSTALL_MEDIA)
    install (
        FILES ${MEDIA_FILES}
        DESTINATION "${INSTALL_DOC_DIR}/examples/media"
        COMPONENT "media"
        )
endif (INSTALL_MEDIA)


if (INSTALL_PHP_EXAMPLES)
    foreach (EXAMPLE ${PHP_EXAMPLES})
        get_filename_component(DIRNAME ${EXAMPLE} PATH)
        install (
            FILES ${EXAMPLE}
            DESTINATION "${INSTALL_DOC_DIR}/examples/${DIRNAME}"
            COMPONENT "examples"
            )
    endforeach (EXAMPLE)
endif (INSTALL_PHP_EXAMPLES)

if (INSTALL_BASH_COMPLETION)
    macro_optional_find_package (BashCompletion)
    if (NOT BASH_COMPLETION_FOUND)
      set (BASH_COMPLETION_COMPLETIONSDIR "/etc/bash_completion.d" CACHE PATH "Location of bash_completion.d")
    endif (NOT BASH_COMPLETION_FOUND)
    install (
        FILES bash-completion/gammu
        DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR}
        COMPONENT "bash"
        )
endif (INSTALL_BASH_COMPLETION)

if (WITH_SYSTEMD)
    configure_file( init/gammu-smsd.service init/gammu-smsd.service )
    install (
        FILES ${CMAKE_CURRENT_BINARY_DIR}/init/gammu-smsd.service
        DESTINATION "${SYSTEMD_SERVICES_INSTALL_DIR}"
        COMPONENT "systemd"
    )
endif (WITH_SYSTEMD)

if (INSTALL_LSB_INIT)
    configure_file( init/gammu-smsd.lsb init/gammu-smsd.lsb )
    install (
        FILES ${CMAKE_CURRENT_BINARY_DIR}/init/gammu-smsd.lsb
        DESTINATION "/etc/init.d"
        RENAME "gammu-smsd"
        COMPONENT "initscript"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
else (INSTALL_LSB_INIT)
    message (STATUS "Init script not being installed, install it by -DINSTALL_LSB_INIT=ON")
endif (INSTALL_LSB_INIT)

if (INSTALL_UDEV_RULES)
    install (
        FILES udev/69-gammu-acl.rules
        DESTINATION "/etc/udev/rules.d"
        COMPONENT "udev"
        )
else (INSTALL_UDEV_RULES)
    message (STATUS "Udev rules not being installed, install it by -DINSTALL_UDEV_RULES=ON")
endif (INSTALL_UDEV_RULES)

# Build some examples
# We do not install them intentionally, just check whether they still compile
add_subdirectory(smscgi)
add_subdirectory(convert)
add_subdirectory(sqlreply)
add_subdirectory(coveragehelper)

if (DEBUG OR "$ENV{TRAVIS}" STREQUAL "true")
    add_subdirectory(cpp-sample)
endif (DEBUG OR "$ENV{TRAVIS}" STREQUAL "true")

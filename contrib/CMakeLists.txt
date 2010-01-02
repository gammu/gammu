# cmake <http://www.cmake.org> build file for Gammu contrib files
# Copyright (c) 2007 - 2010 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-contrib C)

option(INSTALL_GNAPPLET "Install Gnapplet binaries" ON)
option(INSTALL_MEDIA "Install sample media files" ON)
option(INSTALL_PHP_EXAMPLES "Install PHP example scripts" ON)

if (BASH_BIN)
    option(INSTALL_BASH_COMPLETION "Install bash completion script for Gammu" ON)
else (BASH_BIN)
    option(INSTALL_BASH_COMPLETION "Install bash completion script for Gammu" OFF)
endif (BASH_BIN)

option(INSTALL_LSB_INIT "Install LSB compatible init script for Gammu" OFF)

set (SYMBIAN_FILES
    symbian/readme.txt
    symbian/gnapplet.ini
    symbian/gnapplet.sis
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
        FILES ${SYMBIAN_FILES}
        DESTINATION "${INSTALL_DOC_DIR}/symbian"
        COMPONENT "gnapplet"
        )
endif (INSTALL_GNAPPLET)

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
    install (
        FILES bash-completion/gammu
        DESTINATION "/etc/bash_completion.d"
        COMPONENT "bash"
        )
endif (INSTALL_BASH_COMPLETION)

if (INSTALL_LSB_INIT)
    install (
        FILES init/gammu-smsd.lsb
        DESTINATION "/etc/init.d"
        RENAME "gammu-smsd"
        COMPONENT "initscript"
        )
else (INSTALL_LSB_INIT)
    message (STATUS "Init script not being installed, install it by -DINSTALL_LSB_INIT=ON")
endif (INSTALL_LSB_INIT)

# Build some examples
# We do not install them intentionally, just check whether they still compile
add_subdirectory(smscgi)

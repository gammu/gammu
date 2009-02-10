# cmake <http://www.cmake.org> build file for Gammu utils
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-utils C)

set (UTILS
    gammu-config
    jadmaker
    )

install (
    PROGRAMS ${UTILS}
    DESTINATION "${INSTALL_BIN_DIR}"
    COMPONENT "utils"
    )


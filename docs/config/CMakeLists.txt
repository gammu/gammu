# cmake <http://www.cmake.org> build file for Gammu docs
# Copyright (c) 2007 - 2010 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-doc-config C)

install (
    FILES gammurc smsdrc
    DESTINATION "${INSTALL_DOC_DIR}/examples/config"
    COMPONENT "examples"
    )

# cmake <http://www.cmake.org> build file for Gammu docs
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-doc-sql C)

file(GLOB SQL_FILES "*.sql")

install (
    FILES ${SQL_FILES}
    DESTINATION "${INSTALL_DOC_DIR}/examples/sql"
    COMPONENT "examples"
    )


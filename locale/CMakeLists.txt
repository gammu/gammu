# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project(Gammu-Locales C)

# We need gettext
macro_optional_find_package(Gettext)


if(GETTEXT_FOUND AND GETTEXTLIBS_FOUND)
    # List of locales
    file(GLOB LOCALEFILES "*/gammu.po")
    file(GLOB LIBLOCALEFILES "*/libgammu.po")
    # Generate mo files
    GETTEXT_CREATE_TRANSLATIONS("gammu.pot" ALL ${LOCALEFILES})
    GETTEXT_CREATE_TRANSLATIONS("libgammu.pot" ALL ${LIBLOCALEFILES})
endif(GETTEXT_FOUND AND GETTEXTLIBS_FOUND)

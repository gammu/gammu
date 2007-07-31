# cmake <http://www.cmake.org> build file for Gammu
# Copyright (c) 2007 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project(Gammu-Locales C)

# We need gettext
macro_optional_find_package(Gettext)

# List of locales
set(LOCALES 
    cs 
    de 
    es 
    it 
    pl 
    ru 
    )

# Locales files
set(LOCALEFILES)
foreach(LOCALE ${LOCALES})
    list(APPEND LOCALEFILES "${LOCALE}/gammu.po")
endforeach(LOCALE ${LOCALES})

# Generate mo files
if(GETTEXT_FOUND)
    GETTEXT_CREATE_TRANSLATIONS("gammu.pot" ALL ${LOCALEFILES})
endif(GETTEXT_FOUND)

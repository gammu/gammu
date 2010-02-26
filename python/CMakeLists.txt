# cmake <http://www.cmake.org> build file for python-gammu
# Copyright (c) 2007 - 2010 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (python-gammu C)

add_subdirectory(gammu)

# Testing
include(CMakeTests.txt)

# Documentation
find_program(EPYDOC_EXECUTABLE NAMES epydoc
    DOC "Epydoc tool for generating Python API documentation <http://epydoc.sourceforge.net/>")
if (EPYDOC_EXECUTABLE)
    add_custom_target (pydoc ${PYTHON_EXECUTABLE} ${EPYDOC_EXECUTABLE} --verbose --name python-gammu --url http://wammu.eu/python-gammu/ gammu
        DEPENDS ${PYFILES} ${PYTHON_GAMMU_SRC}
        COMMENT "Generating Python documentation")
endif (EPYDOC_EXECUTABLE)



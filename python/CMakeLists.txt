# cmake <http://www.cmake.org> build file for python-gammu
# Copyright (c) 2007-2009 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (python-gammu C)

add_subdirectory(gammu)

# Testing
include(CMakeTests.txt)

# Documentation
find_program(EPYDOC_EXECUTABLE NAMES epydoc
    DOC "Epydoc tool for generating Python API documentation <http://epydoc.sourceforge.net/>")
if (EPYDOC_EXECUTABLE)
    add_custom_target (pydoc ${PYTHON_EXECUTABLE} ${EPYDOC_EXECUTABLE} --verbose --name python-gammu --url http://cihar.com/gammu/python/ gammu
        DEPENDS ${PYFILES} ${PYTHON_GAMMU_SRC}
        COMMENT "Generating Python documentation")
endif (EPYDOC_EXECUTABLE)


find_program(SPHINX_BUILD_EXECUTABLE NAMES sphinx-build
    DOC "Sphinx tool for generating Python-gammu documentation <http://sphinx.pocoo.org/>")
if (SPHINX_BUILD_EXECUTABLE)
    set (SPHINX_PAPER_SIZE "a4" CACHE STRING "Paper size for Sphinx LaTeX output")
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/doc/conf.py" "${CMAKE_CURRENT_BINARY_DIR}/conf.py" @ONLY)
    macro(sphinxdoc _format)
        add_custom_target (pydoc-${_format} ${PYTHON_EXECUTABLE} ${SPHINX_BUILD_EXECUTABLE}
                -c ${CMAKE_CURRENT_BINARY_DIR}/
                -b ${_format}
                -D latex_paper_size=${SPHINX_PAPER_SIZE}
                ${CMAKE_CURRENT_SOURCE_DIR}/doc
                ${CMAKE_CURRENT_BINARY_DIR}/doc-${_format}
            DEPENDS ${PYFILES} ${PYTHON_GAMMU_SRC}
            COMMENT "Generating Sphinx documentation (${_format})")
    endmacro(sphinxdoc _format)
    sphinxdoc(html)
    sphinxdoc(htmlhelp)
    sphinxdoc(latex)
endif (SPHINX_BUILD_EXECUTABLE)

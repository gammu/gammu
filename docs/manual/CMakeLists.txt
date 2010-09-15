# cmake <http://www.cmake.org> build file for Gammu docs
# Copyright (c) 2007 - 2010 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-doc-devel C)

file(GLOB_RECURSE DOCSRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.rst")

set (DEVELDOCS
    ${DOCSRC}
    )

foreach (DOC ${DEVELDOCS})
    get_filename_component(DIRNAME ${DOC} PATH)
    install (
        FILES ${DOC}
        DESTINATION "${INSTALL_DOC_DIR}/manual/${DIRNAME}"
        COMPONENT "manual"
        )
endforeach (DOC)

find_program(SPHINX_BUILD_EXECUTABLE NAMES sphinx-build
    DOC "Sphinx tool for generating Gammu documentation <http://sphinx.pocoo.org/>")

if (SPHINX_BUILD_EXECUTABLE)
    set (SPHINX_PAPER_SIZE "a4" CACHE STRING "Paper size for Sphinx LaTeX output")
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/conf.py" "${CMAKE_CURRENT_BINARY_DIR}/conf.py" @ONLY)
    macro(sphinxdoc _format)
        add_custom_target (manual-${_format} ${PYTHON_EXECUTABLE} ${SPHINX_BUILD_EXECUTABLE}
                -c ${CMAKE_CURRENT_BINARY_DIR}/
                -n
                -b ${_format}
                -D latex_paper_size=${SPHINX_PAPER_SIZE}
                ${CMAKE_CURRENT_SOURCE_DIR}/
                ${CMAKE_CURRENT_BINARY_DIR}/${_format}
            DEPENDS ${PYFILES} ${PYTHON_GAMMU_SRC}
            COMMENT "Generating Sphinx documentation (${_format})")
    endmacro(sphinxdoc _format)
    sphinxdoc(html)
    sphinxdoc(htmlhelp)
    sphinxdoc(latex)
    sphinxdoc(man)
endif (SPHINX_BUILD_EXECUTABLE)

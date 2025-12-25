# cmake <http://www.cmake.org> build file for Gammu docs
# Copyright (c) 2007 - 2018 Michal Cihar
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
    set (SPHINX_PAPER_SIZE "a4" CACHE STRING "Paper size for Sphinx LaTeX/PDF/PS output")
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/conf.py" "${CMAKE_CURRENT_BINARY_DIR}/conf.py" @ONLY)
    macro(sphinxdoc _format)
        add_custom_target (manual-${_format} ${SPHINX_BUILD_EXECUTABLE}
                -c ${CMAKE_CURRENT_BINARY_DIR}/
                -n
                -b ${_format}
                -D latex_paper_size=${SPHINX_PAPER_SIZE}
                ${CMAKE_CURRENT_SOURCE_DIR}/
                ${CMAKE_CURRENT_BINARY_DIR}/${_format}
            COMMENT "Generating Sphinx documentation (${_format})")
        add_dependencies(manual-${_format} apidoc)
    endmacro(sphinxdoc _format)
    sphinxdoc(html)
    sphinxdoc(htmlhelp)
    sphinxdoc(latex)
    sphinxdoc(man)
    add_custom_target(manual-pdf
        yes '' | make all-pdf
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/latex)
    add_custom_target(manual-ps
        yes '' | make all-ps
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/latex)
    add_dependencies(manual-pdf manual-latex)
    add_dependencies(manual-ps manual-latex)
endif (SPHINX_BUILD_EXECUTABLE)

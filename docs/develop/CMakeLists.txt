# cmake <http://www.cmake.org> build file for Gammu docs
# Copyright (c) 2007 - 2010 Michal Cihar
# vim: expandtab sw=4 ts=4 sts=4:

project (Gammu-doc-devel C)

set (DEVELDOCS
    Gammu.htm
    PORTING
    develop.txt
    gammu_hints.txt
    protocol/TDMA_5120.txt
    protocol/carkit.txt
    protocol/n6110.txt
    protocol/n6510.txt
    protocol/n7110.txt
    protocol/nokia.txt
    protocol/readme
    sms/charset.txt
    sms/convert.txt
    sms/readme
    sounds/readme
    sounds/ring2.txt
    examples/Makefile
    examples/phone-info.c
    examples/sms-send.c
    )

foreach (DOC ${DEVELDOCS})
    get_filename_component(DIRNAME ${DOC} PATH)
    install (
        FILES ${DOC}
        DESTINATION "${INSTALL_DOC_DIR}/devel/${DIRNAME}"
        COMPONENT "develdocs"
        )
endforeach (DOC)

find_program(SPHINX_BUILD_EXECUTABLE NAMES sphinx-build
    DOC "Sphinx tool for generating Gammu developer documentation <http://sphinx.pocoo.org/>")

if (SPHINX_BUILD_EXECUTABLE)
    set (SPHINX_PAPER_SIZE "a4" CACHE STRING "Paper size for Sphinx LaTeX output")
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/source/conf.py" "${CMAKE_CURRENT_BINARY_DIR}/conf.py" @ONLY)
    macro(sphinxdoc _format)
        add_custom_target (devdoc-${_format} ${PYTHON_EXECUTABLE} ${SPHINX_BUILD_EXECUTABLE}
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

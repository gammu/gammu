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

project (Gammu-doc-examples C)

set (EXAMPLES
    Makefile
    phone-info.c
    sms-send.c
    smsd.c
    sms-read.c
    long-sms.c
    custom-config.c
    )

foreach (DOC ${EXAMPLES})
    get_filename_component(DIRNAME ${DOC} PATH)
    install (
        FILES ${DOC}
        DESTINATION "${INSTALL_DOC_DIR}/examples/${DIRNAME}"
        COMPONENT "manual"
        )
endforeach (DOC)

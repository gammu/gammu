project (Gammu-user-docs C)

# [a-z] is needed to skip CMakeLists.txt
file(GLOB TEXTDOCS "[a-z]*.txt" "*.html")

install (
    FILES ${TEXTDOCS}
    DESTINATION "${INSTALL_DOC_DIR}"
    COMPONENT "docs"
    )

file(GLOB MAN1_PAGES "*.1")

install (
    FILES ${MAN1_PAGES}
    DESTINATION "${INSTALL_MAN_DIR}/man1"
    COMPONENT "manpages"
    )

file(GLOB MAN5_PAGES "*.5")

install (
    FILES ${MAN5_PAGES}
    DESTINATION "${INSTALL_MAN_DIR}/man5"
    COMPONENT "manpages"
    )

file(GLOB MAN5_PAGES "*.7")

install (
    FILES ${MAN5_PAGES}
    DESTINATION "${INSTALL_MAN_DIR}/man7"
    COMPONENT "manpages"
    )

# This should match list in admin/po4a.config
foreach(LANG cs de es id it pl ru)

    file(GLOB MAN1_PAGES "${LANG}/*.1")

    install (
        FILES ${MAN1_PAGES}
        DESTINATION "${INSTALL_MAN_DIR}/${LANG}/man1"
        COMPONENT "manpages"
        )

    file(GLOB MAN5_PAGES "${LANG}/*.5")

    install (
        FILES ${MAN5_PAGES}
        DESTINATION "${INSTALL_MAN_DIR}/${LANG}/man5"
        COMPONENT "manpages"
        )

    file(GLOB MAN5_PAGES "${LANG}/*.7")

    install (
        FILES ${MAN5_PAGES}
        DESTINATION "${INSTALL_MAN_DIR}/${LANG}/man7"
        COMPONENT "manpages"
        )
endforeach(LANG)

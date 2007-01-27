
MACRO (MACRO_GAMMU_OPTION _name _description _default)
    OPTION(WITH_${_name} "Whether to enable ${_description}" ${_default})
    if (WITH_${_name})
        message("${_description} enabled")
        set(GSM_ENABLE_${_name} TRUE)
    endif (WITH_${_name})
ENDMACRO (MACRO_GAMMU_OPTION)


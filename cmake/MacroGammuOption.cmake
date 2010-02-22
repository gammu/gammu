
set (ON ON)

MACRO (MACRO_GAMMU_OPTION _name _description _default _cond1 _cond2)
    if (${_cond1} AND ${_cond2})
        OPTION(WITH_${_name} "Whether to enable ${_description}" ${_default})
        if (WITH_${_name})
            message (STATUS "${_description} enabled")
            set(GSM_ENABLE_${_name} TRUE CACHE INTERNAL "${_name}")
        else (WITH_${_name})
            message (STATUS "${_description} disabled")
            set(GSM_ENABLE_${_name} FALSE CACHE INTERNAL "${_name}")
        endif (WITH_${_name})
    else (${_cond1} AND ${_cond2})
        set(GSM_ENABLE_${_name} FALSE CACHE INTERNAL "${_name}")
        set(WITH_${_name} FALSE CACHE INTERNAL "Whether to enable ${_description}" FORCE)
    endif (${_cond1} AND ${_cond2})
ENDMACRO (MACRO_GAMMU_OPTION)


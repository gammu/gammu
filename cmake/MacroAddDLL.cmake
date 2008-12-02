# Adds flag to compiler if it supports it

macro (ADD_DLL _name _dir)
    if (WIN32)
        string (TOUPPER "${_name}" FLAGNAME)
        string (REPLACE " " "_" FLAGNAME "${FLAGNAME}")
        string (REPLACE "-" "_" FLAGNAME "${FLAGNAME}")
        string (REPLACE "=" "_" FLAGNAME "${FLAGNAME}")
        string (REPLACE "," "_" FLAGNAME "${FLAGNAME}")
        set (FLAGNAME "DLL_${FLAGNAME}")

        string(REGEX REPLACE "/[^/]*$" "" TMP_LIB_DIR "${_dir}")
        find_file(${FLAGNAME} "${_name}" "${TMP_LIB_DIR}/../bin/" "${TMP_LIB_DIR}") # DOC "Path to ${_name} library")
        list(APPEND WIN32_INSTALL_DLL "${${FLAGNAME}}")
    endif (WIN32)
endmacro (ADD_DLL _name _dir)

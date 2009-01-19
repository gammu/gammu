# Various compiler flags, which are used for Gammu, but can not be 
# used for whole tree (eg. Python module)

MACRO_TUNE_COMPILER("-Wwrite-strings")
MACRO_TUNE_COMPILER("-Wredundant-decls")
if (DEBUG)
    MACRO_TUNE_COMPILER("-fstrict-aliasing")
endif (DEBUG)


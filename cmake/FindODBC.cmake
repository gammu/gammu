# Find ODBC (or UnixODBC)

find_path(ODBC_INCLUDE_DIR NAMES sql.h
          DOC "The ODBC include directory"
)

find_library(ODBC_LIBRARY NAMES odbc odbc32
          DOC "The ODBC library"
)

# handle the QUIETLY and REQUIRED arguments and set ODBC_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ODBC DEFAULT_MSG ODBC_LIBRARY ODBC_INCLUDE_DIR)

if(ODBC_FOUND)
  set( ODBC_LIBRARIES ${ODBC_LIBRARY} )
  set( ODBC_INCLUDE_DIRS ${ODBC_INCLUDE_DIR} )
endif(ODBC_FOUND)

mark_as_advanced(ODBC_INCLUDE_DIR ODBC_LIBRARY)

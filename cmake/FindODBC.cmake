# Find ODBC (or UnixODBC)

find_path(ODBC_INCLUDE_DIR NAMES sql.h
	HINTS
	/usr/include
	/usr/include/odbc
	/usr/local/include
	/usr/local/include/odbc
	/usr/local/odbc/include
	"C:/Program Files/ODBC/include"
    "C:/Program Files/Microsoft SDKs/Windows/*/Include"
    "$ENV{ProgramFiles}/Microsoft SDKs/Windows/*/Include"
	"C:/ODBC/include"
    DOC "The ODBC include directory"
)

find_library(ODBC_LIBRARY NAMES iodbc odbc odbc32
    HINTS
    /usr/lib
    /usr/lib/odbc
    /usr/local/lib
    /usr/local/lib/odbc
    /usr/local/odbc/lib
    "C:/Program Files/ODBC/lib"
    "C:/ODBC/lib/debug"
    "$ENV{ProgramFiles}/Microsoft SDKs/Windows/*/Lib/${CMAKE_VS_PLATFORM_NAME}"
    "C:/Program Files/Microsoft SDKs/Windows/*/Lib/${CMAKE_VS_PLATFORM_NAME}"
    "$ENV{ProgramFiles}/Microsoft SDKs/Windows/*/Lib"
    "C:/Program Files/Microsoft SDKs/Windows/*/Lib"
    DOC "The ODBC library"
)

# handle the QUIETLY and REQUIRED arguments and set ODBC_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ODBC DEFAULT_MSG ODBC_LIBRARY ODBC_INCLUDE_DIR)

if(ODBC_FOUND)
  include(MacroCheckLibraryWorks)
  CHECK_LIBRARY_WORKS("sql.h" "SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, 0);" "${ODBC_INCLUDE_DIR}" "${ODBC_LIBRARY}" "ODBC_WORKS")
  if (NOT ${ODBC_WORKS})
      set(ODBC_FOUND FALSE)
  endif()

  set( ODBC_LIBRARIES ${ODBC_LIBRARY} )
  set( ODBC_INCLUDE_DIRS ${ODBC_INCLUDE_DIR} )
endif(ODBC_FOUND)

mark_as_advanced(ODBC_INCLUDE_DIR ODBC_LIBRARY)

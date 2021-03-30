# CMake module to search for PostgreSQL library
#
# pg_config is searched for in POSTGRES_CONFIG dir,
# default /usr/bin
#
# If it's found it sets POSTGRES_FOUND to TRUE
# and following variables are set:
#    POSTGRES_INCLUDE_DIR
#    POSTGRES_LIBRARY

IF(WIN32)
    FIND_PATH(POSTGRES_INCLUDE_DIR libpq-fe.h
        /usr/local/include /usr/include c:/msys/local/include
        $ENV{POSTGRESQL_PATH}/include/server
        $ENV{POSTGRESQL_PATH}/include
        "C:/Program Files/PostgreSQL/*/include/server"
        $ENV{ProgramFiles}/PostgreSQL/*/include/server
        $ENV{SystemDrive}/PostgreSQL/*/include/server
        "C:/Program Files/PostgreSQL/*/include"
        $ENV{ProgramFiles}/PostgreSQL/*/include
        $ENV{SystemDrive}/PostgreSQL/*/include
    )

    FIND_LIBRARY(POSTGRES_LIBRARY NAMES libpq pq PATHS
     /usr/local/lib /usr/lib c:/msys/local/lib
     $ENV{POSTGRESQL_PATH}/lib
     $ENV{ProgramFiles}/PostgreSQL/*/lib
     $ENV{SystemDrive}/PostgreSQL/*/lib
     "C:/Program Files/PostgreSQL/*/lib"
     $ENV{POSTGRESQL_PATH}/lib/ms
     $ENV{ProgramFiles}/PostgreSQL/*/lib/ms
     $ENV{SystemDrive}/PostgreSQL/*/lib/ms
     "C:/Program Files/PostgreSQL/*/lib/ms"
    )
ELSE(WIN32)
  IF(UNIX)

    SET(POSTGRES_CONFIG_PREFER_PATH "$ENV{POSTGRES_HOME}/bin" CACHE FILEPATH "preferred path to PG (pg_config)")
    FIND_PROGRAM(POSTGRES_CONFIG pg_config
      ${POSTGRES_CONFIG_PREFER_PATH}
      /usr/bin/
      "$ENV{POSTGRES_HOME}/bin"
      "$ENV{POSTGRES_PATH}/bin"
      )
    # MESSAGE("DBG POSTGRES_CONFIG ${POSTGRES_CONFIG}")

    IF (POSTGRES_CONFIG)
      # set INCLUDE_DIR
      EXEC_PROGRAM(${POSTGRES_CONFIG}
        ARGS --includedir
        OUTPUT_VARIABLE PG_TMP)
    ELSE(POSTGRES_CONFIG)
      SET(PG_TMP /opt/postgresql)
    ENDIF(POSTGRES_CONFIG)

      find_path(POSTGRES_INCLUDE_DIR libpq-fe.h
            /usr/local/include
            /usr/local/include/postgresql
            /usr/local/postgresql/include
            /usr/local/postgresql/include/postgresql
            /usr/include
            /usr/include/postgresql
            /usr/include/pgsql
            ${PG_TMP}
            $ENV{POSTGRESQL_HOME}/include/server
            $ENV{POSTGRESQL_HOME}/include
            $ENV{POSTGRESQL_PATH}/include/server
            $ENV{POSTGRESQL_PATH}/include
      )

      # set LIBRARY_DIR
      EXEC_PROGRAM(${POSTGRES_CONFIG}
        ARGS --libdir
        OUTPUT_VARIABLE PG_TMP)
      find_library(POSTGRES_LIBRARY pq
        PATHS
        ${PG_TMP}
        /usr/lib/postgresql
        /usr/local/lib
        /usr/local/lib/postgresql
        /usr/local/postgresql/lib
        $ENV{POSTGRESQL_HOME}/lib
        $ENV{POSTGRESQL_PATH}/lib
      )

  ENDIF(UNIX)
ENDIF(WIN32)

IF (POSTGRES_INCLUDE_DIR AND POSTGRES_LIBRARY)
    include(MacroCheckLibraryWorks)
    CHECK_LIBRARY_WORKS("libpq-fe.h" "PQclear(NULL);" "${POSTGRES_INCLUDE_DIR}" "${POSTGRES_LIBRARY}" "POSTGRES_WORKS")
   SET(POSTGRES_FOUND ${POSTGRES_WORKS} CACHE INTERNAL "PostgreSQL found")
ENDIF (POSTGRES_INCLUDE_DIR AND POSTGRES_LIBRARY)


IF (POSTGRES_FOUND)

   IF (NOT POSTGRES_FIND_QUIETLY)
      MESSAGE(STATUS "Found PostgreSQL: ${POSTGRES_INCLUDE_DIR}, ${POSTGRES_LIBRARY}")
   ENDIF (NOT POSTGRES_FIND_QUIETLY)

   check_library_exists("${POSTGRES_LIBRARY}" PQescapeStringConn "" HAVE_PQESCAPESTRINGCONN)

ELSE (POSTGRES_FOUND)

   IF (POSTGRES_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PostgreSQL")
   ELSE (POSTGRES_FIND_REQUIRED)
      MESSAGE(STATUS "Could not find PostgreSQL")
   ENDIF (POSTGRES_FIND_REQUIRED)

ENDIF (POSTGRES_FOUND)

project (Gammu-contrib-smscgi C)

include(GammuTuneCompiler)

# Test for build sqlreply code
if (MYSQL_FOUND)
    add_executable(sqlreply "c/michal.c")
    target_link_libraries (sqlreply ${MYSQL_LIBRARIES})
    include_directories (${MYSQL_INCLUDE_DIR})
endif (MYSQL_FOUND)

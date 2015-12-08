project (Gammu-contrib-smscgi C)

# Test for build sqlreply code
if (MYSQL_FOUND)
    if (WIN32)
        check_c_source_compiles("
#include <process.h>
int main(){
    const char * const* ptr;
    spawnv(_P_NOWAIT, \"foo\", ptr);
    return 0;
}
" SPAWNV_ARGUMENT_IS_CONST )
    endif (WIN32)


    add_executable(sqlreply "c/michal.c")
    target_link_libraries (sqlreply ${MYSQL_LIBRARIES})
    include_directories (${MYSQL_INCLUDE_DIR})
endif (MYSQL_FOUND)

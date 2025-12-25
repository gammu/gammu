MACRO (CHECK_LIBRARY_WORKS _header _code _include _library _target)


    set(CHECK_LIBRARY_WORKS_BACKUP_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
    set(CHECK_LIBRARY_WORKS_BACKUP_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_INCLUDES "${_include}")
    set(CMAKE_REQUIRED_LIBRARIES "${_library}")
    CHECK_C_SOURCE_COMPILES("
#ifdef _WIN32
#include <windows.h>
#endif
#include <${_header}>

int main(void) {
    ${_code}
    return 0;
}
" "${_target}")

    set(CMAKE_REQUIRED_INCLUDES "${CHECK_LIBRARY_WORKS_BACKUP_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${CHECK_LIBRARY_WORKS_BACKUP_LIBRARIES}")

ENDMACRO()

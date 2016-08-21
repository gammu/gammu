MACRO (CHECK_LIBRARY_WORKS _header _include _library _target)

    if (DEFINED ${_target})
        message(STATUS "${_target} passed")
    else()

    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c"
      "
//TEST
#include <${_header}>

int main(void) {
    return 0;
}
")

    try_compile(TMP_${_target}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c
      LINK_LIBRARIES "${_library}"
      COMPILE_DEFINITIONS "-I${_include}"
      OUTPUT_VARIABLE OUTPUT
    )
    
    if (TMP_${_target})
        message(STATUS "${_target} passed")
        set(${_target} TRUE CACHE INTERNAL "Test ${_target}")
    else()
        message(STATUS "${_target} failed")
        set(${_target} FALSE CACHE INTERNAL "Test ${_target}")
    endif()

    
    endif()

ENDMACRO()

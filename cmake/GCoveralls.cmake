if(__GCOVERALLS_CMAKE__)
  return()
endif()
set(__GCOVERALLS_CMAKE__ TRUE)


set(GCOVERALLS_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})

function(enable_gcoveralls)
  configure_file(${GCOVERALLS_LIST_DIR}/coveralls-upload.in ${CMAKE_BINARY_DIR}/coveralls-upload @ONLY)
endfunction()

project (Gammu-contrib-convert C)

# We use Gammu
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../include")

add_executable(makeconverttable "makeconverttable.c")
target_link_libraries(makeconverttable libGammu)

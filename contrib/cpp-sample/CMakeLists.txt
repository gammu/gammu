project(Gammu-cpp-example CXX)

# We use Gammu
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../include")

add_executable(cpp-gammu gammu.cc)
target_link_libraries(cpp-gammu libGammu)

add_executable(cpp-smsd smsd.cc)
target_link_libraries(cpp-smsd gsmsd)

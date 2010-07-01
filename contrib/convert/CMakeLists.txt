project (Gammu-contrib-convert C)

include(GammuTuneCompiler)

# We use Gammu
include_directories("${Gammu_BINARY_DIR}/include")

add_executable(makeconverttable "makeconverttable.c")
target_link_libraries(makeconverttable libGammu)

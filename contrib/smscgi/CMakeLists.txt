project (Gammu-contrib-smscgi C)

if (NOT WIN32 OR CYGWIN)
include(GammuTuneCompiler)

# We use Gammu
include_directories("${Gammu_BINARY_DIR}/include")

add_executable(smscgid "sms_cgi.c" "smsd.c")
target_link_libraries(smscgid libGammu)
endif (NOT WIN32 OR CYGWIN)

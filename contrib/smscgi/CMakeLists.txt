project (Gammu-contrib-smscgi C)

if (NOT WIN32 OR CYGWIN)

# We use Gammu
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../include")

add_executable(smscgid "sms_cgi.c" "smsd.c")
target_link_libraries(smscgid libGammu)
endif (NOT WIN32 OR CYGWIN)

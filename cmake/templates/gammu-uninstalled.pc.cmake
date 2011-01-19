libdir=@libGammu_BINARY_DIR@
includedir=@Gammu-headers_BINARY_DIR@

Name: gammu
Description: Gammu library
Version: @GAMMU_VERSION@
Libs: -L${libdir} -lGammu -lm @GAMMU_PRIVATE_LIBS@
Cflags: -I${includedir}

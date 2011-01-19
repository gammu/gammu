libdir=@libGammu_BINARY_DIR@
includedir=@Gammu-headers_BINARY_DIR@

Name: gammu-smsd
Description: Gammu SMSD library
Requires: gammu
Version: @GAMMU_VERSION@
Libs: -L${libdir} -lgsmsd -lm @SMSD_PRIVATE_LIBS@
Cflags: -I${includedir}

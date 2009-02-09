libdir=@Gammu_BINARY_DIR@/common
includedir=@Gammu_BINARY_DIR@/include

Name: gammu-smsd
Description: Gammu SMSD library
Requires: gammu
Version: @VERSION@
Libs: -L${libdir} -lgsmsd -lm @SMSD_PRIVATE_LIBS@
Cflags: -I${includedir}

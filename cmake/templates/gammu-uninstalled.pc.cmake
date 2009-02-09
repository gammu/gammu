libdir=@Gammu_BINARY_DIR@/common
includedir=@Gammu_BINARY_DIR@/include

Name: gammu
Description: Gammu library
Version: @VERSION@
Libs: -L${libdir} -lGammu -lm @GAMMU_PRIVATE_LIBS@
Cflags: -I${includedir}

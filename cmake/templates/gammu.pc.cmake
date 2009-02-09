prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/@INSTALL_LIB_DIR@
includedir=${prefix}/@INSTALL_INC_DIR@

Name: gammu
Description: Gammu library
Version: @VERSION@
Libs: -L${libdir} -lGammu -lm @GAMMU_PRIVATE_LIBS@
Cflags: -I${includedir}

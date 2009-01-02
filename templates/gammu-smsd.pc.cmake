prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/@INSTALL_LIB_DIR@
includedir=${prefix}/@INSTALL_INC_DIR@

Name: gammu-smsd
Description: Gammu SMSD library
Requires: gammu
Version: @VERSION@
Libs: -L${libdir} -lgsmsd -lm @SMSD_PRIVATE_LIBS@
Cflags: -I${includedir}

prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=${prefix}/@LIB_INSTALL_DIR@
includedir=${prefix}/@INCLUDE_INSTALL_DIR@

Name: Pinproc
Description: P-ROC interface library
Version: @PINPROC_VERSION@
Requires:
Libs: -L${libdir} -lpinproc
Cflags: -I${includedir}

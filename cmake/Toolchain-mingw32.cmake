# Toolchain configuration for SUSE/Fedora like MinGW32 setup

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_AR i686-w64-mingw32-ar)
SET(CMAKE_LD i686-w64-mingw32-ld)
SET(CMAKE_NM i686-w64-mingw32-nm)
SET(CMAKE_OBJCOPY i686-w64-mingw32-objcopy)
SET(CMAKE_OBJDUMP i686-w64-mingw32-objdump)
SET(CMAKE_RANLIB i686-w64-mingw32-ranlib)
SET(CMAKE_STRIP i686-w64-mingw32-strip)
SET(CMAKE_MC_COMPILER i686-w64-mingw32-windmc)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/i686-w64-mingw32/sys-root/mingw/ /usr/i686-pc-mingw32/sys-root/mingw/ /home/mcihar/win-cross/crosscompiled)

SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Windows libraries names
set(WIN_LIB_ICONV) # builtin
set(WIN_LIB_INTL libintl-8.dll)
set(WIN_LIB_CURL libcurl-4.dll libidn-11.dll libnspr4.dll nss3.dll libssh2-1.dll ssl3.dll zlib1.dll nssutil3.dll libplc4.dll libplds4.dll libgcrypt-11.dll libgpg-error-0.dll)
set(WIN_LIB_MYSQL libmysql.dll)
set(WIN_LIB_PGSQL libpq.dll)
set(WIN_LIB_GLIB libglib-2.0-0.dll libgobject-2.0-0.dll libiconv-2.dll libgthread-2.0-0.dll)

# Disable pkg-config lookups
set(PKG_CONFIG_EXECUTABLE /bin/false)

set(CMAKE_EXECUTABLE_SUFFIX .exe)

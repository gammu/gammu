# Toolchain configuration for Debian like MinGW32 setup with external libs

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
SET(CMAKE_RC_COMPILER i586-mingw32msvc-windres)
SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
SET(CMAKE_AR i586-mingw32msvc-ar)
SET(CMAKE_LD i586-mingw32msvc-ld)
SET(CMAKE_NM i586-mingw32msvc-nm)
SET(CMAKE_OBJCOPY i586-mingw32msvc-objcopy)
SET(CMAKE_OBJDUMP i586-mingw32msvc-objdump)
SET(CMAKE_RANLIB i586-mingw32msvc-ranlib)
SET(CMAKE_STRIP i586-mingw32msvc-strip)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc /home/mcihar/win-cross/crosscompiled)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Windows libraries names
set(WIN_LIB_ICONV libiconv-2.dll)
set(WIN_LIB_INTL libintl-8.dll)
set(WIN_LIB_CURL libcurl-4.dll mgwz.dll)
set(WIN_LIB_MYSQL libmysql.dll)
set(WIN_LIB_PGSQL libpq.dll)
set(WIN_LIB_GLIB libglib-2.0-0.dll libgobject-2.0-0.dll libiconv-2.dll libgthread-2.0-0.dll)

# Disable pkg-config lookups
set(PKG_CONFIG_EXECUTABLE /bin/false)

set(CMAKE_EXECUTABLE_SUFFIX .exe)

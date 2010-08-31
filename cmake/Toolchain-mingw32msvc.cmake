# Toolchain configuration for Debian like MinGW32 setup with external libs

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)

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

.. _installing:

Installing Gammu
================

Prebuilt Binaries for Linux
---------------------------

Many distributions come with prebuilt Gammu binaries, if you can use
them, it is definitely the easiest thing. There are also binary packages
of latest release built for many distributions available on Gammu home
page <http://wammu.eu/gammu/>.

You can usually also find Gammu in your distribution, so unless you need a
newer version, just install package from your distribution.

Debian
++++++

Gammu packages are included in Debian (testing versions go to
experimental and stable to unstable). If you want to build Debian
package on your own, you can find packaging in Git repository at
``git://git.debian.org/git/collab-maint/gammu.git`` (you can browse it on
<http://git.debian.org/?p=collab-maint/gammu.git>).

RPM
+++

Gammu packages are included in openSUSE and Fedora. Additionally source tarball
contains :file:`gammu.spec` which you can use for building RPM package.

Slackware
+++++++++

Gammu packages are included in Gentoo. Additionally source tarball contains
:file:`description-pak` which you can use for building Slackware package.

Prebuilt Binaries for Windows
-----------------------------

You can download Windows binaries from <http://wammu.eu/gammu/>. For
Windows 95, 98 and NT 4.0 you will also need ShFolder DLL, which can be
downloaded from Microsoft:

http://www.microsoft.com/downloads/details.aspx?displaylang=en&FamilyID=6AE02498-07E9-48F1-A5D6-DBFA18D37E0F


.. _deps:

Dependencies
------------

You need CMake from <http://www.cmake.org/> for compiling Gammu.

Additionally pkg-config <http://pkg-config.freedesktop.org/> is used for
detecting available libraries.

.. _opt-deps:

Optional Dependencies
---------------------

Gammu does not require any special libraries at all to build, but you might
miss some features. Optional libraries include:

Bluez-libs
++++++++++

Required for Bluetooth support on Linux.

.. seealso:: http://www.bluez.org/

libusb-1.0
++++++++++

Required for fbususb/dku2 connection support on Linux.

.. seealso:: http://libusb.sourceforge.net/

libCURL
+++++++

Required for new versions notification (see :option:`gammu checkversion`) and
OpenCellID access (see :option:`gammu getlocation`).

.. seealso:: http://curl.haxx.se/libcurl/

libiconv
++++++++

Used to support more character sets in AT engine.

.. seealso:: http://www.gnu.org/software/libiconv/

Gettext
+++++++

Localization of strings.

.. seealso:: http://www.gnu.org/software/gettext/

MySQL
+++++

Required for :ref:`gammu-smsd-mysql` in :ref:`smsd`.

.. seealso:: http://mysql.com/

PostgreSQL
++++++++++

Required for :ref:`gammu-smsd-pgsql` in :ref:`smsd`.

.. seealso:: http://www.postgresql.org/

unixODBC
++++++++

Required for :ref:`gammu-smsd-odbc` in :ref:`smsd`.

.. note:: Not needed on platforms having native ODBC support such as Microsoft Windows.

.. seealso:: http://www.unixodbc.org/

libdbi
++++++

Required for :ref:`gammu-smsd-dbi` in :ref:`smsd`.

.. note:: Required at least version 0.8.2.

.. seealso:: http://libdbi.sourceforge.net/

Python
++++++

Gammu has a Python bindings, see :ref:`python`.

.. seealso:: http://www.python.org/

SQLite + libdbi-drivers with SQLite
+++++++++++++++++++++++++++++++++++

Needed for testing of SMSD using libdbi driver (libdbd-sqlite3), see :ref:`testing`.

.. seealso:: http://www.sqlite.org/

glib
++++

Currently needed only for :ref:`gammu-detect`.

.. seealso:: http://www.gtk.org/

gudev
+++++

Currently needed only for :ref:`gammu-detect`.

.. seealso:: http://gudev.sourceforge.net/


Compiling on Linux/Unix Systems
-------------------------------

First install all :ref:`deps` and :ref:`opt-deps`. Do not forget to install
corresponding devel packages as well, they are usually named with ``-dev`` or
``-devel`` suffix, depending on your distribution.

For example on Debian or Ubuntu, you can install all optional packages by
following command:

.. code-block:: sh

    apt-get install cmake python-dev pkg-config libmysqlclient-dev libpq-dev \
        libcurl4-gnutls-dev libusb-1.0-0-dev libdbi0-dev libbluetooth-dev \
        libgudev-1.0-dev libglib2.0-dev unixodbc-dev

For openSUSE, the installation all optional packages could look like:

.. code-block:: sh

    zypper install libusb-1_0-devel libdbi-devel bluez-devel postgresql-devel \
        mysql-devel python-devel libcurl-devel cmake pkgconfig unixODBC-devel \
        glib2-devel libgudev-1_0-devel

For compatibility reasons, configure like wrapper is provided, if you
don't need much specific tuning, you can use usual set of commands:

.. code-block:: sh

    ./configure
    make
    sudo make install
    
The configure wrapper will create directory build-configure and build all
binaries there (nothing is changed in source tree), for example gammu binary is
in build-configure/gammu directory.

If you need/want to tweak build a bit more than configure wrapper
provides, you have to use `CMake`_ directly. For now, only out of source
build is supported, so you have to create separate directory for build:

.. code-block:: sh

    mkdir build
    cd build

Then just configure project:

.. code-block:: sh

    cmake ..

Build it:

.. code-block:: sh

    make

Test that everything is okay:

.. code-block:: sh

    make test

And finally install it:

.. code-block:: sh

    sudo make install

You can configure build parameters either by command line (see
parameters bellow), or using TUI - ccmake.

Useful cmake parameters:

* ``-DBUILD_SHARED_LIBS=ON`` enables shared library
* ``-DCMAKE_BUILD_TYPE="Debug"`` enables debug build
* ``-DCMAKE_INSTALL_PREFIX="/usr"`` change installation prefix
* ``-DENABLE_PROTECTION=OFF`` disables various compile time protections
  against buffer overflows and similar attacks
* ``-DBUILD_PYTHON=/usr/bin/python2.6`` changes Python used for build Python
  module
* ``-DWITH_PYTHON=OFF`` disables build of python-gammu

You can also disable support for whole set of phones, e.g.:

* ``-DWITH_NOKIA_SUPPORT=OFF`` disables Nokia phones support
* ``-DWITH_BLUETOOTH=OFF`` disables Bluetooth support
* ``-DWITH_IRDA=OFF`` disables IrDA support

Library search paths
++++++++++++++++++++

By installing Gammu to non default system paths, you might need to add path
where libGammu and other Gammu liraries are installed to :program:`ldconfig`
search path.

You can do this by editing :file:`/etc/ld.so.conf` or adding new file to
:file:`/etc/ld.so.conf.d/` directory containing path, wherge Gammu library has
been installed. Some examples:

.. code-block:: sh

    # Gammu on 64-bit Fedora installed to /opt/gammu
    echo /opt/gammu/lib64 > /etc/ld.so.conf.d/gammu.conf

    # Gammu installed to /usr/local
    echo /usr/local/lib > /etc/ld.so.conf.d/gammu.conf

The similar situation exists with Python modules, if you install in path when
your Python interpreter does not search it won't load newly installed Gammu
bindings.

You can also avoid changing ldconfig configuration by installing Gammu to paths
where it already searches, for examble by:

.. code-block:: sh

   cmake .. -DCMAKE_INSTALL_PREFIX="/usr"


Compiling on Microsoft Windows
------------------------------

First install all :ref:`deps` and :ref:`opt-deps`.

`CMake`_ is able to generate projects for various tools including Microsoft
Visual Studio, Borland toolchains, Cygwin or Mingw32. Just click on
CMakeLists.txt in project sources and configure CMake to be able to find
optional libraries (see cross compilation section for more information about
getting those). The result should be project for your compiler where you
should be able to work with it as with any other project.

Compiling using MS Visual C++
+++++++++++++++++++++++++++++

You will probably need additional SDKs:

* Microsoft Windows Platform SDK (required especially for Bluetooth).
  It's given for free. Below are links to different releases (if you
  have problems with latest one, use older). They work for various
  Windows versions, even though Microsoft named them Windows Server 2003
  Platform SDK.
* For free Visual C++ Express 2005 you need to set compiler to work with
  Platform SDK (see description).
* MySQL include/library files from MySQL install package (for MySQL
  support in SMSD).
* PostgreSQL include/library files from PostgreSQL install package (for
  PostgreSQL support in SMSD).
* For gettext (internationalization) support, you will need gettext
  packages from GnuWin32 project.
* As build is now based on CMake, you will need to get it from
  http://cmake.org/.

After downloading and installing them into your system:

* Now you should be able to execute cmake by clicking on CMakeLists.txt
  file in Gammu sources, this should pop up dialog with configuration
  options.

  * You can also start CMakeSetup from start menu and select source
    directory (just point to it to Gammu sources).
  * Select directory where binaries will be stored, I suggest this is
    different than source one, eg. append subdirectory build.
  * Select compiler you want to use in Build for select.

* In list bellow, you can tweak paths to some optional libraries and
  project configuration.
* Then just press Configure button, which will do the hard job. After
  this, just click OK button to generate Visual Studio project.
* Project files for Visual Studio should be now generated in directory
  you selected, just open it in Visual Studio and compile :-).

  * Project file should be named Gammu.dsw or Gammu.sln depending on
    what MSVC version you choose.
  * You should see ALL_BUILD target, which builds everything needed,
    simmilar to make all on Linux.

* I know this guide is incomplete, I don't have environment to test,
  you're welcome to improve it!. Some more information can be found in
  howtos for other projects using CMake, eg. Blender, SIM, KDE, VTK,
  ISGTK. ITK, [wxWidgets http://www.wxwidgets.org/wiki/index.php/CMake].

Compiling using Borland C++
+++++++++++++++++++++++++++

Borland toolchain - you can download compiler at
<http://www.codegear.com/downloads/free/cppbuilder>. You need to add
c:/Borland/BCC55/Bin to system path (or manually set it when running
CMake) and add -Lc:/Borland/BCC55/Lib -Ic:/Borland/BCC55/Include
-Lc:/Borland/BCC55/Lib/PSDK to CMAKE_C_FLAGS in CMake (otherwise
compilation fails).

Compiling using Cygwin
++++++++++++++++++++++

This should work pretty much same as on Linux.

Compiling on Mac OS X
---------------------

First install all :ref:`deps` and :ref:`opt-deps`.

Gammu should be compilable on Mac OS X, you need to have installed
Developer Tols (version 2.4.1 was tested) and `CMake`_ (there is a Mac OS X
"Darwin" DMG download). For database support in SMSD, install wanted
database, eg. MySQL.

The rest of the compilation should be pretty same as on Linux, see Linux
section for more details about compile time options.

If you get some errors while linking with iconv, it is caused by two
incompatible iconv libraries available on the system. You can override the
library name:

.. code-block:: sh

   cmake -D ICONV_LIBRARIES="/opt/local/lib/libiconv.dylib" ..

Or completely disable iconv support:

.. code-block:: sh

   cmake -DWITH_Iconv=OFF ..


Cross compilation for Windows on Linux
--------------------------------------

First install all :ref:`deps` and :ref:`opt-deps` into your mingw build
environment.

Only cross compilation using `CMake`_ has been tested. You need to install
MinGW cross tool chain and run time. On Debian you can do it by apt-get
install mingw32. Build is then quite simple:

.. code-block:: sh

    mkdir build-win32
    cd build-win32
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake
    make

There is also toolchain configuration for Win64 available:

.. code-block:: sh

    mkdir build-win64
    cd build-win64
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw64.cmake
    make

If your MinGW cross compiler binaries are not found automatically, you
can specify their different names in cmake/Toolchain-mingw32.cmake.

To build just bare static library without any dependencies, use:

.. code-block:: sh

    cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake \
        -DBUILD_SHARED_LIBS=OFF \
        -DWITH_MySQL=OFF \
        -DWITH_Postgres=OFF \
        -DWITH_GettextLibs=OFF \
        -DWITH_Iconv=OFF \
        -DWITH_CURL=OFF

To be compatible with current Python on Windows, we need to build
against matching Microsoft C Runtime library. For Python 2.4 and 2.5
MSVCR71 was used, for Python 2.6 the right one is MSVCR90. To achieve
building against different MSVCRT, you need to adjust compiler
specifications, example is shown in cmake/mingw.spec, which is used by
CMakeLists.txt. You might need to tune it for your environment.

Third party libraries
+++++++++++++++++++++

The easies way to link with third party libraries is to add path to
their installation to cmake/Toolchain-mingw32.cmake or to list these
paths in CMAKE_FIND_ROOT_PATH when invoking cmake.


MySQL
~~~~~

You can download MySQL binaries from <http://dev.mysql.com/>, but then
need some tweaks:

.. code-block:: sh

    cd mysql/lib/opt
    reimp.exe -d libmysql.lib
    i586-mingw32msvc-dlltool --kill-at --input-def libmysql.def \
        --dllname libmysql.dll --output-lib libmysql.a

reimp.exe is part of mingw-utils and can be run through wine, I didn't
try to compile native binary from it.


PostgreSQL
~~~~~~~~~~

You can download PostgreSQL binaries from <http://www.postgresql.org/>,
but then you need to add wldap32.dll library to bin.


Gettext
~~~~~~~

For Gettext (internationalization support), you need
gettext-0.14.4-bin.zip, gettext-0.14.4-dep.zip, gettext-0.14.4-lib.zip
from <http://gnuwin32.sourceforge.net/>. Unpack these to same directory.


CURL
~~~~

For CURL support, you need curl-7.19.0-devel-mingw32.zip from
<http://curl.haxx.se/>.

Crosscompiling to different platform
------------------------------------

To cross compile Gammu to different architecture (or platform) you need to
provide CMake toolchain file for that and invoke `CMake`_ with it:

.. code-block:: sh

    cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-eldk-ppc74xx.cmake ..

More information on creating that is described in `CMake Cross Compiling`_ wiki
page. Also distributions like `OpenEmbedded`_ usually already come with
prepared recipes for `CMake`_.

.. _CMake Cross Compiling: http://www.vtk.org/Wiki/CMake_Cross_Compiling
.. _OpenEmbedded: http://www.openembedded.org/

Advanced Build Options
----------------------

The build system accepts wide range of options. You can see them all by
running GUI version of `CMake`_ or by inspecting :file:`CMakeCache.txt` in
build directory.

Limiting set of installed data
++++++++++++++++++++++++++++++

By setting following flags you can control which additional parts will
be installed:

* INSTALL_GNAPPLET - Install Gnapplet binaries
* INSTALL_MEDIA - Install sample media files
* INSTALL_PHP_EXAMPLES - Install PHP example scripts
* INSTALL_BASH_COMPLETION - Install bash completion script for Gammu
* INSTALL_LSB_INIT - Install LSB compatible init script for Gammu
* INSTALL_DOC - Install documentation
* INSTALL_LOC - Install locales data

For example:

.. code-block:: sh

    cmake -DINSTALL_DOC=OFF


Debugging build failures
++++++++++++++++++++++++

If there is some build failure (eg. some dependencies are not correctly
detected), please attach :file:`CMakeCache.txt`,
:file:`CMakeFiles/CMakeError.log` and :file:`CMakeFiles/CMakeOutput.log` files
to the report. It will help diagnose what was detected on the system and
possibly fix these errors.

To find out what is going on during compilation, add
``-DCMAKE_VERBOSE_MAKEFILE=ON`` to :program:`cmake` command line or run
:program:`make` with ``VERBOSE=1``:

.. code-block:: sh
    
    make VERBOSE=1

Debugging crashes
+++++++++++++++++

To debug program crashes, you might want to build Gammu with
``-DENABLE_PROTECTION=OFF``, otherwise debugging tools are somehow confused
with protections GCC makes and produce bogus back traces.


Compliling python-gammu
+++++++++++++++++++++++

Currently python-gammu is distributed together with Gammu, so all you need
to get it is to build Gammu with Python support (it should be automatically
detected if you have development environment installed for Python).

Gammu uses CMake_ to generate build environment (for example Makefiles for
UNIX, Visual Studio projects, Eclipse projects, etc.) which you can later use
for building. You can use ``-DBUILD_PYTHON=/path/to/python`` to define path to
another Python interpreter to use than default one available in the system.

Alternatively you can use standard distutils, for which :file:`setup.py` is placed in
`python` subdirectory.

.. _CMake: http://www.cmake.org/

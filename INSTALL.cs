Gammu All Mobile Management Utilities - Instalace
=================================================


Binárky - Linux
===============

Mnoho distribucí již obsahuje Gammu, takže pokud můžete použít tuto verzi,
bude to pro vás nejjednodušší. Binární balíčky posledních verzí pro mnoho
distribucí naleznete také na stránkách Gammu - <http://cs.wammu.eu/gammu/>.


Binárky - Windows
=================

Binárky pro Windows si můžete stáhnout z <http://cs.wammu.eu/gammu/>. Pro
Windows 95, 98 a NT 4.0 budete také potřebovat ShFolder DLL, která může být
stažena od Microsoftu:

http://www.microsoft.com/downloads/details.aspx?displaylang=en&FamilyID=6AE02498-07E9-48F1-A5D6-DBFA18D37E0F


Ze zdrojového kódu - požadavky
==============================

Gammu je možné zkompilovat bez jakýchkoliv dalších knihoven, ale můžete
postrádat některé funkce. Volitelné závislosti jsou:

Bluez-libs 
    - http://www.bluez.org/
    - Podpora Bluetooth na Linuxu.

libusb-1.0
    - http://libusb.sourceforge.net/
    - připojení fbususb

libCURL
    - http://curl.haxx.se/libcurl/
    - Notifikace o nových verzích a přístup do databáze telefonů.

libiconv
    - http://www.gnu.org/software/libiconv/
    - Podpora pro víc kódování v AT subsystému.

Gettext
    - http://www.gnu.org/software/gettext/
    - Překlady textů.

MySQL
    - http://mysql.com/
    - podpora MySQL v SMSD.

PostgreSQL
    - http://www.postgresql.org/
    - podpora PostgreSQL v SMSD.

libdbi
    - http://libdbi.sourceforge.net/
    - je nutná alespoň verze 0.8.2
    - podpora DBI v SMSD.
        - pro testování prosím nainstalujte libdbd-sqlite3

Python
    - http://www.python.org/
    - Gammu poskytuje rozhraní v Pythonu

SQLite + libdbi-drivers se SQLite
    - http://www.sqlite.org/
    - potřeba pro testování SMSD s ovladačem libdbi


Ze zdrojového kódu - Linux
==========================

Pro kompilaci Gammu potřebujete CMake ze stránek <http://www.cmake.org>.

Kvůli kompatibilitě Gammu obsahuje jednoduchý wrapper který se chová jako
configure skript, takže můžete použít obvyklou sadu příkazů "./configure;
make; sudo make install". Kompilace probíhá v adresáži build-configure  (ve
stromu se zdrojovými kódy se nic nemění), například program gammu je
kompilován v adresáři build-configure/gammu.

Pokud chcete nebo potřebujete nastavit kompilaci víc než umožňuje wrapper
pro configure, musíte použít přímo CMake. V současné době je podporována jen
kompilace mimo zdrojový strom, takže musíte vytvořit samostatný adresář pro
kompilaci:

> mkdir build > cd build

Poté zkonfigurujte projekt:

> cmake ..

Zkompilujte ho:

> make

Otestujte jestli je všechno v pořádku:

> make test

A nakonec ho nainstalujte:

> sudo make install

Parametry můžete měnit na příkazové řádce (jak je popsáno níže), nebo pomocí
textového rozhraní ccmake.

Užitečné parametry cmake:

* -DBUILD_SHARED_LIBS=ON povolí sdílenou knihovnu
* -DCMAKE_BUILD_TYPE="Debug" zapne ladicí build
* -DCMAKE_INSTALL_PREFIX="/usr" změní prefix pro instalaci
* -DENABLE_PROTECTION=OFF vypne různé ochrany proti přetečení bufferu a
  podobným útokům
* -DBUILD_PYTHON=/usr/bin/python2.6 změní Python, který bude použit pro
  kompilaci modulu pro Python
* -DWITH_PYTHON=OFF vypne kompilaci python-gammu

Můžete také vypnout podporu pro různé druhy telefonů, např.:

* -DWITH_NOKIA_SUPPORT=OFF vypne podporu pro Nokie
* -DWITH_BLUETOOTH=OFF vypne podporu pro Bluetooth
* -DWITH_IRDA=OFF vypne podporu pro IrDA

Omezení nainstalovaných věcí
============================

Nastavením následujících parametrů můžete ovlivnit které volitelné části
budou nainstalovány:

* INSTALL_GNAPPLET - instalovat binárky Gnappletu
* INSTALL_MEDIA - instalovat ukázkové soubory s logy a vyzváněními
* INSTALL_PHP_EXAMPLES - instalovat ukázkové skripty v PHP
* INSTALL_BASH_COMPLETION - instalovat skript na doplňování parametrů Gammu
  v bashi
* INSTALL_LSB_INIT - instalovat LSB kompatibilní init skript pro Gammu
* INSTALL_DOC - instalovat dokumentaci
* INSTALL_LOC - instalovat data překladu aplikace

Například:

-DINSTALL_DOC=OFF


Ze zdrojového kódu - Windows
============================

You need CMake from <http://www.cmake.org> for configuring Gammu. CMake is
able to generate projects for various tools including Microsoft Visual
Studio, Borland toolchains, Cygwin or Mingw32. Just click on CMakeLists.txt
in project sources and configure CMake to be able to find optional libraries
(see cross compilation section for more information about getting
those). The result should be project for your compiler where you should be
able to work with it as with any other project.

Podrobnější informace naleznete na wiki:
http://www.gammu.org/wiki/index.php?title=Gammu:Compiling/installing_in_Windows

Borland toolchain - you can download compiler at
<http://www.codegear.com/downloads/free/cppbuilder>. You need to add
c:/Borland/BCC55/Bin to system path (or manually set it when running CMake)
and add -Lc:/Borland/BCC55/Lib -Ic:/Borland/BCC55/Include
-Lc:/Borland/BCC55/Lib/PSDK to CMAKE_C_FLAGS in CMake (otherwise compilation
fails).

Ze zdrojového kódu - Mac OS X
=============================

Gammu by mělo jít zkompilovat na Mac OS X, aktuální informace by měly být na
wiki:

http://www.gammu.org/wiki/index.php?title=Gammu:Compiling/installing_on_Mac_OS_X


Křížová kompilace pro Windows na Linuxu
=======================================

Only cross compilation using CMake has been tested. You need to install
MinGW cross tool chain and run time. On Debian you can do it by apt-get
install mingw32. Build is then quite simple:

mkdir build-win32 cd build-win32 cmake
.. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake make

If your MinGW cross compiler binaries are not found automatically, you can
specify their different names in cmake/Toolchain-mingw32.cmake.

To build just bare static library without any dependencies, use:

cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake \
    -DBUILD_SHARED_LIBS=OFF \
    -DWITH_MySQL=OFF \
    -DWITH_Postgres=OFF \
    -DWITH_GettextLibs=OFF \
    -DWITH_Iconv=OFF \
    -DWITH_CURL=OFF \

To be compatible with current Python on Windows, we need to build against
MSVCR71, to achieve this, libmoldname71 library is needed. It is part of
current MinGW releases, but for older ones you can build it following way:

1. Get moldname.def.in from MinGW-runtime sources.
2. Get MinGW-runtime.
3. Execute following commands (crafted for Debian):

i586-mingw32msvc-ar x /usr/i586-mingw32msvc/lib/libmoldname.a isascii.o
iscsym.o iscsymf.o toascii.o strcasecmp.o strncasecmp.o wcscmpi.o
i586-mingw32msvc-gcc -DRUNTIME=71 -D__FILENAME__=moldname-71.def
-D__MSVCRT__ -C -E -P -xc-header moldname.def.in >moldname-71.def
i586-mingw32msvc-dlltool -k -U --dllname msvcr71.dll --def moldname-71.def
--output-lib libmoldname71.a i586-mingw32msvc-ar rc libmoldname71.a
isascii.o iscsym.o iscsymf.o toascii.o strcasecmp.o strncasecmp.o wcscmpi.o
i586-mingw32msvc-ranlib libmoldname71.a

Information taken from <http://www.pygame.org/wiki/PreparingMinGW>.


Externí knihovny
----------------

The easies way to link with third party libraries is to add path to their
installation to cmake/Toolchain-mingw32.cmake or to list these paths in
CMAKE_FIND_ROOT_PATH when invoking cmake.


MySQL
-----

Binární balíčky MySQL můžete stáhnout z <http://dev.mysql.com/>, po
rozbalení je potřeba provést pár úprav:

cd mysql/lib/opt
reimp.exe -d libmysql.lib
i586-mingw32msvc-dlltool --kill-at --input-def libmysql.def \
    --dllname libmysql.dll --output-lib libmysql.a

reimp.exe is part of mingw-utils and can be run through wine, I didn't try
to compile native binary from it.


PostgreSQL
----------

Můžete si stáhnout binární balíčky PostgreSQL z
<http://www.postgresql.org/>, ale budete muset odjinud přidat knihovnu
wldap32.dll do adresáře bin.


Gettext
-------

Pro podporu překladu programu potřebujete gettext-0.14.4-bin.zip,
gettext-0.14.4-dep.zip, gettext-0.14.4-lib.zip z
<http://gnuwin32.sourceforge.net/>. Rozbalte je všechny do jednoho adresáře.


CURL
----

Pro podporu CURL potřebujete curl-7.19.0-devel-mingw32.zip z
<http://curl.haxx.se/>.


Testování
=========

Gammu comes with testsuite which should be run after build. You can do this
using 'make test'. CMake build system uses for testing CTest, which also
includes option to connect to dashboard and submit test results there, so
that they can be reviewed and fixed by others. To participate in this
testing, you need just to run 'make Experimental'. It will compile current
version, run tests and submit data to dashboard:

http://cdash.cihar.com/index.php?project=Gammu

There are some more options for testing:

- make ExperimentalMemCheck

    This checks memory accesses using valgrind during tests and submits
    report. You need to do this after 'make Experimental' and you can
    submit results using 'make ExperimentalSubmit'.

- pokrytí testy

    To get test coverage reports, you need to configure project using 
    'cmake -DCOVERAGE=ON'

- noční testování

    Currently several machines do compile and test Gammu every night. If
    you want to tak part of this, just ensure that your machine executes
    test suite every night (preferably after 3:00 CET). You can select
    either 'make Nightly' to do regullar testing or 'make
    NightlyMemoryCheck' to test with valgrind. Also you can enable
    coverage tests as described above.

- zvláštní případy:

    You can enable some additional tests, which require some external
    components to be setup and are disabled by default:

    MYSQL_TESTING - you need to have setup MySQL server with database
    where SMSD can play.

    PSQL_TESTING - you need to have setup PostgreSQL server with
    database where SMSD can play.


# vim: et ts=4 sw=4 sts=4 tw=72 spell spelllang=en_us

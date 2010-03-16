Gammu Testsuite
===============

Gammu comes with testsuite which should be run after build. You can do this
using ``make test``. CMake build system uses for testing CTest, which also
includes option to connect to dashboard and submit test results there, so that
they can be reviewed and fixed by others. To participate in this testing, you
need just to run 'make Experimental'. It will compile current version, run
tests and submit data to dashboard:

http://cdash.cihar.com/index.php?project=Gammu

There are some more options for testing:

``make ExperimentalMemCheck``

    This checks memory accesses using valgrind during tests and submits
    report. You need to do this after ``make Experimental`` and you can
    submit results using ``make ExperimentalSubmit``.

Coverage reports

    To get test coverage reports, you need to configure project using 
    ``cmake -DCOVERAGE=ON``

Nightly testing

    Currently several machines do compile and test Gammu every night. If
    you want to tak part of this, just ensure that your machine executes
    test suite every night (preferably after 3:00 CET). You can select
    either ``make Nightly`` to do regullar testing or 
    ``make NightlyMemoryCheck`` to test with valgrind. Also you can enable
    coverage tests as described above.


Testing of SMSD
---------------

SMSD tests are performed using :doc:`dummy-driver` and uses file backend and
sqlite database by default (if Gammu is compiled with libdbi). Testing of
additional database backends must be enabled separately:

``MYSQL_TESTING``:
    you need to have setup MySQL server with database where SMSD can play.

``PSQL_TESTING``
    you need to have setup PostgreSQL server with database where SMSD can play.

Testing of command line utility
-------------------------------

Gammu command line tests are performed using :doc:`dummy-driver` where
required. It covers most of command line interface, but some parts need to be
explicitly enabled:

``ONLINE_TESTING``:
    enable testing of features which require internet access

Testing of Python interface
---------------------------

Python module tests are performed using :doc:`dummy-driver` where required. It
does also cover testing of SMSD interface, which is done using libdbi(sqlite)
driver.

Testing of reply functions
--------------------------

The :file:`tests` directory contains various tests which do inject data into
reply functions and check their response.

Testing of data parsing
-----------------------

The :file:`tests` directory contains various tests which just try to parse
various file formats supported by libGammu.

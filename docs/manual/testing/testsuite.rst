Gammu Testsuite
===============

Gammu comes with quite big test suite. It covers some basic low level
functions, handling replies from the phone and also does testing of command
line utilities and SMSD.


Running the tests
-----------------

You can run the test suite this using ``make test``. CMake build system uses
for testing CTest, which also includes option to connect to dashboard and
submit test results there, so that they can be reviewed and fixed by others. To
participate in this testing, you need just to run ``make Experimental`` which
also does submission to the dashboard.

There are some more options for testing:

``make test``

    Runs testsuite with no uploading of results.

``make Experimental``

    Runs testsuite and uploads results to the dashboard.

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

Running single test

    You can run single test by directly calling ctest::

        ctest -R test-name

    Adding ``-V`` runs it in verbose mode with all test output::

        ctest -V -R test-name

Collecting results
------------------

The tests are ran daily on several platforms and you can find the
results on `dashboard <https://cdash.cihar.com/index.php?project=Gammu>`_.

You are welcome to join this effort, all you need is to setup job to pull
current Gammu sources and execute the test suite every day (the preferred time
it 3:00 CET)::

    git pull
    make -C build-configure Nightly

or also with checking for memory leaks::

    git pull
    make -C build-configure NightlyMemCheck

Testing of SMSD
---------------

SMSD tests are performed using :ref:`dummy-driver` and uses file backend and
sqlite database by default. For this you nee Gammu compiled with libdbi, have
installed sqlite driver for libdbi and have :program:`sqlite3` binary available
on the syste,.

Testing of additional database backends must be enabled separately:

``MYSQL_TESTING``:
    you need to have setup MySQL server with database where SMSD can play.

``PSQL_TESTING``
    you need to have setup PostgreSQL server with database where SMSD can play.

Testing of command line utility
-------------------------------

Gammu command line tests are performed using :ref:`dummy-driver` where
required. It covers most of command line interface, but some parts need to be
explicitly enabled:

``ONLINE_TESTING``:
    enable testing of features which require internet access

Testing of Python interface
---------------------------

Python module tests are performed using :ref:`dummy-driver` where required. It
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

Configuration of the test suite
-------------------------------

You can pass various parameters to configure the test suite:

Programs used for testing
+++++++++++++++++++++++++

``SH_BIN``
    Path to the :program:`sh` program
``BASH_BIN``
    Path to the :program:`bash` program
``SQLITE_BIN``
    Path to the :program:`sqlite3` program
``SED_BIN``
    Path to the :program:`sed` program
``MYSQL_BIN``
    Path to the :program:`mysql` program
``PSQL_BIN``
    Path to the :program:`psql` program

Limiting testsuite
++++++++++++++++++

``ONLINE_TESTING``
    Enable testing of parts which use remote servers, requires connection to interned
``PSQL_TESTING``
    Enable testing of PostgreSQL SMSD backend, requires configured PostgreSQL database
``MYSQL_TESTING``
    Enable testing of MySQL SMSD backend, requires configured MySQL database

Database backends configuration
+++++++++++++++++++++++++++++++

``PSQL_HOST``
    Host to use for PostgreSQL tests (default: ``127.0.0.1``)
``PSQL_DATABASE``
    Database to use for PostgreSQL tests (default: ``smsd``)
``PSQL_USER``
    User to use for PostgreSQL tests (default: ``smsd``)
``PSQL_PASSWORD``
    Password to use for PostgreSQL tests (default: ``smsd``)
``MYSQL_HOST``
    Host to use for MySQL tests (default: ``127.0.0.1``)
``MYSQL_DATABASE``
    Database to use for MySQL tests (default: ``smsd``)
``MYSQL_USER``
    User to use for MySQL tests (default: ``smsd``)
``MYSQL_PASSWORD``
    Password to use for MySQL tests (default: ``smsd``)
``ODBC_DSN```
    ODBC DSN to use for ODBC tests (default: ``smsd``). 
    Currently needs to point to MySQL database.

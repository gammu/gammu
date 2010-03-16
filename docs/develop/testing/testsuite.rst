Gammu Testsuite
===============

Gammu comes with testsuite which should be run after build. You can do
this using 'make test'. CMake build system uses for testing CTest, which
also includes option to connect to dashboard and submit test results
there, so that they can be reviewed and fixed by others. To participate
in this testing, you need just to run 'make Experimental'. It will
compile current version, run tests and submit data to dashboard:

http://cdash.cihar.com/index.php?project=Gammu

There are some more options for testing:

- ``make ExperimentalMemCheck``

    This checks memory accesses using valgrind during tests and submits
    report. You need to do this after ``make Experimental`` and you can
    submit results using ``make ExperimentalSubmit``.

- coverage reports

    To get test coverage reports, you need to configure project using 
    ``cmake -DCOVERAGE=ON``

- nightly testing

    Currently several machines do compile and test Gammu every night. If
    you want to tak part of this, just ensure that your machine executes
    test suite every night (preferably after 3:00 CET). You can select
    either ``make Nightly`` to do regullar testing or 
    ``make NightlyMemoryCheck`` to test with valgrind. Also you can enable
    coverage tests as described above.

- special cases:

    You can enable some additional tests, which require some external
    components to be setup and are disabled by default:

    ``MYSQL_TESTING`` - you need to have setup MySQL server with database
    where SMSD can play.

    ``PSQL_TESTING`` - you need to have setup PostgreSQL server with
    database where SMSD can play.


The tests can be separated to several groups.

Testing of SMSD
---------------

SMSD tests are performed using :doc:`dummy-driver` and uses 

Testing of command line utility
-------------------------------

Testing of Python interface
---------------------------

Testing of reply functions
--------------------------

Testing of data parsing
-----------------------

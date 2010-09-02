Testing
=======

Gammu comes with quite big test suite. It covers some basic low level
functions, handling replies from the phone and also does testing of command
line utilities and SMSD.

Running the tests
-----------------

You can run the test suite this using ``make test``. CMake build system uses for
testing CTest, which also includes option to connect to dashboard and submit
test results there, so that they can be reviewed and fixed by others. To
participate in this testing, you need just to run ``make Experimental``.

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

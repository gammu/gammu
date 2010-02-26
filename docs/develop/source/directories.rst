Directory structure
===================

libgammu directory
------------------

This directory contains sources of Gammu library. You can find all phone
communication and data encoding functionality here.

gammu directory
---------------

Sources of Gammu command line utility. It contains interface to libGammu
and some additional functionality as well.

smsd directory
--------------

Sources of SMS Daemon as well as all it's service backends.

python directory
----------------

Sources of python-gammu module and some examples.

helper directory
----------------

These are some helper functions used either as replacement for
functionality missing on some platforms (eg. strptime) or used in more
places (message command line processing which is shared between SMSD and
Gammu utility).

docs directory
--------------

Documentation for both end users and developers as well as SQL scripts
for creating SMSD database.

admin directory
---------------

Administrative scripts for updating locales, making release etc.

cmake directory
---------------

CMake include files and templates for generated files.

include directory
-----------------

Public headers for libGammu.

locale directory
----------------

Gettext po files for translating Gammu, libGammu and user documentation.

tests directory
---------------

CTest based testsuite for libGammu.

utils directory
---------------

Various utilities usable with Gammu.

contrib directory
-----------------

Third party contributions based on Gammu.


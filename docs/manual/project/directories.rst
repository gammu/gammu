Directory structure
===================

libgammu directory
------------------

This directory contains sources of Gammu library. You can find all phone
communication and data encoding functionality here.

There are following subdirectories:

``device``
  drivers for devices such serial ports or irda
``device/serial``
  drivers for serial ports
``device/irda``
  drivers for infrared over sockets
``protocol``
  protocol drivers
``protocol/nokia``
  Nokia specific protocols
``phone``
  phone modules
``phone/nokia``
  modules for different Nokia phones
``misc``
  different services. They can be used for any project
``service``
  different gsm services for logos, ringtones, etc.

gammu directory
---------------

Sources of Gammu command line utility. It contains interface to libGammu
and some additional functionality as well.

smsd directory
--------------

Sources of SMS Daemon as well as all it's service backends.

The ``services`` subdirectory contains source code for :ref:`smsd_services`.

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

``config``
    configuration file samples
``examples``
    examples using libGammu
``manual``
    sources of The Gammu Manual which you are reading
``sql``
    SQL scripts to create table structures for :ref:`smsd`
``user``
    user documentation like man pages

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
See :doc:`localization` for more information.

tests directory
---------------

CTest based test suite for libGammu.
See :doc:`testing` for more information.

utils directory
---------------

Various utilities usable with Gammu.

contrib directory
-----------------

This directory contains various things which might be useful with Gammu.
Most of them were contributed by Gammu users.

.. note::

    Please note that that code here might have different license terms than
    Gammu itself.

.. warning::

    Most of scripts provided here are not actively maintained and might
    be broken.

bash-completion
***************

Completion script for bash.

conversion
**********

Various scripts for converting data.

init
****

Init scripts for Gammu SMSD.

media
*****

Sample media files which can be used with Gammu.

perl
****

Various perl scripts which interface to Gammu or SMSD.

php
***

Various PHP frontends to SMSD or Gammu directly.

sms
***

This directory contains SMS default alphabet saved in Unicode text file
(:file:`charset.txt`) and table used for converting chars during saving
SMS with default alphabet (:file:`convert.txt`).

sms-gammu2android
*****************

Perl script to convert :ref:`gammu-smsbackup` into XML suitable for
Android SMS Backup & Restore application.

.. seealso:: http://blog.ginkel.com/2009/12/transferring-sms-from-nokia-to-android/

smscgi
******

Simple cgi application gor handling SMS messages (a bit lighter version
of SMSD).

sql
***

Various SQL snippets and triggers useful with SMSD.

testing
*******

Helper scripts for automatic testing or git bisect.

sqlreply
********

System for automatic replying to SMS messages.

symbian
*******

GNapplet sources and binaries. This comes from Gnokii project, but Gammu
includes slightly modified version.

s60
***

Series60 applet to use with recent Symbian phones.

.. seealso:: :ref:`s60`

win32
*****

Unsupported applications built on top of libGammu.dll on Windows.

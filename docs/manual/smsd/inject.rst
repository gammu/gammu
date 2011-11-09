.. _gammu-smsd-inject:

gammu-smsd-inject
=================

.. program:: gammu-smsd-inject

Synopsis
--------

.. code-block:: text

    gammu-smsd-inject [OPTION]... MESSAGETYPE RECIPIENT [MESSAGE_PARAMETER]...

Description
-----------

This manual page documents briefly the :program:`gammu-smsd-inject` command.

:program:`gammu-smsd-inject` is a program that enqueues message in Gammu SMS
Daemon, which will be later sent by the daemon using connected GSM modem.

Support for this program depends on features available in currently used SMSD
service backend, however currently it is supported by all of them.

Program accepts following options (please note that long options might be not
accepted on some platforms):

.. option:: -h, --help

    Shows help.

.. option:: -v, --version

    Shows version information and compiled in features.

.. option:: -c, --config=file

    Configuration file to use, default is /etc/gammu-smsdrc, on Windows there
    is no default and configuration file path has to be always specified.

.. option:: -l, --use-log

    Use logging as configured in config file.

.. option:: -L, --no-use-log

    Do not use logging as configured in config file (default).

For description of message types and their parameters, please check documentation
for :option:`gammu savesms`.

Examples
--------

To check it out, you need to have configuration file for SMSD, see
:ref:`gammu-smsdrc` for more details about it.

Inject text message up to standard 160 chars:

.. code-block:: sh

    echo "All your base are belong to us" | gammu-smsd-inject TEXT 123456

or

.. code-block:: sh

    gammu-smsd-inject TEXT 123456 -text "All your base are belong to us"

Inject long text message:

.. code-block:: sh

    echo "All your base are belong to us" | gammu-smsd-inject TEXT 123456 -len 400

or

.. code-block:: sh

    gammu-smsd-inject TEXT 123456 -len 400 -text "All your base are belong to us"

or

.. code-block:: sh

    gammu-smsd-inject EMS 123456 -text "All your base are belong to us"

Inject some funky message with predefined sound and animation from 2 bitmaps:

.. code-block:: sh

    gammu-smsd-inject EMS 123456 -text "Greetings" -defsound 1 -text "from Gammu" -tone10 axelf.txt -animation 2 file1.bmp file2.bmp

Inject protected message with ringtone:

.. code-block:: sh

    gammu-smsd-inject EMS 123456 -protected 2 -variablebitmaplong ala.bmp -toneSElong axelf.txt -toneSE ring.txt

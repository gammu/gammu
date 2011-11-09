.. _gammu-smsd-monitor:

gammu-smsd-monitor
==================

.. program:: gammu-smsd-monitor

Synopsis
--------

.. code-block:: text

    gammu-smsd-monitor [OPTION]...

Description
-----------

This manual page documents briefly the :program:`gammu-smsd-monitor` command.

:program:`gammu-smsd-monitor` is a program that monitors state of Gammu SMS
Daemon. It periodically displays information about phone and number of
processed messages.

Program accepts following options (please note that long options might be not
accepted on some platforms):

.. option:: -h, --help

    Shows help.

.. option:: -v, --version

    Shows version information and compiled in features.

.. option:: -c, --config=file

    Configuration file to use, default is /etc/gammu-smsdrc, on Windows there
    is no default and configuration file path has to be always specified.

.. option:: -n, --loops=count

    Number of loops, by default monitor loops infinitely.

.. option:: -d, --delay=seconds

    Delay betwen polling SMSD state, default is 20 seconds.

.. option:: -C, --csv

    Print output in comma separated values format:

    .. code-block:: text

        client;phone ID;IMEI;sent;received;failed;battery;signal

.. option:: -l, --use-log

    Use logging as configured in config file.

.. option:: -L, --no-use-log

    Do not use logging as configured in config file (default).


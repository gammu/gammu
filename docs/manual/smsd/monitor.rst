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

    Delay between polling SMSD state, default is 20 seconds.

.. option:: -C, --csv

    Print output in comma separated values format:

    .. code-block:: text

        client;phone ID;IMEI;IMSI;sent;received;failed;battery;signal

    CSV status records are always written to standard output and are not given
    logging prefixes, including when :option:`--use-log` is specified.

.. option:: -l, --use-log

    Write human-readable status updates using the logging configuration from
    the config file.

.. option:: -L, --no-use-log

    Write human-readable status updates to standard output (default). Output is
    flushed after every polling cycle so it can be consumed by a pipeline.

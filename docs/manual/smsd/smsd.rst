.. _gammu-smsd:

gammu-smsd
==========

.. program:: gammu-smsd

Synopsis
--------

.. code-block:: text

    gammu-smsd [OPTION]...


Description
-----------

This manual page documents briefly the :program:`gammu-smsd` command.

:program:`gammu-smsd` is a program that periodically scans GSM modem for
received messages, stores them in defined storage and also sends messages
enqueued in this storage.

The daemon can reload configuration file after sending hangup signal (SIGHUP)
and properly terminates itself on SIGINT and SIGTERM.

Program accepts following options (please note that long options might be not
accepted on some platforms):

.. option:: -h, --help

    Shows help.

.. option:: -v, --version

    Shows version information and compiled in features.

.. option:: -c, --config=file

    Configuration file to use, default is /etc/gammu-smsdrc, on Windows there
    is no default and configuration file path has to be always specified.

    If you run SMSD as a system daemon (or service), it is recommended to use
    absolute path to configuration file as startup directory might be
    different than you expect.

    See :ref:`gammu-smsdrc` for configuration file documentation.

.. option:: -p, --pid=file

    Lock file for storing pid, empty for no locking. Not supported on Windows.

.. option:: -U, --user=user

    Drop daemon privileges to chosed user after starting.

.. option:: -G, --group=group

    Drop daemon privileges to chosen group after starting.

.. option:: -d, --daemon

    Daemonize program on startup. Not supported on Windows.

.. option:: -i, --install-service

    Installs SMSD as a Windows service.

.. option:: -u, --uninstall-service

    Uninstalls SMSD as a Windows service.

.. option:: -s, --start-service

    Starts SMSD Windows service.

.. option:: -k, --stop-service

    Stops SMSD Windows service.

.. option:: -f, --max-failures=count

    Terminate after defined number of failures. Use 0 to not terminate (this is default).

.. option:: -X, --suicide=seconds

    Kills itself after number of seconds.

.. option:: -S, --run-service

    Runs pogram as SMSD Windows service. This should not be used manually, but
    only Windows Service manager should use this command.

.. option:: -n, --service-name=name

    Defines name of a Windows service. Each service requires an unique name,
    so if you want to run several SMSD instances, you have to name each
    service differently. Default is "GammuSMSD".

.. option:: -l, --use-log

    Use logging as configured in config file (default).

.. option:: -L, --no-use-log

    Do not use logging as configured in config file.

.. option:: -e, --install-event-log

    Installs Windows EventLog description to registry.

    .. versionadded:: 1.31.90

.. option:: -E, --uninstall-event-log

    Uninstalls Windows EventLog description to registry.

    .. versionadded:: 1.31.90

Signals
-------

SMSD can be controlled using following POSIX signals (if your platform supports this):

SIGHUP
    Reload configuration and reconnect to phone.
SIGINT, SIGTERM
    Gracefully shutdown the daemon.
SIGALRM
    Used internally for :option:`gammu-smsd -X`
SIGUSR1
    Suspends SMSD operartion, closing connection to phone and database.
SIGUSR2
    Resumes SMSD operattion (after previous suspend).

.. versionchanged:: 1.22.91
    Added support for SIGHUP.
.. versionchanged:: 1.22.95
    Added support for SIGALRM.
.. versionchanged:: 1.31.90
    Added support for SIGUSR1 and SIGUSR2.

Examples
--------

Linux/Unix Examples
+++++++++++++++++++

Start SMSD as a daemon on Linux:

.. code-block:: sh

    gammu-smsd --config /etc/gammu-smsdrc --pid /var/run/gammu-smsd.pid --daemon

Start SMSD as a daemon on Linux with reduced privileges:

.. code-block:: sh

    gammu-smsd --config /etc/gammu-smsdrc --pid /var/run/gammu-smsd.pid --daemon --user gammu --group gammu

SMSD as a system wide daemon
++++++++++++++++++++++++++++

To use SMSD as a daemon, you might want to use init script which is shipped
with Gammu in contrib/init directory. It is not installed by default, either
install it manually or check INSTALL file for instructions.

Under Windows 7 you might need to disable UAC (user account control) before
you will be able to install SMSD service.

Windows Service Examples
++++++++++++++++++++++++

Install Gammu SMSD Windows service:

.. code-block:: sh

    gammu-smsd.exe -c c:\Gammu\smsdrc -i

Install two instances of SMSD Windows service:

.. code-block:: sh

    gammu-smsd.exe -c c:\Gammu\smsdrc-1 -n Gammu-first-phone -i

    gammu-smsd.exe -c c:\Gammu\smsdrc-2 -n Gammu-second-phone -i

To uninstall a Windows service:

.. code-block:: sh

    gammu-smsd.exe -u

Troubleshooting Windows Service
+++++++++++++++++++++++++++++++

If Gammu fails to start as a Windows service (you will usually get "Error
1053: The service did not respond to the start or control request in a timely
fashion"), first check your SMSD logs. If they do not contain any useful hint,
try starting SMSD manually with exactly same parameters as you installed the
service (without -i).

For example the command line can look like:

.. code-block:: sh

    gammu-smsd.exe -c smsdrc

You now should be able to get errors from SMSD even if it fails to start as a
service.

Known Limitations
-----------------

You can not use same phone by more programs in same time. However in case you
did not enable locking in :config:section:`[gammu]` section, it might be able
to start the communication with phone from more programs. In this case neither
of the programs will probably work.

There is no way to detect that SMS message is reply to another by looking at
message headers. The only way to achieve this is to add some token to the
message and let the user include it in the message on reply.

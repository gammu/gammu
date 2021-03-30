.. _quick:

Quick starter guide
===================

Gammu family
------------

Gammu family consists of several programs and libraries:

:ref:`gammu`
    Command line utility to talk to the phone. It performs one time operations
    only.
`Wammu <https://wammu.eu/wammu>`_
    Graphical interface for Gammu, providing basic functions.
:ref:`gammu-smsd`
    Daemon to receive and send messages using your phone.
:ref:`gammu-smsd-inject`
    Injects outgoing messages into :ref:`gammu-smsd` queue.
:ref:`gammu-detect`
    Simple utility to detect phones or modems connected to computer.
:ref:`python`
    Python bindings for Gammu, use it from Python scripts.
:ref:`libgammu`
    Core library, used by all other parts and you can use it directly in your C
    programs.

Installing Gammu
----------------

On most platforms you can install Gammu from binaries - most Linux
distributions ship Gammu and for Windows you can download binaries from
`Gammu website <https://wammu.eu/download/>`_. You can find more detailed
instructions (including instructions for compiling from source) in
:ref:`installing`.

Starting with Gammu on Linux
----------------------------

First you need to find out device name where your phone/modem is connected. In
most cases you can rely on :ref:`gammu-detect` to find it (it will also list
all serial ports in your systems, where probably nothing is connected).

Generally for most current modems you will end up with ``/dev/ttyUSB0``.

The next step is to create configuration file in :file:`~/.gammurc` (see
:ref:`gammurc`):

.. code-block:: ini

    [gammu]
    device = /dev/ttyUSB0
    connection = at

And you can connect to the phone:

.. code-block:: console

    $ gammu identify
    Device               : /dev/ttyUSB0
    Manufacturer         : Wavecom
    Model                : MULTIBAND  900E  1800 (MULTIBAND  900E  1800)
    Firmware             : 641b09gg.Q2403A 1320676 061804 14:38
    IMEI                 : 123456789012345
    SIM IMSI             : 987654321098765

Starting with Gammu on Windows
------------------------------

First you need to find out device name where your phone/modem is connected. The
easiest way is to look into :guilabel:`Device manager` under
:guilabel:`Ports (COM & LPT)` and lookup correct COM port there.

Generally for most current modems you will end up with something like
``COM12``.

The next step is to create configuration file in
:file:`$PROFILE\\Application Data\\gammurc` (see :ref:`gammurc`):

.. code-block:: ini

    [gammu]
    device = COM12:
    connection = at

And you can connect to the phone:

.. code-block:: console

    C:\Program Files\Gammu 1.33.0\bin> gammu identify
    Device               : COM12:
    Manufacturer         : Wavecom
    Model                : MULTIBAND  900E  1800 (MULTIBAND  900E  1800)
    Firmware             : 641b09gg.Q2403A 1320676 061804 14:38
    IMEI                 : 123456789012345
    SIM IMSI             : 987654321098765

Starting with SMSD
------------------

.. note::

    Before starting with SMSD, make sure you can connect to your phone using
    Gammu (see chapters above for guide how to do that).

Once you have configured Gammu, running :ref:`gammu-smsd` is pretty easy. You
need to decide where you want to store messages (see :config:option:`Service`).
For this example we will stick with MySQL database, but the instructions are
quite similar for any storage service.

Configuring the storage
+++++++++++++++++++++++

First we have to setup the actual storage. With MySQL, we need access to the
MySQL server. Now connect as administrative user to the server (usually
``root``), grant privileges to the ``smsd`` user and create ``smsd`` database:

.. code-block:: mysql

    GRANT USAGE ON *.* TO 'smsd'@'localhost' IDENTIFIED BY 'password';

    GRANT SELECT, INSERT, UPDATE, DELETE ON `smsd`.* TO 'smsd'@'localhost';

    CREATE DATABASE smsd;

Once this is ready, you should import the tables structure. It is shipped as
:file:`docs/sql/mysql.sql` with Gammu, so all you have to do is to import this
file (see :ref:`mysql-create` for more details):

.. code-block:: console

    $ mysql -u root -p password smsd < docs/sql/mysql.sql

Configuring SMSD
++++++++++++++++

Now we just have to tell SMSD what service it is supposed to use. This is done
in the SMSD configuration file. You can place it anywhere and tell SMSD on
startup where it can find it, but on Linux the recommended location for system
wide service is :file:`/etc/gammu-smsdrc` (see :ref:`gammu-smsdrc` for more
information).

You have to put both modem and storage service configuration into this file:

.. code-block:: ini

    [gammu]
    device = /dev/ttyUSB0
    connection = at

    [smsd]
    service = SQL
    driver = native_mysql
    host = localhost
    database = smsd
    user = smsd
    password = password

There are many ways to customize SMSD, but the defaults should work fine in
most environments. You can find more information on customizing SMSD in
:ref:`gammu-smsdrc`.

Running SMSD
++++++++++++

With configuration file ready, you can actually start SMSD. You can do this
manually or as a system wide service.

For manual startup, just execute it:

.. code-block:: console

    $ gammu-smsd

Alternatively you can specify path to the configuration file:

.. code-block:: console

    $ gammu-smsd -c /path/to/gammu-smsdrc

The binary packages on Linux usually come with support for starting SMSD as a
system wide daemon.

With systemd, you can start it by:

.. code-block:: console

    $ systemctl start gammu-smsd.service

Sending message through SMSD
++++++++++++++++++++++++++++

Once SMSD is up and running, you can send some messages using it:

.. code-block:: console

    $ gammu-smsd-inject TEXT 123456 -text "All your base are belong to us"

You can find more examples in the :ref:`gammu-smsd-inject` documentation:
:ref:`smsd-inject-examples`.

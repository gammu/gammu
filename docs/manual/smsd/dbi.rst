.. _gammu-smsd-dbi:

DBI Backend
===========

DBI backend stores all data in any database supported by `libdbi`_, which
parameters are defined by configuration (see :ref:`gammu-smsdrc` for description of
configuration options).

For tables description see :ref:`gammu-smsd-tables`.

This backend is based on :ref:`gammu-smsd-sql`.

.. note::

    The DBI driver is currently not supported on Windows because libdbi
    library does not support this platform.


Supported drivers
-----------------

For complete list of drivers for `libdbi`_ see `libdbi-drivers`_ project. The
drivers for example include:

* ``sqlite3`` - for SQLite 3
* ``mysql`` - for MySQL
* ``pgsql`` - for PostgeSQL
* ``freetds`` - for MS SQL Server or Sybase

.. _libdbi: http://libdbi.sourceforge.net/
.. _libdbi-drivers: http://libdbi-drivers.sourceforge.net/

Example
-------

SQL script for creating tables in SQLite database:

.. literalinclude:: ../../sql/sqlite.sql
   :language: sql

.. note::

    You can find the script in :file:`docs/sql/sqlite.sql` as well. There are
    also scripts for other databases in same folder.

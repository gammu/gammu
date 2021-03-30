.. _gammu-smsd-odbc:

ODBC Backend
============

Description
-----------

.. versionadded:: 1.29.92

ODBC backend stores all data in any database supported by `ODBC`_, which
parameters are defined by configuration (see :ref:`gammu-smsdrc` for description of
configuration options).

For tables description see :ref:`gammu-smsd-tables`.

This backend is based on :ref:`gammu-smsd-sql`.

Supported drivers
-----------------

On Microsoft Windows, Gammu uses native ODBC, on other platforms, `unixODBC`_
can be used.

.. _ODBC: https://en.wikipedia.org/wiki/Open_Database_Connectivity
.. _unixODBC: http://www.unixodbc.org/

Limitations
-----------

Due to limits of the ODBC interface, Gammu can not reliably detect which SQL
engine it is connected to.

In most cases this can be solved by setting :config:option:`SQL` setting to
correct dialect.

If that fails, you can also tweak the SQL queries to work in used SQL server,
see :ref:`SQL Queries` for more details. Still you should set
:config:option:`SQL` to closest matching SQL dialect.

Configuration
-------------

Before running :ref:`gammu-smsd` you need to create necessary tables in the
database. You can use examples given in database specific backends parts of
this manual to do that.

You specify data source name (DSN) as :config:option:`Host` in
:ref:`gammu-smsdrc`. The data source is configured depending on your platform.

.. note::

    Please remember that SMSD might be running in different context than your
    user (separate account on Linux or as as service on Windows), so the ODBC
    DSN needs to be configured as system wide in this case (system DSN on
    Windows or in global configuration on Linux).

On Microsoft Windows, you can find instructions on Microsoft website:
https://support.microsoft.com/kb/305599

For unixODBC this is documented in the user manual:
http://www.unixodbc.org/doc/UserManual/

Creating tables
---------------

Prior to starting SMSD you have to create tables it will use. Gammu ships SQL
scripts for several databases to do that:

* :ref:`mysql-create`
* :ref:`pgsql-create`
* :ref:`sqlite-create`


Example
-------

Example configuration:

.. code-block:: ini

    [smsd]
    service = sql
    driver = odbc
    host = dsn_of_your_database
    sql = sql_variant_to_use
    user = username
    password = password

.. seealso:: :ref:`gammu-smsdrc`

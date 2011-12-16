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

.. _ODBC: http://en.wikipedia.org/wiki/Open_Database_Connectivity
.. _unixODBC: http://www.unixodbc.org/

Limitations
-----------

Due to limits of the ODBC interface, you might have to tweak SQL queries to
work in used SQL server, see :ref:`SQL Queries` for more details.

Partially this can be configured using :config:option:`SQL`.

Configuration
-------------

Before running :ref:`gammu-smsd` you need to create necessary tables in the
database. You can use examples given in database specific backends parts of
this manual to do that.

You specify data source name (DSN) as :config:option:`Host` in
:ref:`gammu-smsdrc`. The data source is configured depending on your platform.

On Microsoft Windows, you can find instructions on Microsoft website:
http://support.microsoft.com/kb/305599

For unixODBC this is documented in the user manual:
http://www.unixodbc.org/doc/UserManual/

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

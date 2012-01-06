.. _gammu-smsd-pgsql:

PostgreSQL Backend
==================

Description
-----------

PGSQL backend stores all data in a `PostgreSQL`_ database server, which
parameters are defined by configuration (see :ref:`gammu-smsdrc` for description of
configuration options).

For tables description see :ref:`gammu-smsd-tables`.

This backend is based on :ref:`gammu-smsd-sql`.

.. _PostgreSQL: http://www.postgresql.org/

Configuration
-------------

Before running :ref:`gammu-smsd` you need to create necessary tables in the
database, which is described below.

The configuration file then can look like:

.. code-block:: ini

    [smsd]
    service = sql
    driver = native_pgsql
    host = localhost

.. seealso:: :ref:`gammu-smsdrc`

Creating tables
---------------

SQL script for creating tables in PostgreSQL database:

.. literalinclude:: ../../sql/pgsql.sql
   :language: sql

.. note::

    You can find the script in :file:`docs/sql/pgsql.sql` as well.

Upgrading tables
----------------

The easiest way to upgrade database structure is to backup old one and start
with creating new one based on example above.

For upgrading existing database, you can use changes described in
:ref:`smsd-tables-history` and then manually update ``Version`` field in
``gammu`` table.

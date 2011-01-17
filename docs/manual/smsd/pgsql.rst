.. _gammu-smsd-pgsql:

PostgreSQL Backend
==================

PGSQL backend stores all data in a `PostgreSQL`_ database server, which
parameters are defined by configuration (see :ref:`gammu-smsdrc` for description of
configuration options).

For tables description see :ref:`gammu-smsd-tables`.

This backend is based on :ref:`gammu-smsd-sql`.

.. _PostgreSQL: http://www.postgresql.org/

Example
-------

SQL script for creating tables in PostgreSQL database:

.. literalinclude:: ../../sql/pgsql.sql
   :language: sql

.. note::

    You can find the script in :file:`docs/sql/pgsql.sql` as well.

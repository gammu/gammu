.. _gammu-smsd-mysql:

MySQL Backend
=============

Description
-----------

MYSQL backend stores all data in a `MySQL`_ database server, which parameters are
defined by configuration (see :ref:`gammu-smsdrc` for description of configuration
options).

For tables description see :ref:`gammu-smsd-tables`.

This backend is based on :ref:`gammu-smsd-sql`.

Configuration
-------------

Before running :ref:`gammu-smsd` you need to create necessary tables in the
database, which is described bellow.

The configuration file then can look like:

.. code-block:: ini

    [smsd]
    service = sql
    driver = native_mysql
    host = localhost

.. seealso:: :ref:`gammu-smsdrc`

Privileges
----------

The user accessing the database does not need much privileges, the following
privleges should be enough:

.. code-block:: sql

    GRANT USAGE ON *.* TO 'smsd'@'localhost' IDENTIFIED BY 'password';

    GRANT SELECT, INSERT, UPDATE, DELETE ON `smsd`.* TO 'smsd'@'localhost';

.. note::

   For creating the SQL tables you need more privileges, especially for
   creating triggers, which are used for some functionality.

.. _MySQL: http://www.mysql.com/

Creating tables
---------------

SQL script for creating tables in MySQL database:

.. literalinclude:: ../../sql/mysql.sql
   :language: mysql

.. note::

    You can find the script in :file:`docs/sql/mysql.sql` as well.

Upgrading tables
----------------

The easiest way to upgrade database structure is to backup old one and start
with creating new one based on example above.

For upgrading existing database, you can use changes described in
:ref:`smsd-tables-history` and then manually update ``Version`` field in
``gammu`` table.

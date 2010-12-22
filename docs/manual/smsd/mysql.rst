.. _gammu-smsd-mysql:

MySQL Backend
=============

MYSQL backend stores all data in a MySQL database server, which parameters are
defined by configuration (see :ref:`gammu-smsdrc` for description of configuration
options).

For tables description see :ref:`gammu-smsd-tables`.

This backend is based on :ref:`gammu-smsd-sql`.

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

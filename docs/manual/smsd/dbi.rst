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

.. _libdbi: http://libdbi.sourceforge.net/

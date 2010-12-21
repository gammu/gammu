.. _dummy-driver:

Dummy Driver
============

.. versionadded:: 1.22.93

The ``dummy`` driver in Gammu emulates all operations on filesystem. It is
used by :doc:`testsuite`, but it is also very helpful for application
developers, because they can test the functionality without using real phone
and avoiding risk of corrupting data in the phone.

Filesystem structure
--------------------

The dummy driver emulates all phone functionality on filesystem. The
:config:option:`Device` configuration directive sets top level directory,
where all data are stored.

This directory contains file :file:`operations.log`, where are logged
operations which do not modify any data in the dummy phone (eg. sending
message).

Messages
++++++++

Messages are stored in :file:`sms/<FOLDER>` directories (``<FOLDER>`` is in
range 1-5) in Gammu native smsbackup format.

Phonebook
+++++++++

Phonebook (and calls registers) are stored in :file:`pbk/<MEMORY>` (``<MEMORY>``
is type of memory like ``ME`` or ``SM``) directories in vCard format.

Notes
+++++

Notes are stored in :file:`note` directory in vNote format.

Calendar
++++++++

Calendar entries are stored in :file:`calendar` directory in vCalendar format.

Todo
++++

Todo entries are stored in :file:`todo` directory in vCalendar format.

Filesystem
++++++++++

Filesystem is stored in :file:`fs` directory. You can create another
subdirectories there.

Other features
--------------

By specifying :config:option:`Features` you can configure some specific behavior:

``DISABLE_GETNEXT``
    Makes the dummy driver fail all GetNext* calls as not supported (with
    exception of GetNextSMS* and GetNextFile*).
``DISABLE_GETNEXTSMS``
    Makes the dummy driver fail all GetNextSMS* calls as not supported.

Examples
--------

To use dummy driver, you need something like following in :file:`~/.gammurc`:

.. code-block:: ini

    [gammu]
    model = dummy
    connection = none
    device = /path/to/directory/

For disabling GetNext* functions within dummy driver, you need something like following in :file:`~/.gammurc`:

.. code-block:: ini

    [gammu]
    model = dummy
    connection = none
    features = DISABLE_GETNEXT
    device = /path/to/directory/

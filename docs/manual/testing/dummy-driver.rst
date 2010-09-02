Dummy Driver
============

Recently there has been added dummy driver to Gammu, which emulates all
operations on filesystem. It is used by :doc:`testsuite`, but it is also very
helpful for application developers, because they can test the functionality
without using real phone and avoiding risk of corrupting data in the phone.
To use dummy driver, you need something like following in :file:`~/.gammurc`::

    [gammu]
    model = dummy
    connection = none
    port = /path/to/direcotry/

Filesystem structure
--------------------

The dummy driver emulates all phone functionality on filesystem. The ``port``
configuration directive sets top level directory, where all data are stored.

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

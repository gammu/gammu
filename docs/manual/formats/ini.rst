.. _ini:

INI file format
===============

The INI file format is widely used in Gammu, for both configuration (see
:ref:`gammurc`) and storing data (see :ref:`gammu-backup` and :ref:`gammu-smsbackup`).

This file use ini file syntax, with comment parts being marked with both ``;``
and ``#``. Sections of config file are identified in square brackets line
``[this]``. All key values are case insensitive.

Examples
--------

You most likely know INI files from other programs, however to illustrate,
here is some example:

.. code-block:: ini

    ; comment

    [section]
    key = value

    [another section]

    key =  longer value


    # another comment

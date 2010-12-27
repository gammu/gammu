
.. _gammu-detect:

gammu-detect
============

.. program:: gammu-detect

Synopsis
--------

.. code-block:: text

    gammu-detect [-d|--debug]

Description
-----------

Script to detect available devices, which might be suitable for :ref:`gammu`.

Currently it supports following devices:

* USB devices using udev
* Serial ports using udev

This program follows the usual GNU command line syntax, with long options
starting with two dashes (``-``). A summary of options is included below.

.. option:: -h, --help

    Show summary of options.

.. option:: -d, --debug

    Show debugging output for detecting devices.

.. option:: -u, --no-udev

    Disables scanning of udev.

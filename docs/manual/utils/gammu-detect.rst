
.. _gammu-detect:

gammu-detect
============

.. program:: gammu-detect

Synopsis
--------

.. code-block:: text

    gammu-detect [OPTIONS]

Description
-----------

Script to detect available devices, which might be suitable for :ref:`gammu`.

.. note::

    This program lists all devices, which might be suitable, it does not do
    any probing on device itself.

Currently it supports following devices:

* USB devices using udev
* Serial ports using udev
* Bluetooth devices using Bluez

This program follows the usual GNU command line syntax, with long options
starting with two dashes (``-``). A summary of options is included below.

.. option:: -h, --help

    Show summary of options.

.. option:: -d, --debug

    Show debugging output for detecting devices.

.. option:: -u, --no-udev

    Disables scanning of udev.

.. option:: -b, --no-bluez

    Disables scanning using Bluez.

Output
------

The output of :ref:`gammu-detect` is configuration file for Gammu (see
:ref:`gammurc`) with configuration section for every device which might be
used with :ref:`gammu`. 

.. note::
   
    You can choose which section to use by :option:`gammu -s`.

When invoked as :option:`gammu-detect -d`, also all examined devices are
listed as comments in the output.

Example
-------

.. code-block:: ini

    ; gammu-detect output
    [gammu]
    device = /dev/ttyACM0
    name = Nokia E52
    connection = at

    [gammu1]
    device = /dev/ttyACM1
    name = Nokia E52
    connection = at

    [gammu2]
    device = /dev/ttyS0
    name = Phone on serial port 0
    connection = at

    [gammu3]
    device = /dev/ttyS1
    name = Phone on serial port 1
    connection = at

    [gammu4]
    device = /dev/ttyS2
    name = Phone on serial port 2
    connection = at

    [gammu5]
    device = /dev/ttyS3
    name = Phone on serial port 3
    connection = at

    [gammu6]
    device = 5C:57:C8:BB:BB:BB
    name = Nokia E52
    connection = bluephonet


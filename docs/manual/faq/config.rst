.. _faq-config:

Configuring Gammu FAQ
=====================

.. _faq-config-at:

How to configure 3G/UMTS/... modem or AT capable phone?
-------------------------------------------------------

As most modems support AT commands, this is pretty easy and you should use
``at`` :config:option:`Connection`. For :config:option:`Device` you should use
device name as modem appears in your system, for example ``/dev/ttyACM0`` or
``COM7:``.

Some modems expose more serial ports and you need to carefully choose the right
one - for example only one of them can receive USSD notifications.

.. note::

    On Linux, you might have to install `usb-modeswitch`_ to make your modem
    acutally behave like a modem and not like a disk containing drivers for
    Windows.

.. seealso:: :ref:`faq-devname`, :ref:`gammurc`

.. _usb-modeswitch: http://www.draisberghof.de/usb_modeswitch/

Example configuration on Linux:

.. code-block:: ini

    [gammu]
    device = /dev/ttyACM3
    connection = at

Example configuration on Windows:

.. code-block:: ini

    [gammu]
    device = COM12:
    connection = at

.. _faq-config-symbian:

How to configure Symbian based phone?
-------------------------------------

The only support for Symbian phones is using applet installed to phone and
Bluetooth connection. You should use ``blues60`` :config:option:`Connection`
and Bluetooth address of phone as :config:option:`Device`. On older Symbian 
phones you will have to use gnapplet and ``bluerfgnapbus`` connection.

.. seealso:: :ref:`s60`, :ref:`gammurc`

.. note::

    Do not forget to start the applet before trying to connect to the phone.

Example configuration:

.. code-block:: ini

    [gammu]
    device = 11:22:33:44:55:66 # Bluetooth address of your phone
    connection = blues60

.. _faq-config-nokia:

How to configure Nokia phone?
-----------------------------

If you have Series 40 (S40) phone, it should work using either Bluetooth or USB
cable.

For Bluetooth connection, ``bluephonet`` :config:option:`Connection` is always
the right choice with Bluetooth address of phone as :config:option:`Device`.

For USB cable choosing the right connection type is more tricky and depends on
generation of your phone. Newest phones usually work with ``dku2`` and the
older ones with ``dlr3`` as :config:option:`Connection`.

Should you have old phone with serial cable (and USB to serial converter),
``fbus`` :config:option:`Connection` is the right one.

.. seealso:: :ref:`gammurc`

Example configuration for Bluetooth:

.. code-block:: ini

    [gammu]
    device = 11:22:33:44:55:66 # Bluetooth address of your phone
    connection = bluephonet

Example configuration for newer phones:

.. code-block:: ini

    [gammu]
    connection = dku2

Example configuration for older phones on Linux:

.. code-block:: ini

    [gammu]
    device = /dev/ttyACM3
    connection = dlr3

Example configuration for older phones on Windows:

.. code-block:: ini

    [gammu]
    device = COM12:
    connection = dlr3

How to configure phone not listed above?
----------------------------------------

First check whether your phone is supported. In case it is, it most likely
falls into one of above categories.

You can also find additional user experiences in `Gammu Phones Database`_.

.. seealso:: :ref:`faq-phones`, :ref:`gammurc`

.. _Gammu Phones Database: http://wammu.eu/phones/

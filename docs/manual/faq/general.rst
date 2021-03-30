General Gammu FAQ
=================

Will Gammu work on my system?
-----------------------------

Gammu is known to run on wide range of systems. It can be compiled natively on
Linux, Mac OS X, FreeBSD, OpenBSD and Microsoft Windows. It can be probably
compiled also elsewhere, but nobody has yet tried. On some platforms however
you might lack support for some specific kind of devices (eg. Bluetooth or
USB).

.. seealso:: :ref:`installing`

How to set sender number in message?
------------------------------------

You can quite often see messages sent from textual address or with some other
nice looking sender number. However this needs to be done in the GSM network
and it is not possible to influence this from the terminal device (phone).
Usually it is set by SMSC and some network providers allow you to set this
based on some contract. Alternatively you can use their SMS gateways, which
also allow this functionality.

.. seealso:: :ref:`gammu-sms`

Can I use Gammu to send MMS?
----------------------------

MMS contains of two parts - the actual MMS data in SMIL format and the SMS
containing notification about the data. Gammu can create the notification SMS,
where you just need to put URL of the data (use :option:`gammu sendsms`
MMSINDICATOR for that). However you need to encode MMS data yourself or use
other program to do that.

.. _faq-mms-download:

Can I use Gammu to receive MMS?
-------------------------------

MMS contains of two parts - the actual MMS data in SMIL format and the SMS
containing notification about the data. Gammu (or SMSD) will receive the
notification SMS, where URL to download the MMS content is included.

However in most situations the URL is accessible only from the network and APN
specific for the MMS messages, so downloading it is a bit tricky and needs to
connect using GSM modem to the network using this APN.

.. _faq-devname:

Device name always changes on Linux, how to solve that?
-------------------------------------------------------

You can use udev to assign persistent device name (used as
:config:option:`Device`). You can either use standard persistent names based on
serial number (located in :file:`/dev/serial/by-id/`) or define own rules::

    ACTION=="add", SUBSYSTEMS=="usb", ATTRS{manufacturer}=="Nokia", KERNEL=="ttyUSB*", SYMLINK+="phone"

Better is to use vendor and product IDs (you can get them for example using :command:`lsusb`)::

    ACTION=="add", SUBSYSTEMS=="usb", ATTRS{idVendor}=="xxxx", ATTRS{idProduct}=="yyyy", SYMLINK+="phone"

If you're using 3G modem, it's quite likely that it exposes multiple interfaces
and only one of them is good for Gammu usage. In this case you should match
against interface number as well::

    ACTION=="add", SUBSYSTEMS=="usb", ATTRS{idVendor}=="xxxx", ATTRS{idProduct}=="yyyy", ATTRS{bInterfaceNumber}=="00", SYMLINK+="phone"

You can match by various attributes, you can figure them using udevadm command:

.. code-block:: sh

    udevadm info --name=/dev/ttyUSB1 --attribute-walk

.. seealso::

    Various documentation on creating persistent device names using udev is
    available online, for example on the `Debian wiki`_ or in
    `Writing udev rules`_ document.

.. _echo:

Mutliple programs using same device cause various errors, how to fix that?
--------------------------------------------------------------------------

Gammu needs to be the only program using the device, otherwise you will get
strange errors from both programs as they will read answer to command sent by
something else.

In gammu, it can happen quite early with error message "Phone does not support
enabled echo, it can not work with Gammu!", but it can be spotted later as
well, depending on various conditions.

In case you see such behavior, check what other programs are using given device.
This can be done using :program:`fuser` tool:

.. code-block:: sh

    fuser -va /dev/ttyACM0

The usual programs involve:

* NetworkManager with ModemManager, you need to disable mobile networking to
  stop it using the device, disabling the modem connection does not seem to be
  enough.
* Other Gammu instance, in case you want to interact with modem while SMSD is
  running see :ref:`gammu-smsd-suspend`.


What are free alternatives to Gammu?
------------------------------------

It depends on your phone. For Nokia or AT based phones, you can try `Gnokii`_,
but Gammu should be superior in most cases. For Symbian phone you can try using
`Series60-Remote`_, which works pretty well with S60 phones, though Gammu
brings various fixes to their applet.

If you are looking for synchronisation, try using something what supports
SyncML to retrieve contacts or calendar from your phone, for example
`OpenSync`_ or `syncEvolution`_.

.. _Debian wiki: http://wiki.debian.org/udev#persistent-name
.. _Writing udev rules: http://reactivated.net/writing_udev_rules.html
.. _Series60-Remote: http://series60-remote.sourceforge.net/
.. _OpenSync: http://www.opensync.org/
.. _syncEvolution: http://syncevolution.org/
.. _Gnokii: http://gnokii.org/

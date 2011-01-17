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

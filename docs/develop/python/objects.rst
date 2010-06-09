Objects
=======

For various (mostly historical) reasons, all objects you get from Gammu are
not real objects but rather a dictionaries. This has quite a big impact of
usability and will most likely change in the future.

.. _sms_obj:

SMS Object
----------

Message dictionary can consist of following fields:

.. attribute:: SMSC

   SMSC information, see :ref:`smsc_obj`.

.. attribute:: Number

   Recipient number, needs to be set for sending.

.. attribute:: Name

   Name of the message, does not make any effect on sending, some phones might
   store it.

.. attribute:: UDH

   User defined headers for SMS, see :ref:`udh_obj`.

.. attribute:: Text

   Message text

.. attribute:: Folder

   Folder where the message is stored

.. attribute:: Location

   Location where the message is stored

.. attribute:: InboxFolder

   Indication whether folder is an inbox

.. attribute:: DeliveryStatus

   Message delivery status, used only for received messages

.. attribute:: ReplyViaSameSMSC

   Flag indicating whether reply using same SMSC is requested

.. attribute:: Class

   Message class

.. attribute:: MessageReference

   Message reference number, used mostly to identify delivery reports

.. attribute:: ReplaceMessage

   Id of message which this message is supposed to replace

.. attribute:: RejectDuplicates

   Whether to reject duplicates

.. attribute:: Memory

   Memory where the message is stored

.. attribute:: Type

   Message type, one of:

* ``Submit`` - message to be send
* ``Deliver`` - delivered message
* ``Status_Report`` - when creating new message this will create submit message
  with request for delivery report

.. attribute:: Coding

   Message encoding, one of:

* ``Unicode_No_Compression`` - unicode message which can contain any chars,
  but can be only 70 chars long
* ``Unicode_Compression`` - not supported by Gammu and most phones
* ``Default_No_Compression`` - message with GSM alphabet only, up to 160 chars
  long
* ``Default_Compression`` - not supported by Gammu and most phones
* ``8bit`` - for binary messages

.. attribute:: DateTime

   Timestamp when the message was received or sent.

   Please note that most phones do no record timestamp of sent messages.

.. attribute:: SMSCDateTime

   Timestamp when the message was at SMSC.

.. attribute:: State

   Message state, one of:

* ``Sent``
* ``UnSent``
* ``Read``
* ``UnRead``

Examples::

    # Simple message to send, using SMSC from phone
    SMS_1 = {
        'Number': '123465',
        'SMSC': {'Location': 1},
        'Text': 'Hello world!',
    }

    # Class 0 (on display) message using custom SMSC number
    SMS_2 = {
        'Number': '123465',
        'SMSC': {'Number': '+420987654321'},
        'Text': 'Hello world!',
        'Class': 0,
    }


.. _udh_obj:

UDH Object
----------

UDH dictionary can consist of following fields:

.. attribute:: ID8bit

   8-bit ID of the message, not required

.. attribute:: ID16bit

   16-bit ID of the message, not required

.. attribute:: PartNumber

   Number of current part

.. attribute:: AllParts

   Count of all message parts

.. attribute:: Type

   UDH type, one of predefined strings:

* ``NoUDH``
* ``ConcatenatedMessages``
* ``ConcatenatedMessages16bit``
* ``DisableVoice``
* ``DisableFax``
* ``DisableEmail``
* ``EnableVoice``
* ``EnableFax``
* ``EnableEmail``
* ``VoidSMS``
* ``NokiaRingtone``
* ``NokiaRingtoneLong``
* ``NokiaOperatorLogoLong``
* ``NokiaCallerLogo``
* ``NokiaWAP``
* ``NokiaWAPLong``
* ``NokiaCalendarLong``
* ``NokiaProfileLong``
* ``NokiaPhonebookLong``
* ``UserUDH``

.. attribute:: Text

   UDH content

Example::

    UDH = {
        'ID8bit': 0xcd,
        'PartNumber': 1,
        'AllParts': 2,
        'Type': 'ConcatenatedMessages',
    }

.. _smsc_obj:

SMSC Object
-----------

SMSC dictionary can consist of following fields:

.. attribute:: Location

   Location where the SMSC is stored

.. attribute:: Number

   SMSC number

.. attribute:: Name

   Name of the SMSC configuration

.. attribute:: DefaultNumber

   Default recipient number, ignored on most phones

.. attribute:: Format

   Default message format, one of:

* ``Text``
* ``Pager``
* ``Fax``
* ``Email``

.. attribute:: Validity

   Default message validity as a string

* ``NA`` - validity not available
* ``Max`` - maximal validity allowed by network
* ``nM``, ``nH``, ``nD``, ``nW`` - period defined in minutes, hours, days or
  weeks, eg. ``3W``

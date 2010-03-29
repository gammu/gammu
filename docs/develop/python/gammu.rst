:mod:`gammu` -- Mobile phone access
===================================

.. module:: gammu
    :synopsis: Provides access to mobile phones.

This module wraps all python-gammu functionality. 

:class:`gammu.StateMachine`
---------------------------

.. autoclass:: gammu.StateMachine
   :members: 

Generic functions
-----------------

.. function:: Version()

    Get version information.

    :return: Tuple of version information - Gammu runtime version, python-gammu version, build time Gammu version.
    :rtype: tuple

Debugging configuration
-----------------------

.. function:: SetDebugFile(File)

    Sets global debug file.

    :param File: File where to write debug stuff (as configured by L{SetDebugLevel}). Can be either None for no file, Python file object or filename.
    :type File: mixed
    :return: None
    :rtype: None

.. function:: SetDebugLevel(Level)

    Sets global debug level accorting to passed string. You need to configure output file using L{SetDebugFile} to activate it.

    :type Level: string
    :param Level: name of debug level to use, currently one of:

       * nothing
       * text
       * textall
       * binary
       * errors
       * textdate
       * textalldate
       * errorsdate
    :return: None
    :rtype: None

Message processing
------------------

.. function:: LinkSMS(Messages, EMS)

    Links multi part SMS messages.

    :type Messages: list
    :type EMS: boolean
    :param Messages: List of messages to link
    :param EMS: Whether to detect ems, defauls to True
    :return: List of linked messages
    :rtype: list

.. function:: DecodeSMS(Messages, EMS)

    Decodes multi part SMS message.

    :param Messages: Nessages to decode
    :type Messages: list
    :param EMS: Whether to use EMS, defalt to True
    :type EMS: boolean
    :return: Multi part message information
    :rtype: hash

.. function:: EncodeSMS(MessageInfo)

    Encodes multi part SMS message.

    :param MessageInfo: Description of message
    :type MessageInfo: dict
    :return: Dictionary with raw message
    :rtype: dict

.. function:: DecodePDU(Data, SMSC = False)

    Parses PDU packet.

    :param Data: PDU data, need to be binary not hex encoded
    :type Data: string
    :param SMSC: Whether PDU includes SMSC.
    :type SMSC: boolean
    :return: Message data
    :rtype: dict

.. function:: EncodePDU(SMS, Layout = Submit)

    Creates PDU packet.

    :param SMS: SMS dictionary
    :type SMS: dict
    :param Layout: Layout (one of Submit, Deliver, StatusReport), Submit is default
    :type Layout: string
    :return: Message data
    :rtype: string

    .. versionadded:: 1.27.93

Encoding and decoding entries
-----------------------------

.. function:: DecodeVCARD(Text)

    Decodes memory entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Memory entry
    :rtype: hash

.. function:: EncodeVCARD(Entry)

    Encodes memory entry to a vCard.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vCard
    :rtype: string

.. function:: DecodeVCS(Text)

    Decodes todo/calendar entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Calendar or todo entry (whatever one was included in string)
    :rtype: hash

.. function:: DecodeICS(Text)

    Decodes todo/calendar entry v from a string.

    :param Text: String to decode
    :type Text: string
    :return: Calendar or todo entry (whatever one was included in string)
    :rtype: hash

.. function:: EncodeVCALENDAR(Entry)

    Encodes memory entry to a vCalendar.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vCalendar
    :rtype: string

.. function:: EncodeICALENDAR(Entry)

    Encodes memory entry to a iCalendar.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with iCalendar
    :rtype: string

.. function:: EncodeVTODO(Entry)

    Encodes memory entry to a vTodo.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vTodo
    :rtype: string

.. function:: EncodeITODO(Entry)

    Encodes memory entry to a iTodo.

    :param Entry: Memory entry
    :type Entry: dict
    :return: String with vCard
    :rtype: string

Backup reading and writing
--------------------------

.. function:: SaveRingtone(Filename, Ringtone, Format)

    Saves ringtone into file.

    :param Filename: Name of file where ringote will be saved
    :type Filename: string
    :param Ringtone: Ringtone to save
    :type Ringtone: dict
    :param Format: One of 'ott', 'mid', 'rng', 'imy', 'wav', 'rttl'
    :type Format: string
    :return: None
    :rtype: None

.. function:: SaveBackup(Filename, Backup, Format)

    Saves backup into file.

    :param Filename: Name of file to read backup from
    :type Filename: string
    :param Backup: Backup data, see L{ReadBackup} for description
    :type Backup: dict
    :param Format: File format to use (default is AutoUnicode)
    :type Format: string (Auto, AutoUnicode, LMB, VCalendar, VCard, LDIF, ICS, Gammu, GammuUnicode)
    :return: None
    :rtype: None

.. function:: ReadBackup(Filename, Format)

    Reads backup into file.

    :param Filename: Name of file where backup is stored
    :type Filename: string
    :param Format: File format to use (default is AutoUnicode)
    :type Format: string (Auto, AutoUnicode, LMB, VCalendar, VCard, LDIF, ICS, Gammu, GammuUnicode)
    :return: Dictionary of read entries, it contains following keys, each might be empty:

         * IMEI
         * Model
         * Creator
         * PhonePhonebook
         * SIMPhonebook
         * Calendar
         * ToDo
         * DateTime
    :rtype: dict

.. function:: SaveSMSBackup(Filename, Backup)

    Saves SMS backup into file.

    :param Filename: Name of file where to save SMS backup
    :type Filename: string
    :param Backup: List of messages to store
    :type Backup: list
    :return: None
    :rtype: None

.. function:: ReadSMSBackup(Filename)

    Reads SMS backup into file.

    :param Filename: Name of file where SMS backup is stored
    :type Filename: string
    :return: List of messages read from file
    :rtype: list

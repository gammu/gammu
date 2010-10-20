.. _smsbackup:
.. _SMS Backup Format:

SMS Backup Format
=================

The SMS backup format is text file encoded in current encoding of platform
where Gammu is running.

This file use ini file syntax, see :ref:`ini`.

The file consists of unlimited number of sections, each is for one message and
it's name is formatted like ``SMSBackup001``. The numbering of messages must
be consistent - Gammu stops to read the file on first skipped number.

Example
-------

The backup of message can look like following:

.. code-block:: ini

    [SMSBackup000]
    #ABCDEFGHIJKLMNOPQRSTUVWXYZ
    #
    SMSC = "+4540590000"
    SMSCUnicode = 002B0034003500340030003500390030003000300030
    Sent = 20021201T025023
    State = UnRead
    Number = "+4522706947"
    NumberUnicode = 002B0034003500320032003700300036003900340037
    Name = ""
    NameUnicode =
    Text00 = 004100420043004400450046004700480049004A004B004C004D004E004F0050005100520053005400550056005700580059005A000A
    Coding = Default
    Folder = 1
    Length = 27
    Class = \-1
    ReplySMSC = False
    RejectDuplicates = True
    ReplaceMessage = 0
    MessageReference = 0

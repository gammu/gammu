.. _gammu-smsd-files:

Files backend
=============

Description
-----------

FILES backend stores all data on a filesystem in folders defined by
configuration (see :ref:`gammu-smsdrc` for description of configuration options).

Receiving of messages
---------------------

Received messages are stored in a folder defined by configuration. The
filename will be ``IN<date>_<time>_<serial>_<sender>_<sequence>.<ext>``,
for example ``NN20021130_021531_00_+45409000931640979_00.txt``.

Explanation of fields:

``<date>``
    date in format ``YYYYMMDD``
``<time>``
    time in format ``HHMMSS``
``<sender>``
    sender number
``<serial>``
    order of a message (in case more messages were received at same time), in format ``NN``
``<sequence>``
    part of the message for multipart messages, in format ``NN``
``<ext>``
    ``txt`` for text message, 8-bit messages are stored with ``bin`` extension, ``smsbackup`` for :ref:`gammu-smsbackup`

The content of the file is content of the message and the format is defined by
configuration directive :config:option:`InboxFormat` (see :ref:`gammu-smsdrc`).

Transmitting of messages
------------------------

Transmitted messages are read from a folder defined by configuration. The
filename should be one of the following formats:

- ``OUT<recipient>.<ext>``
- ``OUT<priority>_<recipient>_<serial>.<ext>``
- ``OUT<priority><date>_<time>_<serial>_<recipient>_<note>.<ext>``

Explanation of fields:

``<recipient>``
    recipient number where to send message
``<priority>``
    an alphabetic character (A-Z) A = highest priority
``<ext>``
    ``txt`` for normal text SMS, ``smsbackup`` for :ref:`gammu-smsbackup`
``<note>``
    any artibrary text which is ignored

For text messages, you can additionally append flags to extension:

    ``d``
        delivery report requested
    ``f``
        flash SMS
    ``b``
        WAP bookmark as name,URL

Other fields are same as for received messages.

For example OUTG20040620_193810_123_+4512345678_xpq.txtdf is a flash text SMS
requesting delivery reports.

SMSes will be transmitted sequentially based on the file name. The contents of
the file is the SMS to be transmitted (in Unicode or standard character set).

The contents of the file is the SMS to be transmitted (in Unicode or standard
character set), for WAP bookmarks it is split on as Name,URL, for text
messages whole file content is used.

Please note that if file is not in Unicode, encoding is detected based on
locales, which do not have to be configured if SMSD is running from init
script. If this is your case, please add locales definition to init script.

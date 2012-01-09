.. _gammu-smsd-tables:

SMSD Database Structure
=======================

The backends themselves are described in their sections, this document
describes general database structure and required tables.

More SMS daemons can share single database. If you do not specify PhoneID in
their configuration, all are treated equally and you have no guarantee which
one sends outgoing message. If you configure PhoneID and use it when inserting
message to the ``outbox`` table (:ref:`gammu-smsd-inject` does this), each SMS
daemon will have separate outbox queue.

Receiving of messages
---------------------

Received messages are stored in :ref:`inbox` table.

Transmitting of messages
------------------------

Transmitted messages are read from table :ref:`outbox` and possible subsequent parts
of the same message from :ref:`outbox_multipart`.

Description of tables
---------------------

daemons
+++++++

Information about running daemons.

gammu
+++++

Table holding single field ``Version`` - version of a database schema. See
:ref:`smsd-tables-history` for details what has changed.

.. _inbox:

inbox
+++++

Table where received messages will be stored.

Fields description:

``UpdatedInDB`` (timestamp)
    when somebody (daemon, user, etc.) updated it

``ReceivingDateTime`` (timestamp)
    when SMS was received

``Text`` (text)
    encoded SMS text (for all SMS)

``SenderNumber`` (varchar(20))
    decoded SMS sender number

``Coding`` (enum('Default_No_Compression', 'Unicode_No_Compression', '8bit', 'Default_Compression', 'Unicode_Compression'))
    SMS text coding

``UDH`` (text)
    encoded User Data Header text

``SMSCNumber`` (varchar(20))
    decoded SMSC number

``Class`` (integer)
    SMS class or \-1 (0 is flash SMS, 1 is normal one)

``TextDecoded`` (varchar(160))
    decoded SMS text (for Default Alphabet/Unicode SMS)

``ID`` (integer unsigned)
    SMS identificator (for using with external applications)

``RecipientID`` (text)
    which Gammu daemon has added it

``Processed`` (enum('false', 'true'))
    you can use for marking, whether SMS was processed or not


.. _outbox:

outbox
++++++

Messages enqueued for sending should be placed in this table. If message
is multipart, subsequent parts are stored in table :ref:`outbox_multipart`.

Fields description:

``UpdatedInDB`` (timestamp)
    when somebody (daemon, user, etc.) updated it

``InsertIntoDB`` (timestamp)
    when message was inserted into database

``SendingDateTime`` (timestamp)
    set it to some value, when want to force sending after some planned time

``SendBefore`` (time)
    Send message before specified time, can be used to limit messages from
    being sent in night. Default value is 23:59:59

    .. versionadded:: 1.29.90

``SendAfter`` (time)
    Send message after specified time, can be used to limit messages from
    being sent in night. Default value is 00:00:00

    .. versionadded:: 1.29.90

``Text`` (text)
    SMS text encoded using hex values in proper coding. If you want to use
    TextDecoded field, keep this NULL (or empty).

``DestinationNumber`` (varchar(20))
    recipient number

``Coding`` (enum('Default_No_Compression', 'Unicode_No_Compression', '8bit', 'Default_Compression', 'Unicode_Compression'))
    SMS text coding

``UDH`` (text)
    User Data Header encoded using hex values which will be used for constructing
    the message. Without this, message will be sent as plain text.

``Class`` (integer)
    SMS class or \-1 (0 is normal SMS, 1 is flash one)

``TextDecoded`` (varchar(160))
    SMS text in "human readable" form

``ID`` (integer unsigned)
    SMS/SMS sequence ID

    Please note that this number has to be unique also for sentitems table, so
    reusing message IDs might not be a good idea.

``MultiPart`` (enum('false','true'))
    info, whether there are more SMS from this sequence in outbox_multipart

``RelativeValidity`` (integer)
    SMS relative validity like encoded using GSM specs

``SenderID`` (text)
    which SMSD instance should send this one sequence, see :config:option:`PhoneID`

``SendingTimeOut`` (timestamp)
    used by SMSD instance for own targets

``DeliveryReport`` (enum('default','yes','no'))
    when default is used, Delivery Report is used or not according to SMSD instance settings; yes forces Delivery Report.

``CreatorID`` (text)
    sender identification, it has to match PhoneID in SMSD configuration to make
    SMSD process this message

.. _outbox_multipart:

outbox_multipart
++++++++++++++++

Data for outgoing multipart messages.

Fields description:

``ID`` (integer unsigned)
    the same meaning as values in outbox table
``Text`` (text)
    the same meaning as values in outbox table
``Coding`` (enum('Default_No_Compression', 'Unicode_No_Compression', '8bit', 'Default_Compression', 'Unicode_Compression'))
    the same meaning as values in outbox table
``UDH`` (text)
    the same meaning as values in outbox table
``Class`` (integer)
    the same meaning as values in outbox table
``TextDecoded`` (varchar(160))
    the same meaning as values in outbox table
``ID`` (integer unsigned)
    the same meaning as values in outbox table

``SequencePosition`` (integer)
    info, what is SMS number in SMS sequence (start at 2, first part is in :ref:`outbox`
    table).


phones
++++++

Information about connected phones. This table is periodically refreshed and
you can get information such as battery or signal level from here.

Fields description:

``ID`` (text)
    PhoneID value

``UpdatedInDB`` (timestamp)
    when this record has been updated

``InsertIntoDB`` (timestamp)
    when this record has been created (when phone has been connected)

``TimeOut`` (timestamp)
    when this record expires

``Send`` (boolean)
    indicates whether SMSD is sending messages, depends on configuration directive :config:option:`Send`

``Receive`` (boolean)
    indicates whether SMSD is receiving messages, depends on configuration directive :config:option:`Receive`

``IMEI`` (text)
    IMEI of phone

``Client`` (text)
    client name, usually string Gammu with version

``Battery`` (integer)
    battery level in percent (or \-1 if unknown)

``Signal`` (integer)
    signal level in percent (or \-1 if unknown)

``Sent`` (integer)
    Number of sent SMS messages (SMSD does not reset this counter, so it might
    overflow).

``Received`` (integer)
    Number of received SMS messages (SMSD does not reset this counter, so it might
    overflow).

sentitems
+++++++++

Log of sent messages (and unsent ones with error code). Also if delivery
reports are enabled, message state is updated after receiving delivery report.

Fields description:

``UpdatedInDB`` (timestamp)
    when somebody (daemon, user, etc.) updated it

``InsertIntoDB`` (timestamp)
    when message was inserted into database

``SendingDateTime`` (timestamp)
    when message has been sent

``DeliveryDateTime`` (timestamp)
    Time of receiving delivery report (if it has been enabled).

``Status`` (enum('SendingOK', 'SendingOKNoReport', 'SendingError', 'DeliveryOK', 'DeliveryFailed', 'DeliveryPending', 'DeliveryUnknown', 'Error'))
    Status of message sending. SendingError mens that phone failed to send the
    message, Error indicates some other error while processing message.

    ``SendingOK``
        Message has been sent, waiting for delivery report.
    ``SendingOKNoReport``
        Message has been sent without asking for delivery report.
    ``SendingError``
        Sending has failed.
    ``DeliveryOK``
        Delivery report arrived and reported success.
    ``DeliveryFailed``
        Delivery report arrived and reports failure.
    ``DeliveryPending``
        Delivery report announced pending deliver.
    ``DeliveryUnknown``
        Delivery report reported unknown status.
    ``Error``
        Some other error happened during sending (usually bug in SMSD).

``StatusError`` (integer)
    Status of delivery from delivery report message, codes are defined in GSM
    specification 03.40 section 9.2.3.15 (TP-Status).

``Text`` (text)
    SMS text encoded using hex values

``DestinationNumber`` (varchar(20))
    decoded destination number for SMS

``Coding`` (enum('Default_No_Compression', 'Unicode_No_Compression', '8bit', 'Default_Compression', 'Unicode_Compression'))
    SMS text coding

``UDH`` (text)
    User Data Header encoded using hex values

``SMSCNumber`` (varchar(20))
    decoded number of SMSC, which sent SMS

``Class`` (integer)
    SMS class or \-1 (0 is normal SMS, 1 is flash one)

``TextDecoded`` (varchar(160))
    SMS text in "human readable" form

``ID`` (integer unsigned)
    SMS ID

``SenderID`` (text)
    which SMSD instance sent this one sequence, see :config:option:`PhoneID`

``SequencePosition`` (integer)
    SMS number in SMS sequence

``TPMR`` (integer)
    Message Reference like in GSM specs

``RelativeValidity`` (integer)
    SMS relative validity like encoded using GSM specs

``CreatorID`` (text)
    copied from CreatorID from outbox table, matches PhoneID


pbk
+++

Not used by SMSD currently, included only for application usage.

pbk_groups
++++++++++

Not used by SMSD currently, included only for application usage.

.. _smsd-tables-history:

History of database structure
-----------------------------

.. note::

    Testing versions (see :ref:`versioning`) do not have to keep same table
    structure as final releases. Bellow mentioned versions are for
    informational purposes only, you should always use stable versions in
    production environment.

History of schema versions:

13
    Added ``SendBefore`` and ``SendAfter`` fields.

    .. versionchanged:: 1.29.90

    Also PostgreSQL fields are now case sensitive (same as other backends).

    .. versionchanged:: 1.29.93
12
    the changes only affect MySQL structure changing default values for
    timestamps from ``0000-00-00 00:00:00`` to ``CURRENT_TIMESTAMP()`` by
    using triggers, to update to this version, just execute triggers
    definition at the end of SQL file.

    .. versionchanged:: 1.28.94
11
    all fields for storing message text are no longer limited to 160 chars,
    but are arbitrary length text fields.
    
    .. versionchanged:: 1.25.92
10
    ``DeliveryDateTime`` is now NULL when message is not delivered, added several
    indexes

    .. versionchanged:: 1.22.95
9
    added sent/received counters to phones table

    .. versionchanged:: 1.22.93
8
    Signal and battery state are now stored in database.

    .. versionchanged:: 1.20.94
7
    Added ``CreatorID`` to several tables.

    .. versionchanged:: 1.07.00
6
    Many fields in outbox can now be NULL.

    .. versionchanged:: 1.06.00
5
    Introduced daemons table and various other changes.

    .. versionchanged:: 1.03.00
3
    Introduced phones table and various other changes.

    .. versionchanged:: 0.98.0


Examples
--------

Creating tables
+++++++++++++++

SQL scripts to create all needed tables for most databases are included in
Gammu documentation (docs/sql). As well as some PHP scripts interacting with
the database.

For example to create SQLite tables, issue following command:

.. code-block:: sh

    sqlite3 smsd.db < docs/sql/sqlite.sql

Injecting a message using SQL
+++++++++++++++++++++++++++++

To send a message, you can either use :ref:`gammu-smsd-inject`, which does all the
magic for you, or you can insert the message manually. The simplest example is
short text message:

.. code-block:: sql

    INSERT INTO outbox (
        DestinationNumber,
        TextDecoded,
        CreatorID,
        Coding
    ) VALUES (
        '800123465',
        'This is a SQL test message',
        'Program',
        'Default_No_Compression'
    );

Please note usage of ``TextDecoded`` field, for ``Text`` field, you would have
to hex encode the unicode text:

.. code-block:: sql

    INSERT INTO outbox (
        DestinationNumber,
        Text,
        CreatorID,
        Coding
    ) VALUES (
        '800123465',
        '005400680069007300200069007300200061002000530051004c002000740065007300740020006d006500730073006100670065',
        'Program',
        'Default_No_Compression'
    );

Injecting long message using SQL
++++++++++++++++++++++++++++++++

Inserting multipart messages is a bit more tricky, you need to construct also
UDH header and store it hexadecimally written into UDH field. Unless you have
a good reason to do this manually, use :ref:`gammu-smsd-inject`.

For long text message, the UDH starts with ``050003`` followed by byte as a
message reference (you can put anything there, but it should be different for
each message, ``D3`` in following example), byte for number of messages (``02``
in example, it should be unique for each message you send to same phone number)
and byte for number of current message (``01`` for first message, ``02`` for
second, etc.).

For example long text message of two parts could look like following:

.. code-block:: sql

    INSERT INTO outbox (
        CreatorID,
        MultiPart,
        DestinationNumber,
        UDH,
        TextDecoded,
        Coding
    ) VALUES (
        'Gammu 1.23.91',
        'true',
        '123465',
        '050003D30201',
        'Mqukqirip ya konej eqniu rejropocejor hugiygydewl tfej nrupxujob xuemymiyliralj. Te tvyjuh qaxumur ibewfoiws zuucoz tdygu gelum L ejqigqesykl kya jdytbez',
        'Default_No_Compression'
    )

    INSERT INTO outbox_multipart (
        SequencePosition,
        UDH,
        Class,
        TextDecoded,
        ID,
        Coding
    ) VALUES (
        2,
        '050003D30202',
        'u xewz qisubevumxyzk ufuylehyzc. Nse xobq dfolizygqysj t bvowsyhyhyemim ovutpapeaempye giuuwbib.',
        <ID_OF_INSERTED_RECORD_IN_OUBOX_TABLE>,
        'Default_No_Compression'
    )

.. note::

    Adding UDH means that you have less space for text, in above example you
    can use only 153 characters in single message.

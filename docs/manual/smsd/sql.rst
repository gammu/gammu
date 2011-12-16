.. _gammu-smsd-sql:

SQL Service
===========

Description
-----------

SQL service stores all its data in database. It can use one of these SQL backends
(configuration option :config:option:`Driver` in smsd section):

* ``native_mysql`` for :ref:`gammu-smsd-mysql`
* ``native_pgsql`` for :ref:`gammu-smsd-pgsql`
* ``odbc`` for :ref:`gammu-smsd-odbc`
* drivers supported by DBI for :ref:`gammu-smsd-dbi`, which include:
    * ``sqlite3`` - for SQLite 3
    * ``mysql`` - for MySQL
    * ``pgsql`` - for PostgeSQL
    * ``freetds`` - for MS SQL Server or Sybase

SQL connection parameters
-------------------------

Common for all backends:

* :config:option:`User` - user connecting to database
* :config:option:`Password` - password for connecting to database
* :config:option:`Host` - database host or data source name
* :config:option:`Database` - database name
* :config:option:`Driver` - ``native_mysql``, ``native_pgsql``, ``odbc`` or DBI one
* :config:option:`SQL` - SQL dialect to use

Specific for DBI:

* :config:option:`DriversPath` - path to DBI drivers
* :config:option:`DBDir` - sqlite/sqlite3 directory with database

.. seealso:: The variables are fully described in :ref:`gammurc` documentation.

.. _SQL Queries:

SQL Queries
-----------

Almost all queries are configurable. You can edit them in
:config:section:`[sql]` section. There are several variables used in SQL
queries. We can separate them into three groups:

* phone specific, which can be used in every query, see :ref:`Phone Specific Parameters`
* SMS specific, which can be used in queries which works with SMS messages, see :ref:`SMS Specific Parameters`
* query specific, which are numeric and are specific only for given query (or set of queries), see :ref:`Configurable queries`

.. _Phone Specific Parameters:

Phone Specific Parameters
+++++++++++++++++++++++++

``%I``
    IMEI of phone
``%P``
    PHONE ID (hostname)
``%N``
    client name (eg. Gammu 1.12.3)

.. _SMS Specific Parameters:

SMS Specific Parameters
+++++++++++++++++++++++

``%R``
    remote number [#f1]_
``%C``
    delivery datetime
``%e``
    delivery status on receiving or status error on sending
``%t``
    message reference
``%d``
    receiving datetime for received sms
``%E``
    encoded text of SMS
``%c``
    SMS coding (ie 8bit or UnicodeNoCompression)
``%F``
    sms centre number
``%u``
    UDH header
``%x``
    class
``%T``
    decoded SMS text
``%A``
    CreatorID of SMS (sending sms)
``%V``
    relative validity

.. [#f1] Sender number for received messages (insert to inbox or delivery notifications), destination otherwise.

.. _Configurable queries:

Configurable queries
--------------------

All configurable queries can be set in :config:section:`[sql]` section. Sequence of rows in selects are mandatory.

All default queries noted here are noted for MySQL. Actual time and time addition
are selected for default queries during initialization.

.. config:option:: delete_phone

    Deletes phone from database.

    Default value:

    .. code-block:: sql

        DELETE FROM phones WHERE IMEI = %I

.. config:option:: insert_phone

    Inserts phone to database.

    Default value:

    .. code-block:: sql

        INSERT INTO phones (IMEI, ID, Send, Receive, InsertIntoDB, TimeOut, Client, Battery, SignalStrength)
        VALUES (%I, %P, %1, %2, NOW(), (NOW() + INTERVAL 10 SECOND) + 0, %N, -1, -1)

    Query specific parameters:

    ``%1``
        enable send (yes or no) - configuration option Send
    ``%2``
        enable receive (yes or no)  - configuration option Receive

.. config:option:: save_inbox_sms_select

    Select message for update delivery status.

    Default value:

    .. code-block:: sql

        SELECT ID, Status, SendingDateTime, DeliveryDateTime, SMSCNumber FROM sentitems
        WHERE DeliveryDateTime IS NULL AND SenderID = %P AND TPMR = %t AND DestinationNumber = %R

.. config:option:: save_inbox_sms_update_delivered

    Update message delivery status if message was delivered.

    Default value:

    .. code-block:: sql

        UPDATE sentitems SET DeliveryDateTime = %C, Status = %1, StatusError = %e WHERE ID = %2 AND TPMR = %t

    Query specific parameters:

    ``%1``
        delivery status returned by GSM network
    ``%2``
        ID of message

.. config:option:: save_inbox_sms_update

    Update message if there is an delivery error.

    Default value:

    .. code-block:: sql

        UPDATE sentitems SET Status = %1, StatusError = %e WHERE ID = %2 AND TPMR = %t

    Query specific parameters:

    ``%1``
        delivery status returned by GSM network
    ``%2``
        ID of message

.. config:option:: save_inbox_sms_insert

    Insert received message.

    Default value:

    .. code-block:: sql

        INSERT INTO inbox (ReceivingDateTime, Text, SenderNumber, Coding, SMSCNumber, UDH,
        Class, TextDecoded, RecipientID) VALUES (%d, %E, %R, %c, %F, %u, %x, %T, %P)

.. config:option:: update_received

    Update statistics after receiving message.

    Default value:

    .. code-block:: sql

        UPDATE phones SET Received = Received + 1 WHERE IMEI = %I

.. config:option:: refresh_send_status

    Update messages in outbox.

    Default value:

    .. code-block:: sql

        UPDATE outbox SET SendingTimeOut = (NOW() + INTERVAL locktime SECOND) + 0
        WHERE ID = %1 AND (SendingTimeOut < NOW() OR SendingTimeOut IS NULL)

    Query specific parameters:

    ``%1``
        ID of message

.. config:option:: find_outbox_sms_id

    Find sms messages for sending.

    Default value:

    .. code-block:: sql

        SELECT ID, InsertIntoDB, SendingDateTime, SenderID FROM outbox
        WHERE SendingDateTime < NOW() AND SendingTimeOut <  NOW() AND
        SendBefore >= CURTIME() AND SendAfter <= CURTIME() AND
        ( SenderID is NULL OR SenderID = '' OR SenderID = %P ) ORDER BY InsertIntoDB ASC LIMIT %1

    Query specific parameters:

    ``%1``
        limit of sms messages sended in one walk in loop

.. config:option:: find_outbox_body

    Select body of message.

    Default value:

    .. code-block:: sql

        SELECT Text, Coding, UDH, Class, TextDecoded, ID, DestinationNumber, MultiPart,
        RelativeValidity, DeliveryReport, CreatorID FROM outbox WHERE ID=%1

    Query specific parameters:

    ``%1``
        ID of message

.. config:option:: find_outbox_multipart

    Select remaining parts of sms message.

    Default value:

    .. code-block:: sql

        SELECT Text, Coding, UDH, Class, TextDecoded, ID, SequencePosition
        FROM outbox_multipart WHERE ID=%1 AND SequencePosition=%2

    Query specific parameters:

    ``%1``
        ID of message
    ``%2``
        Number of multipart message

.. config:option:: delete_outbox

    Remove messages from outbox after threir successful send.

    Default value:

    .. code-block:: sql

        DELETE FROM outbox WHERE ID=%1

    Query specific parameters:

    ``%1``
        ID of message

.. config:option:: delete_outbox_multipart

    Remove messages from outbox_multipart after threir successful send.

    Default value:

    .. code-block:: sql

        DELETE FROM outbox_multipart WHERE ID=%1

    Query specific parameters:

    ``%1``
        ID of message

.. config:option:: create_outbox

    Create message (insert to outbox).

    Default value:

    .. code-block:: sql

        INSERT INTO outbox (CreatorID, SenderID, DeliveryReport, MultiPart,
        InsertIntoDB, Text, DestinationNumber, RelativeValidity, Coding, UDH, Class,
        TextDecoded) VALUES (%1, %P, %2, %3, NOW(), %E, %R, %V, %c, %u, %x, %T)

    Query specific parameters:

    ``%1``
        creator of message
    ``%2``
        delivery status report - yes/default
    ``%3``
        multipart - FALSE/TRUE
    ``%4``
        Part (part number)
    ``%5``
        ID of message

.. config:option:: create_outbox_multipart

    Create message remaining parts.

    Default value:

    .. code-block:: sql

        INSERT INTO outbox_multipart (SequencePosition, Text, Coding, UDH, Class,
        TextDecoded, ID) VALUES (%4, %E, %c, %u, %x, %T, %5)

    Query specific parameters:

    ``%1``
        creator of message
    ``%2``
        delivery status report - yes/default
    ``%3``
        multipart - FALSE/TRUE
    ``%4``
        Part (part number)
    ``%5``
        ID of message

.. config:option:: add_sent_info

    Insert to sentitems.

    Default value:

    .. code-block:: sql

        INSERT INTO sentitems (CreatorID,ID,SequencePosition,Status,SendingDateTime,
        SMSCNumber, TPMR, SenderID,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,
        InsertIntoDB,RelativeValidity)
        VALUES (%A, %1, %2, %3, NOW(), %F, %4, %P, %E, %R, %c, %u, %x, %T, %5, %V)

    Query specific parameters:

    ``%1``
        ID of sms message
    ``%2``
        part number (for multipart sms)
    ``%3``
        message state (SendingError, Error, SendingOK, SendingOKNoReport)
    ``%4``
        message reference (TPMR)
    ``%5``
        time when inserted in db

.. config:option:: update_sent

    Update sent statistics after sending message.

    Default value:

    .. code-block:: sql

         UPDATE phones SET Sent= Sent + 1 WHERE IMEI = %I

.. config:option:: refresh_phone_status

    Update phone status (battery, signal).

    Default value:

    .. code-block:: sql

        UPDATE phones SET TimeOut= (NOW() + INTERVAL 10 SECOND) + 0,
        Battery = %1, SignalStrength = %2 WHERE IMEI = %I

    Query specific parameters:

    ``%1``
        battery percent
    ``%2``
        signal percent

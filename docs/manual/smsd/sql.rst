.. _gammu-smsd-sql:

SQL Service
===========

SQL service stores all its data in database. It can use one of these SQL backends 
(configuration option :config:option:`Driver` in smsd section):

* ``native_mysql``
* ``native_pgsql``
* drivers supported by DBI

SQL connection parameters
-------------------------
Common for all queries:

* :config:option:`User` - user connecting to database
* :config:option:`Password` - password for connecting to database
* :config:option:`Host` - database host
* :config:option:`Database` - database name
* :config:option:`Driver` - native_mysql, native_pgsql or DBI one

Specific for DBI:

* :config:option:`DriversPath` - path to DBI drivers
* :config:option:`DBDir` - sqlite/sqlite3 directory with database

SQL Queries
-----------
Almost all queries are configurable. You can edit it in [sql] section. There are variables
used in SQL queries. We can separate it to three groups:

* phone specific, which can be used in every query
* SMS specific, which can be used in queries which works with SMS messages
* query specific, which are numeric and are specific only for a few queries

+-----------+-----------------+------------------------------------------------------------------+
| Parameter |	SMS Specific? |	Description				       	                 |
+-----------+-----------------+------------------------------------------------------------------+
| %I	    |      no	      | provides IMEI of phone                                           |
+-----------+-----------------+------------------------------------------------------------------+
| %P	    |      no	      | provides PHONE ID (hostname)                                     |
+-----------+-----------------+------------------------------------------------------------------+
| %N        |	   no	      | client name (eg. Gammu 1.12.3)                                   |
+-----------+-----------------+------------------------------------------------------------------+
| %R	    |      yes	      | in received sms (insert to inbox, delivery notifications)        |
|           |                 |	it provides sender number, else it provides destination number   |
+-----------+-----------------+------------------------------------------------------------------+
| %C        |	   yes	      | provides delivery datetime                                       |
+-----------+-----------------+------------------------------------------------------------------+
| %e	    |      yes	      | provides delivery status on receiving or status error on sending |
+-----------+-----------------+------------------------------------------------------------------+
| %t	    |	   yes	      | message reference						 |
+-----------+-----------------+------------------------------------------------------------------+
| %d	    |	   yes	      | receiving datetime for received sms                              |
+-----------+-----------------+------------------------------------------------------------------+
| %E	    |	   yes	      | encoded text of SMS                                              |
+-----------+-----------------+------------------------------------------------------------------+
| %c	    |	   yes	      | SMS coding (ie 8bit or UnicodeNoCompression)                     |
+-----------+-----------------+------------------------------------------------------------------+
| %F	    |	   yes	      | sms centre number                                                |
+-----------+-----------------+------------------------------------------------------------------+
| %u	    |      yes	      | UDH header                                                       |
+-----------+-----------------+------------------------------------------------------------------+
| %x	    |      yes	      |	class                                                            |
+-----------+-----------------+------------------------------------------------------------------+
| %T	    |      yes	      |	decoded SMS text                                                 |
+-----------+-----------------+------------------------------------------------------------------+
| %A        |      yes	      | CreatorID of SMS (sending sms)                                   |
+-----------+-----------------+------------------------------------------------------------------+
| %V	    |      yes	      | relative validity                                                |
+-----------+-----------------+------------------------------------------------------------------+


Query specific parameters
-------------------------
INSERT_PHONE
 1) enable send (yes or no) - configuration option Send
 2) enable receive (yes or no)  - configuration option Receive

SAVE_INBOX_SMS_UPDATE_DELIVERED, SAVE_INBOX_SMS_UPDATE
 1) delivery status returned by GSM network
 2) ID of message

REFRESH_SEND_STATUS
 1) ID of message

FIND_OUTBOX_SMS
 1) limit of sms messages sended in one walk in loop

FIND_OUTBOX_BODY, FIND_OUTBOX_MULTIPART
 1) ID of message
 2) number of multipart message

DELETE_OUTBOX, DELETE_OUTBOX_MULTIPART
 1) ID of message

CREATE_OUTBOX, CREATE_OUTBOX_MULTIPART
 1) creator of message
 2) delivery status report - yes/default
 3) multipart - FALSE/TRUE
 4) Part (part number)
 5) id of message

ADD_SENT_INFO
 1) ID of sms message
 2) part number (for multipart sms)
 3) message state (SendingError, Error, SendingOK, SendingOKNoReport)
 4) message reference (TPMR)
 5) time when inserted in db

REFRESH_PHONE_STATUS
 1) battery percent
 2) signal percent

Configurable queries
--------------------
All configurable queries can be set in [sql] section. sequence of rows in selects are mandatory.

* delete_phone - deletes phone from database
* insert_phone - inserts phone to database
* save_inbox_sms_select - select message for update delivery status
* save_inbox_sms_update_delivered - update message delivery status if message was delivered
* save_inbox_sms_update - update message if there is an delivery error
* save_inbox_sms_insert - insert received message
* update_received - update statistics after receiving message
* refresh_send_status - update messages in outbox
* find_outbox_sms_id - find sms messages for sending
* find_outbox_body - select body of message
* find_outbox_multipart - select remaining parts of sms message
* delete_outbox - remove messages from outbox after threir successful send
* delete_outbox_multipart - remove messages from outbox_multipart after threir successful send
* create_outbox - create message (insert to outbox)
* create_outbox_multipart - create message remaining parts
* add_sent_info - insert to sentitems 
* update_sent - update sent statistics after sending message
* refresh_phone_status - update phone status (battery, signal)

Default SQL queries
-------------------
All queries noted here are noted for mysql. Actual time and time addition 
are selected for default queries during initialization.

*delete_phone* 

::

	DELETE FROM phones WHERE IMEI = %I

*insert_phone*

::

	INSERT INTO phones (IMEI, ID, Send, Receive, InsertIntoDB, TimeOut, Client, Battery, Signal) 
	VALUES (%I, %P, %1, %2, NOW(), (NOW() + INTERVAL 10 SECOND) + 0, %N, -1, -1)"

*save_inbox_sms_select* 

::

	SELECT ID, Status, SendingDateTime, DeliveryDateTime, SMSCNumber FROM sentitems 
	WHERE DeliveryDateTime IS NULL AND SenderID = %P AND TPMR = %t AND DestinationNumber = %R

*save_inbox_sms_update_delivered*

::

	UPDATE sentitems SET DeliveryDateTime = %C, Status = %1, StatusError = %e WHERE ID = %2 AND TPMR = %t

*save_inbox_sms_update*

::

	UPDATE sentitems SET Status = %1, StatusError = %e WHERE ID = %2 AND TPMR = %t

*save_inbox_sms_insert*

::

	INSERT INTO inbox (ReceivingDateTime, Text, SenderNumber, Coding, SMSCNumber, UDH, 
	Class, TextDecoded, RecipientID) VALUES (%d, %E, %R, %c, %F, %u, %x, %T, %P)

*update_received*

::

	UPDATE phones SET Received = Received + 1 WHERE IMEI = %I

*reresh_send_status*

::

	UPDATE outbox SET SendingTimeOut = (NOW() + INTERVAL locktime SECOND) + 0 
	WHERE ID = %1 AND (SendingTimeOut < NOW() OR SendingTimeOut IS NULL)

*find_outbox_sms_id* 

::

	SELECT ID, InsertIntoDB, SendingDateTime, SenderID FROM outbox 
	WHERE SendingDateTime < NOW() AND SendingTimeOut <  NOW() AND 
	( SenderID is NULL OR SenderID = '' OR SenderID = %P ) ORDER BY InsertIntoDB ASC LIMIT %1"

*find_outbox_body*

::

	SELECT Text, Coding, UDH, Class, TextDecoded, ID, DestinationNumber, MultiPart, 
	RelativeValidity, DeliveryReport, CreatorID FROM outbox WHERE ID=%1

*find_outbox_multipart*

::

	SELECT Text, Coding, UDH, Class, TextDecoded, ID, SequencePosition 
	FROM outbox_multipart WHERE ID=%1 AND SequencePosition=%2"

*delete_outbox*

::

	DELETE FROM outbox WHERE ID=%1

*delete_outbox_multipart*

::

	DELETE FROM outbox_multipart WHERE ID=%1

*create_outbox*

::

	INSERT INTO outbox (CreatorID, SenderID, DeliveryReport, MultiPart, 
	InsertIntoDB, Text, DestinationNumber, RelativeValidity, Coding, UDH, Class, 
	TextDecoded) VALUES (%1, %P, %2, %3, NOW(), %E, %R, %V, %c, %u, %x, %T)

*create_outbox_multipart*

::

	INSERT INTO outbox_multipart (SequencePosition, Text, Coding, UDH, Class, 
	TextDecoded, ID) VALUES (%4, %E, %c, %u, %x, %T, %5)

*add_sent_info*

::

	INSERT INTO sentitems (CreatorID,ID,SequencePosition,Status,SendingDateTime,
	SMSCNumber, TPMR, SenderID,Text,DestinationNumber,Coding,UDH,Class,TextDecoded,
	InsertIntoDB,RelativeValidity) 
	VALUES (%A, %1, %2, %3, NOW(), %F, %4, %P, %E, %R, %c, %u, %x, %T, %5, %V)

*update_sent*

::

	 UPDATE phones SET Sent= Sent + 1 WHERE IMEI = %I

*refresh_phone_status*

::

	UPDATE phones SET TimeOut= (NOW() + INTERVAL 10 SECOND) + 0, 
	Battery = %1, Signal = %2 WHERE IMEI = %I"

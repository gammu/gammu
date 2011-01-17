m-obex protocol used by some Samsung mobiles
============================================

This document is copied from <http://code.google.com/p/samsyncro/wiki/mobex> and extended.

Introduction
------------

This is an attempt to document the m-obex protocol. It is a obex-variation by Samsung used to exchange PIM data and files over bluetooth.

This documentation is by no means complete but is only a reference for the samsyncro implementation. As I don't know the obex protocol I can't say in which parts it differs from the standard-obex. The only thing I found strange is the fact, that you will always get 0xA0 as a response. Wich means Ok, sucess in obex. If there was an error you will find it's error code in the 0x42 header. If this is a normal behavior: Why are there so many response codes defined?

The information about the protocol was gained by listening to the transfered data from Samsungs New PC Studio to a SGH-F480i and B2100 mobile.

Requirements
------------

* Established bluetooth connection to the serial channel of the mobile
* Some way to access this serial port. For example minicom.

Starting the obex server
------------------------

To start the obex server you have to send this AT command first::

    AT+SYNCML=MOBEXSTART

Some phones seem to start with following command::

    AT$TSSPCSW=1

Obex commands
-------------

In the following chapters I will describe the obex packages to read and edit data on the mobile. I think most of them are in standard-obex format and are following this structure:

+-------------------------------------------+-----------------------------------+-------------------------------------------+
|Package Header                             |Session Id                         |Obex Header(s)                             |
+===========================================+===================================+===========================================+
|* First byte: Type of request.             |* 0xCB and four bytes of session id|* First byte: Type of header.              |
|* Second and third bytes: length of package|                                   |* Second and third bytes: length of header.|
|                                           |                                   |* Next bytes: data.                        |
|                                           |                                   |* Last byte: 0x00                          |
+-------------------------------------------+-----------------------------------+-------------------------------------------+

For detailed information about obex, for example what types of packages and headers exists, get the official Obex documentation from Inrared Data Association. But I don't know if this is available for free.

Here is a list of the most used types for the Samsung mobiles:

There exists mainly two types of operations: Put (package header 0x02 and 0x82) to write data to the mobile and Get (package header 0x03 and 0x83) to retrieve data from the mobile. A put or get operation can be divided into several packages. The high-bit indicates if this is the last package of an operation. For example if you want to transfer a file to the mobile you send n-time 0x02 packages and only the last one is 0x82.

Headers consists normally out of three blocks: First byte: Header type, second and third byte: length of the header (if the headers length is variable), following bytes: data. The most used header types are

+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| Obex description      | Byte | following two bytes    | following bytes                                                                                                                                   |
+=======================+======+========================+===================================================================================================================================================+
| Name                  | 0x01 | length of header       | Used for filesystem operation to name a path or file                                                                                              |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| Type                  | 0x42 | length of header       | Obex command for example "m-obex/contacts/list"                                                                                                   |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| Length                | 0xC3 |                        | Used in put operations and specifies the length of the transfered data (without header bytes). The length is represented in 4 bytes.              |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| Body                  | 0x48 | length of header       | Data in a multi-package put operation                                                                                                             |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| End of Bady           | 0x49 | length of header       | Last data package in a put operation                                                                                                              |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| Session id            | 0xCB |                        | Four bytes representing the session id. Needed for multiplexing                                                                                   |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+
| Application Parameter | 0X4C | length of header       | In a request: Parameters for example a contact's id. <p>In an answer: The error/return code. If it is 0x00 0x00 than the operation was successful |
+-----------------------+------+------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------+

Contacts
--------

Get contacts count
++++++++++++++++++

Request
~~~~~~~

*83* 00 25
    Obex Get
CB 00 00 00 00
    Session Id
*42* 00 19 6D 2D 6F 62 65 78 2F 63 6F 6E 74 61 63 74 73 2F 63 6F 75 6E 74 00
    m-obex command: m-obex/contacts/count
4C 00 04 01
    Unknown! Didn' see PC Studio sending something other than 0x01 as parameter

Answer
~~~~~~
*A0* 00 14
    Obex ok
*C3* 00 00 00 04
    Maybe the number of requests you have to send to get all contacts. See next chapter for more information
*4C* 00 05 00 00
    Error code
*49* 00 07 07 D0 00 18
    First two data bytes: maximal number of contacts (0x07D0 = 2000). Last two data bytes: Current number of contacts


List all
++++++++

Request
~~~~~~~
*83* 00 26
    Obex Get package
*CB* 00 00 00 00
    Session Id
*42* 00 18 6D 2D 6F 62 65 78 2F 63 6F 6E 74 61 63 74 73 2F 6C 6F 61 64 00
    m-obex Command: m-obex/contacts/load
*4C* 00 06 01 00 00
    First Byte unknown. Last two bytes: increment until all contacts recieved

Answer
~~~~~~
*A0* 08 C1
    Obex Ok
*C3* 00 00 08 B1
    Length of sent data
*4C* 00 05 00 02
    Indicates if these are the last contacts
*49* 07 41 01 10 01 8D ...."
    The first byte is unknown but all answers have this byte, then byte 2 and 3 contains the length of the answer, bytes 4 and 5 are the ID of the first entry bytes 6 and 7 are the length of this entry.

    In one response more than 1 vcard can be returned in this case, entries are separated by 4 bytes with the following meaning: bytes 1 and 2 ID of the entry, bytes 3 and 4: length of the entry.

To get all contacts the request have to be sent several times. The last two bytes must be incremented by every call.

The end of the contacts list is reached if the header 0x4C is 0. The header will be 4C 00 05 00 00.


Create a contact
++++++++++++++++

Beware: This is a put operation and is performed in some obex implementations in several packages (for example 0x02, 0x02, 0x82). But I didn't get the mobile to accept this. I had to create/update PIM data in exactly one package.

Request
~~~~~~~
*82* 00 88
    Obex put
*CB* 00 00 00 00
    Session id
*42* 00 1A 6D 2D 6F 62 65 78 2F 63 6F 6E 74 61 63 74 73 2F 63 72 65 61 74 65 00
    m-obex/contacts/create
*4C* 00 04 01
    ? maybe flag for internal/external memory
C3 00 00 00 5A
    Length of the vcard string
49 00 5D 42 45....
    Contact as vcard

Answer
~~~~~~
*A0* 00 12
    Obex ok
*C3* 00 00 00 02
    ?
*4C* 00 05 00 00
    Error code
*49* 00 05 00 21
    last two bytes: the id of the newly created contact


Update a contact
++++++++++++++++

Beware: This is a put operation and is performed in some obex implementations in several packages (for example 0x02, 0x02, 0x82). But I didn't get the mobile to accept this. I had to create/update PIM data in exactly one package.

Request
~~~~~~~
*82* 00 8D
    Obex put
*CB* 00 00 00 00
    Session id
*42* 00 19 6D 2D 6F 62 65 78 2F 63 6F 6E 74 61 63 74 73 2F 77 72 69 74 65 00
    m-obex/contacts/write
*4C* 00 06 01 00 20
    Id of the contact which should be updated
C3 00 00 00 5E
    Length of the vcard string
49 00 61 42...
    Contact as vcard

Answer
~~~~~~
*A0* 00 08
    Obex ok
*4C* 00 05 00 00
    Error code: 0x00 0x00 means successful


Read one contact
++++++++++++++++

There is also the possibility to read exactly one contact.

Request
~~~~~~~
*83* 00 26
    Obex get
*CB* 00 00 00 00
    Session id
*42* 00 18 6D 2D 6F 62 65 78 2F 63 6F 6E 74 61 63 74 73 2F 72 65 61 64 00
    m-obex/contacts/read
*4C* 00 06 01 00 20
    First byte:? Last two bytes: Id of contact

Answer
~~~~~~
*A0* 00 C4
    Obex ok
*C3* 00 00 00 B4
    Length of vcard (without headers, just data)
*4C* 00 05 00 00
    Error code
49 00 B7 42 45 47 49 4E ...
    contact as vcard. TODO: where is id? First two bytes?

Delete contact
++++++++++++++

To delete a contact you only have to know it's id.

Request
~~~~~~~
*82* 00 28
    Obex put
*CB* 00 00 00 00
    Session id
*42* 00 1A 6D 2D 6F 62 65 78 2F 63 6F 6E 74 61 63 74 73 2F 64 65 6C 65 74 65 00
    m-obex/contacts/delete
*4C* 00 06 01 00 19
    First byte: ? Last two bytes: Id of contact

Answer
~~~~~~

*A0* 00 08
    Obex ok
*4C* 00 05 00 00
    Error code


Calendar
--------

Get count
+++++++++

Request
~~~~~~~
*83* 00 25
    Obex get
*CB* 00 00 00 00
    Session id
*42* 00 19 6D 2D 6F 62 65 78 2F 63 61 6C 65 6E 64 61 72 2F 63 6F 75 6E 74 00
    m-obex/calendar/count
*4C* 00 04 FF
    ?

Answer
~~~~~~
A0 00 1C
    Obex ok
*C3* 00 00 00 0C
    length of data
*4C* 00 05 00 00
    Error code
*49* 00 0F 01 2C 00 06 00 64 00 00 00 64 00 00
    ?TODO?

List all
++++++++

Request
~~~~~~~
*83* 00 20
    Obex get
*CB* 00 00 00 00
    Session id
*42* 00 18 6D 2D 6F 62 65 78 2F 63 61 6C 65 6E 64 61 72 2F 69 6E 66 6F 00
    m-obex/calendar/load


Answer
~~~~~~
*A0* 00 C0
    Obex ok
*C3* 00 00 00 B0
    Session
*4C* 00 05 00 00
    Error code
*49* 00 B3 01 07 08 00 00 00 00 00 00 00 00 ...
    Calendar items in vcalendar format. TODO: where are the ids?

Create
++++++

Request
~~~~~~~
*82* 00 CC
    Obex put
*CB* 00 00 00 00
    Session
*42* 00 1A 6D 2D 6F 62 65 78 2F 63 61 6C 65 6E 64 61 72 2F 63 72 65 61 74 65 00
    m-obex/calendar/create
4C 00 04 01
    ?
*C3* 00 00 00 9E
    Length of vcalendar
*49* 00 A1 42 45 47 49 4E 3A 56 43 41 4C 45 ...
    vcalendar

Answer
~~~~~~
*A0* 00 12
    Obex ok
*C3* 00 00 00 02
    Length
*4C* 00 05 00 00
    Error code
*49* 00 05 00 06
    Id of the created item

Update
++++++

Request
~~~~~~~
*82* 00 F7
    Obex put
*CB* 00 00 00 00
    Session
*42* 00 19 6D 2D 6F 62 65 78 2F 63 61 6C 65 6E 64 61 72 2F 77 72 69 74 65 00
    m-obex/calendar/write
*4C* 00 06 01 00 05
    First byte: ? Second and third byte: Id of the item
*C3* 00 00 00 C8
    Length of vcalendar
*49* 00 CB 42 45 47 49 4E 3A 56
    vcalendar item

Answer
~~~~~~
*A0* 00 08
    Obex ok
*4C* 00 05 00 00
    Error code


Read
++++

Request
~~~~~~~
*83* 00 26
    Obex get
*CB* 00 00 00 00
    Session
*42* 00 18 6D 2D 6F 62 65 78 2F 63 61 6C 65 6E 64 61 72 2F 72 65 61 64 00
    m-obex/calendar/read
4C 00 06 01 00 06
    Id of calendar item

Answer
~~~~~~
*A0* 00 C0
    Obex ok
*C3* 00 00 00 B0
    Length
*4C* 00 05 00 00
    Error code
*49* 00 B3 42 45 47 49 4E 3A 56 43 41 4C 45 4E 44 41 52 0D 0A 56 45 52 53 49 4F 4E 3A 31 2E 3....
    vcalendar item. TODO: Where is the id?

Delete
++++++

Request
~~~~~~~
*82* 00 28
    Obex put
*CB* 00 00 00 00
    Session
*42* 00 1A 6D 2D 6F 62 65 78 2F 63 61 6C 65 6E 64 61 72 2F 64 65 6C 65 74 65 00
    m-obex/calendar/delete
*4C* 00 06 01 00 06
    id of calendar item


Notes
-----


Tasks
-----


Files
-----

To get the file structure on the mobile, there are two commands. One that lists all subdirectories and one that lists all files.

List directories
++++++++++++++++

List files
++++++++++

Get file
++++++++

Create file
+++++++++++

Delete file
+++++++++++


SMS
---

0x01: Inbox
0x08: Outbox

Get sms count
+++++++++++++

List all sms
++++++++++++

Send sms
++++++++

Create sms
++++++++++

I don't think this is possible. At least I didn't find the function in New PC Studio. So sadly there will be no backup of sms messages.

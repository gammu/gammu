.. _reply-functions:

Reply functions
===============

When phone gives answers, we check if we requested received info and we
redirect it to concrete reply function, which will decode it. Different
phone answers can go to one reply function let's say responsible for
getting sms status. 

.. c:type:: GSM_Reply_Function

    Defines reply function for phone driver.

    .. c:member:: GSM_Error (*Function)	(GSM_Protocol_Message *msg, GSM_StateMachine *s);

        Callback on reply match.

    .. c:member:: const unsigned char		*msgtype;

        String match on the message.

    .. c:member:: const size_t			subtypechar;

        Position for char match inside reply. If 0, message type is checked.

    .. c:member:: const int			subtype;

       Match for char/message type check (see above).

    .. c:member:: const GSM_Phone_RequestID	requestID;

       Match for request ID. this is filled in when calling :c:func:`GSM_WaitFor`.

There are three types of answer matching:

Binary
------

Example:

.. code-block:: c

    {N6110_ReplySaveSMSMessage,"\x14",0x03,0x05,ID_SaveSMSMessage},

ID_SaveSMSMessage request function reply. Frame is type "\x14",
0x03 char of frame must be 0x05. If yes, we go to N6110_ReplySaveSMSMessage.
Of course, things like frame type are found in protocol (here FBUS, MBUS,
etc.) funcitons. If don't need anything more than frame type, 0x03,0x05
should be 0x00, 0x00 - it means then, that we check only frame type.

Text
----

Example:

.. code-block:: c

    {ATGEN_ReplyIncomingCallInfo,"+CLIP",0x00,0x00,ID_IncomingFrame},

All incoming (not requested in the moment, sent by phone, who
likes us - ID_IncomingFrame) responses starting from "+CLIP" will go
to the ATGEN_ReplyIncomingCallInfo.

Numeric
-------

Example:

.. code-block:: c

	{S60_Reply_Generic, "", 0x00, NUM_QUIT, ID_Terminate },

When match string is empty and match char position is zero, matching on message
type is performed.

Requests
--------

This is how GSM_Reply_Function is filled. Now how to make phone requests ?

Example:

.. code-block:: c

    static GSM_Error N6110_GetMemory (GSM_StateMachine   *s,
                                     GSM_PhonebookEntry *entry)
    {
      unsigned char req[] = {
           N6110_FRAME_HEADER, 0x01,
           0x00,            /* memory type */
           0x00,            /* location */
           0x00};

      req[4] = NOKIA_GetMemoryType(entry->MemoryType,N6110_MEMORY_TYPES);
      if (req[4]==0xff) return GE_NOTSUPPORTED;

      req[5] = entry->Location;

      s->Phone.Data.Memory=entry;
      dprintf("Getting phonebook entry\n");
      return GSM_WaitFor (s, req, 7, 0x03, 4, ID_GetMemory);
    }

First we fill req according to values in \*entry. Later set pointer
in s->Phone.Data (it's available for reply functions and they set
responses exactly to it) and use GSM_WaitFor. It uses s statemachine,
sends req frame with length 7, msg type is 0x03, we wait for answer
during 4 seconds, request id is ID_GetMemory. GSM_WaitFor internally
checks incoming bytes from phone and redirect them to protocol functions.
If they found full frame, there is checked GSM_Reply_Function, where is
called ReplyFunction or showed debug info, that frame is unknown. If
there is ReplyFunction, it has access to s->Phone.Data and decodes answer.
Returns error or not (and this is value for GSM_WaitFor). If there is
no requested answer during time, GSM_WaitFor returns GE_TIMEOUT.



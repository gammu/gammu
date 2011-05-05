Developer documentation
=======================

Backend services
----------------

The backend service is responsible for storing received messages and giving
the SMSD core messages to send. It is solely up to them how the message will
be stored, for example currently Gammu includes backends to store messages on
filesystem (:ref:`gammu-smsd-files`), various databases (:ref:`gammu-smsd-mysql`,
:ref:`gammu-smsd-pgsql`, :ref:`gammu-smsd-dbi`) or backend which does not store anything
at all (:ref:`gammu-smsd-null`).


Backend interface
+++++++++++++++++

Each backend service needs to support several operations, which are exported
in ``GSM_SMSDService`` structure:

.. c:function:: GSM_Error	GSM_SMSDService::Init 	      (GSM_SMSDConfig *Config)

    Initializes internal state, connect to backend storage.

    :param Config: Pointer to SMSD configuration data
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::Free 	      (GSM_SMSDConfig *Config)

    Freeing internal data, disconnect from backend storage.

    :param Config: Pointer to SMSD configuration data
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::InitAfterConnect   (GSM_SMSDConfig *Config)

    Optional hook called after SMSD is connected to phone, can be used for storing infromation about phone in backend.

    :param Config: Pointer to SMSD configuration data
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::SaveInboxSMS       (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char **Locations)

    Saves message into inbox.

    :param sms: Message data to save
    :param Config: Pointer to SMSD configuration data
    :param Locations: Newly allocation pointer to string with IDs identifying saved messages.
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::FindOutboxSMS      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID)

    Finds message in outbox suitable for sending.

    :param sms: Found outbox message will be stored here
    :param Config: Pointer to SMSD configuration data
    :param ID: Identification of found message will be stored here, this
        should be unique for different message, so that repeated attempts to
        send same message can be detected by SMSD core. Empty string avoids
        this check.
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::MoveSMS  	      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, gboolean alwaysDelete, gboolean sent)

    Moves sent message from outbox to sent items.

    :param sms: Message which should be moved, backend usually can get it by ID as well.
    :param Config: Pointer to SMSD configuration data.
    :param ID: Identification of message to be moved.
    :param alwaysDelete: Whether to delete message from outbox even if moving fails.
    :param sent: Whether message was sent (``TRUE``) or there was a failure (``FALSE``).
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::CreateOutboxSMS    (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *NewID)

    Saves message into outbox queue.

    :param sms: Message data to save
    :param Config: Pointer to SMSD configuration data
    :param NewID: ID of created message will be stored here.
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::AddSentSMSInfo     (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR)

    Logs information about sent message (eg. delivery report).

    :param sms: Message which should be moved, backend usually can get it by ID as well.
    :param Config: Pointer to SMSD configuration data
    :param ID: Identification of message to be marked.
    :param Part: Part of the message which is being processed.
    :param err: Status of sending message.
    :param TPMR: Message reference if available (:term:`TPMR`).
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::RefreshSendStatus  (GSM_SMSDConfig *Config, char *ID)

    Updates sending status in service backend.

    :param Config: Pointer to SMSD configuration data
    :param ID: Identification of message to be marked.
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::RefreshPhoneStatus (GSM_SMSDConfig *Config)

    Updates information about phone in database (network status, battery, etc.).

    :param Config: Pointer to SMSD configuration data
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::ReadConfiguration (GSM_SMSDConfig *Config)

    Reads configuration specific for this backend.

    :param Config: Pointer to SMSD configuration data
    :return: Error code.

Message ID
++++++++++

You might have noticed that message ID is often used in the API. The primary
reason for this is that it is usually easier for backend to handle message
just by it's internal identification instead of handling message data from
:c:type:`GSM_MultiSMSMessage`.

If the backend does not use any IDs internally, it really does not have to
provide them, with only exception of :c:func:`GSM_SMSDService::FindOutboxSMS`,
where ID is used for detection of repeated sending of same message.

The lifetime of ID for sent message:

    * :c:func:`GSM_SMSDService::CreateOutboxSMS` or direct manipulation
      with backend storage creates new ID
    * :c:func:`GSM_SMSDService::FindOutboxSMS` returns ID of message to
      process
    * :c:func:`GSM_SMSDService::AddSentSMSInfo` and
      :c:func:`GSM_SMSDService::RefreshSendStatus` are then notified using
      this ID about sending of the message
    * :c:func:`GSM_SMSDService::MoveSMS` then moves the message based on
      ID to sent items

The lifetime of ID for incoming messages:

    * :c:func:`GSM_SMSDService::SaveInboxSMS` generates the message
    * :ref:`gammu-smsd-run` uses this ID

Message Sending Workflow
------------------------

.. graphviz::

   digraph smsdsending {
      "new message" [shape=box];
      "message in storage" [shape=box];
      "message sent" [shape=box];
      "error sending message" [shape=box];
      "new message" -> "manually created SMS";
      "new message" -> "CreateOutboxSMS";
      "manually created SMS" -> "message in storage";
      "CreateOutboxSMS" -> "message in storage"
      "message in storage" -> "FindOutboxSMS";
      "FindOutboxSMS" -> "AddSentSMSInfo(ERROR)" [label="Error", style=dotted];
      "FindOutboxSMS" -> "check duplicates";
      "check duplicates" -> "AddSentSMSInfo(ERROR)" [label="Too many retries", style=dotted];
      "check duplicates" -> "GSM_SendSMS";
      "GSM_SendSMS" -> "RefreshSendStatus";
      "GSM_SendSMS" -> "AddSentSMSInfo(ERROR)" [label="Error", style=dotted];
      "RefreshSendStatus" -> "RefreshSendStatus" [label="Sending"];
      "RefreshSendStatus" -> "AddSentSMSInfo(ERROR)" [label="Timeout", style=dotted];
      "RefreshSendStatus" -> "AddSentSMSInfo(OK)";
      "AddSentSMSInfo(OK)" -> "MoveSMS(noforce, OK)";
      "MoveSMS(noforce, OK)" -> "MoveSMS(force, ERR)" [label="Error", style=dotted];
      "AddSentSMSInfo(OK)" -> "MoveSMS(force, ERR)" [label="Error", style=dotted];
      "AddSentSMSInfo(ERROR)" -> "MoveSMS(force, ERR)";
      "MoveSMS(noforce, OK)" -> "message sent";
      "MoveSMS(force, ERR)" -> "error sending message";
   }

Message Receiving Workflow
--------------------------

.. graphviz::

   digraph smsdreceiving {
       "received message" [shape=box];
       "ignored message" [shape=box];
       "failed message" [shape=box];
       "waiting message" [shape=box];
       "processed message" [shape=box];
       "received message" -> "GSM_GetNextSMS";
       "GSM_GetNextSMS" -> "SMSD_ValidMessage";
       "SMSD_ValidMessage" -> "GSM_LinkSMS";
       "SMSD_ValidMessage" -> "ignored message" [label="Not valid", style=dotted];
       "GSM_LinkSMS" -> "SMSD_CheckMultipart";
       "SMSD_CheckMultipart" -> "SaveInboxSMS";
       "SMSD_CheckMultipart" -> "waiting message" [label="Not all parts", style=dotted];
       "SaveInboxSMS" -> "SMSD_RunOnReceive" [label="Locations are passed here"];
       "SaveInboxSMS" -> "failed message" [label="Error", style=dotted];
       "SMSD_RunOnReceive" -> "GSM_DeleteSMS";
       "GSM_DeleteSMS" -> "processed message"
       "GSM_DeleteSMS" -> "failed message" [label="Error", style=dotted];
   }

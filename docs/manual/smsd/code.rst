Developer documentation
=======================

Backend services
----------------

Each service needs to support few operations:

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
    :param Locations: Newly allocation pointer to string with locations identifying saved messages.
    :return: Error code.

.. c:function:: GSM_Error	GSM_SMSDService::FindOutboxSMS      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID)

    Finds message in outbox suitable for sending.

    :param sms: Found outbox message will be stored here
    :param Config: Pointer to SMSD configuration data
    :param ID: Identification of found message will be stored here.
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

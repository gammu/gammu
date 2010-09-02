Developer documentation
=======================

Backend services
----------------

Each service needs to support few operations:

* ``Init`` - Initializes internal state, connect to backend storage.
* ``Free`` - Freeing internal data, disconnect from backend storage.
* ``InitAfterConnect`` - Optional hook called after SMSD is connected to phone, can be used for storing infromation about phone in backend.
* ``SaveInboxSMS`` - Saves message into inbox.
* ``FindOutboxSMS`` - Finds message in outbox suitable for sending.
* ``MoveSMS`` - Moves sent message from outbox.
* ``CreateOutboxSMS`` - Saves message into outbox queue.
* ``AddSentSMSInfo`` - Logs information about sent message (eg. delivery report).
* ``RefreshSendStatus`` - Updates information about phone in database (network status, battery, etc.).


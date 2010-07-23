SMS Daemon
==========

Gammu SMS Daemon is a program that periodically scans GSM modem for received
messages, stores them in defined storage and also sends messages enqueued in
this storage.


.. figure:: smsd-interactions.png
   :alt: SMSD interactions

   Interaction of SMSD programs

.. _smsd_services:

Backend services
----------------

The backend service is used to store messages (both incoming and queue of
outgoing ones).

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

Files service
+++++++++++++

Files service stores all messages as files in folders defined by configuration.


Database based services
+++++++++++++++++++++++

All database based services share same (or similar) database schema. Currently
following databases are supported:

* MySQL
* PostgreSQL
* libDBI driver, which can connect to various other databases

Null service
++++++++++++

This service is there primarily for testing purposes, as it does not store
messages at all.

SMSD operation
--------------

The SMSD operation consist of several steps.

1. Process command line options.
2. Configure backend service.
3. Main loop is executed until it is signalled to be terminated.
    1. Try to connect to phone if not connected.
    2. Check for security code if configured (configured by ``CheckSecurity``).
    3. Check for received messages (frequency configured by ``ReceiveFrequency``).
    4. Check for reset of the phone if configured (frequency configured by ``ResetFrequency``).
    5. Check for messages to send (frequency configured by ``CommTimeout``).
    6. Check phone status (frequency configured by ``StatusFrequency``).
    7. Sleep for defined time (``LoopSleep``).
4. Backend service is freed.

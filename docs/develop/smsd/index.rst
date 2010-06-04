SMS Daemon
==========

Gammu SMS Daemon is a program that periodically scans GSM modem for received
messages, stores them in defined storage and also sends messages enqueued in
this storage.


.. figure:: smsd-interactions.png
   :alt: SMSD interactions

   Interaction of SMSD programs

Backend services
----------------

The backend service is used to store messages (both incoming and queue of
outgoing ones).

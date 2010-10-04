Overview
========

Gammu SMS Daemon is a program that periodically scans GSM modem for received
messages, stores them in defined storage and also sends messages enqueued in
this storage.

Overall schema
--------------

The interactions of SMS Daemon and related components can be seen on following
picture.

.. graphviz::

   digraph smsdinteractions {
       "gammu-smsd-inject" [shape=box];
       "gammu-smsd" [shape=box];
       "Run on receive" [shape=box, style=dotted];
       "GSM modem" [shape=box, style=dashed];
       "gammu-smsd-monitor" [shape=box];
       "Service\nstorage" [shape=doublecircle];
       "Kalkun" [shape=box, style=dotted];
       "gammu-smsd" -> "GSM modem" [label="Outgoing"];
       "GSM modem" -> "gammu-smsd" [label="Incoming"];
       "gammu-smsd" -> "Service\nstorage" [label="Save"];
       "Service\nstorage" -> "gammu-smsd" [label="Read"];
       "gammu-smsd" -> "Run on receive" [label="Execute"];
       "Service\nstorage" -> "Run on receive" [label="Read"];
       "Kalkun" -> "Service\nstorage" [label="Inject"];
       "Service\nstorage" -> "Kalkun" [label="Read"];
       "gammu-smsd-inject" -> "Service\nstorage" [label="Inject"];
       "gammu-smsd" -> "gammu-smsd-monitor" [label="Monitor"];
   }

SMSD operation
--------------

The SMSD operation consist of several steps.

1. Process command line options.
2. Configure backend service.
3. Main loop is executed until it is signalled to be terminated.
    1. Try to connect to phone if not connected.
    2. Check for security code if configured (configured by :config:option:`CheckSecurity`).
    3. Check for received messages (frequency configured by :config:option:`ReceiveFrequency`).
    4. Check for reset of the phone if configured (frequency configured by :config:option:`ResetFrequency`).
    5. Check for messages to send (frequency configured by :config:option:`CommTimeout`).
    6. Check phone status (frequency configured by :config:option:`StatusFrequency`).
    7. Sleep for defined time (:config:option:`LoopSleep`).
4. Backend service is freed.

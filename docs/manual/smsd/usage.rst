Usage
=====

This chapter will describe basic ways of using SMSD. It's use is not limited
to these, but they can give you overview of SMSD abilities.

Storing Messages in Backend
---------------------------

The standard mode of operating SMSD. You simply configure backend service, and
all received messages will end up in it and any message you put into outbox
storage will be sent.

Creating Messages to Send
-------------------------

Creating of messages to send heavily depends on service backend you use. Most
of them support :ref:`gammu-smsd-inject`, which can be used to construct
the message, or you can just insert message manually to the backend storage.

Alternatively you can use :c:func:`SMSD_InjectSMS` (from C) or using
:meth:`gammu.smsd.SMSD.InjectSMS` (from Python).

Notification about Received Messages
------------------------------------

Once SMSD receives message and stores it in backend service, it can invoke
your own program to do any message processing, see :ref:`gammu-smsd-run`.

Monitoring SMSD Status
----------------------

You can use :ref:`gammu-smsd-monitor` to monitor status of SMSD. It uses
shared memory segment to get current status of running SMSD.

Alternatively you can get the same functionality from libGammu using
:c:func:`SMSD_GetStatus` or python-gammu using
:meth:`gammu.smsd.SMSD.GetStatus`.

Reporting Bugs
--------------

Please report bugs to <http://bugs.cihar.com>.

Before reporting a bug, please enable verbose logging in SMSD configuration:


.. code-block:: ini

    [smsd]
    debuglevel = 255
    logfile = smsd.log

and include this verbose log within bug report.

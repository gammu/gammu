.. _smsd_run:

RunOnReceive Directive
======================

Description
-----------

Gammu SMSD can be configured by RunOnReceive directive (see :ref:`gammu-smsdrc`
for details) to run defined program after receiving message. 
    
This parameter is executed through shell, so you might need to escape some
special characters and you can include any number of parameters. Additionally
parameters with identifiers of received messages are appended to the command
line. The identifiers depend on used service backend, typically it is ID of
inserted row for database backends or file name for file based backends.

Gammu SMSD waits for the script to terminate. If you make some time consuming
there, it will make SMSD not receive new messages. However to limit breakage
from this situation, the waiting time is limited to two minutes. After this
time SMSD will continue in normal operation and might execute your script
again.

Environment
-----------

program is executed with environment which contains lot of information about
the message. You can use it together with NULL service (see :ref:`smsd_null`)
to implement completely own processing of messages.

Global variables
++++++++++++++++

.. envvar:: SMS_MESSAGES

    Number of physical messages received.

.. envvar:: DECODED_PARTS

    Number of decoded message parts.

Per message variables
+++++++++++++++++++++

The variables further described as ``SMS_1_...`` are generated for each physical
message, where 1 is replaced by current number of message.

.. envvar:: SMS_1_CLASS

    Class of message.

.. envvar:: SMS_1_NUMBER

    Sender number.

.. envvar:: SMS_1_TEXT

    Message text. Text is not available for 8-bit binary messages.

Per part variables
++++++++++++++++++

The variables further described as ``DECODED_1_...`` are generated for each message
part, where 1 is replaced by current number of part. Set are only those
variables whose content is present in the message.

.. envvar:: DECODED_1_TEXT

    Decoded long message text.

.. envvar:: DECODED_1_MMS_SENDER

    Sender of MMS indication message.

.. envvar:: DECODED_1_MMS_TITLE

    title of MMS indication message.

.. envvar:: DECODED_1_MMS_ADDRESS

    Address (URL) of MMS from MMS indication message.

.. envvar:: DECODED_1_MMS_SIZE

    Size of MMS as specified in MMS indication message.


Examples
--------

Passing MMS indication parameters to external program
+++++++++++++++++++++++++++++++++++++++++++++++++++++

Following script (if used as RunOnReceive handler) will write information
about each received MMS indication to the log file. Just replace echo command
with your own program to do custom processing.

.. code-block: sh

    #!/bin/sh
    if [ $DECODED_PARTS \-eq 0 ] ; then
        # No decoded parts, nothing to process
        exit
    fi
    if [ "$DECODED_1_MMS_ADDRESS" ] ; then
        echo "$DECODED_1_MMS_ADDRESS" "$DECODED_1_MMS_SENDER" "$DECODED_1_MMS_TITLE" >> /tmp/smsd-mms.log
    fi

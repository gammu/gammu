.. _gammu-smsd-run:

RunOnReceive Directive
======================

Description
-----------

Gammu SMSD can be configured by :config:option:`RunOnReceive` directive (see
:ref:`gammu-smsdrc` for details) to run defined program after receiving
every message. It can receive single message or more messages, which are parts
of one multipart message.

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

.. note::

    All input and output file descriptors are closed when this program is
    invoked, so you have to ensure to open files on your own.

Environment
-----------

.. versionadded:: 1.28.0

Program is executed with environment which contains lot of information about
the message. You can use it together with NULL service (see
:ref:`gammu-smsd-null`) to implement completely own processing of messages.

Global variables
++++++++++++++++

.. envvar:: SMS_MESSAGES

    Number of physical messages received.

.. envvar:: DECODED_PARTS

    Number of decoded message parts.

.. envvar:: PHONE_ID

    .. versionadded:: 1.38.2

    Value of :config:option:`PhoneID`. Useful when running multiple instances
    (see :ref:`smsd-multi`).

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

.. envvar:: SMS_1_REFERENCE

    .. versionadded:: 1.38.5

    Message Reference. If delivery status received, this variable contains TPMR of original message

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

    .. seealso:: :ref:`faq-mms-download`

.. envvar:: DECODED_1_MMS_SIZE

    Size of MMS as specified in MMS indication message.


Examples
--------

Activating RunOnReceive
+++++++++++++++++++++++

To activate this feature you need to set :config:option:`RunOnReceive` in
the :ref:`gammu-smsdrc`.

.. code-block:: ini

    [smsd]
    RunOnReceive = /path/to/script.sh

Processing messages from the files backend
++++++++++++++++++++++++++++++++++++++++++

Following script (if used as :config:option:`RunOnReceive` handler) passes
message data to other program. This works only with the :ref:`gammu-smsd-files`.

.. literalinclude:: ../../../contrib/smsd-scripts/receive-files
    :language: sh

Invoking commands based on message text
+++++++++++++++++++++++++++++++++++++++

Following script (if used as :config:option:`RunOnReceive` handler) executes
given programs based on message text.

.. literalinclude:: ../../../contrib/smsd-scripts/sms-commands
    :language: sh

Passing message text to program
+++++++++++++++++++++++++++++++

Following script (if used as :config:option:`RunOnReceive` handler) passes
message text and sender to external program.

.. literalinclude:: ../../../contrib/smsd-scripts/receive-exec
    :language: sh

Passing MMS indication parameters to external program
+++++++++++++++++++++++++++++++++++++++++++++++++++++

Following script (if used as :config:option:`RunOnReceive` handler) will write
information about each received MMS indication to the log file. Just replace
echo command with your own program to do custom processing.

.. literalinclude:: ../../../contrib/smsd-scripts/receive-mms
    :language: sh

Processing message text in Python
+++++++++++++++++++++++++++++++++

Following script (if used as :config:option:`RunOnReceive` handler) written
in Python will concatenate all text from received message:

.. literalinclude:: ../../../contrib/smsd-scripts/receive-python
    :language: python

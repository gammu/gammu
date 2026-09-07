.. _gammu-smsd-run:

RunOnReceive Directive
======================

Description
-----------

Gammu SMSD can be configured by :config:option:`RunOnReceive` directive (see
:ref:`gammu-smsdrc` for details) to run defined program after receiving
every message. It can receive single message or more messages, which are parts
of one multipart message.

The configured command can include parameters. SMSD appends identifiers of
received messages as additional arguments. The identifiers depend on the
service backend: typically database row IDs or file names for the files backend.
File names do not include the inbox directory. There can be multiple arguments
for a multipart message, so scripts must handle all supplied identifiers.

On POSIX systems, the configured command is executed through a shell, so special
characters in that command need to be escaped. Message identifiers are passed
as separate literal arguments. SMSD waits for the script to terminate, so slow
scripts can delay receiving new messages. Do not rely on a fixed timeout to
interrupt a blocked script.

On Windows, SMSD starts the configured executable directly. Batch files require
an explicit command interpreter, as shown in :ref:`smsd-run-windows`. SMSD does
not wait for the process to finish or collect its exit status. Hooks can overlap,
and successful process creation does not mean that message forwarding succeeded.

.. note::

    On POSIX systems, standard input is closed and standard output and standard
    error are captured in the SMSD log. On Windows, the hook must arrange its own
    output and error logging. Scripts should not require interactive input.

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
    (see :ref:`smsd-multi`). This variable is available to all ``RunOn``
    directives, including calls which do not provide message details.

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

.. envvar:: SMS_1_SENT_ID

    For a delivery report matched by a database backend, the ID of the
    corresponding row in the ``sentitems`` table. The value is empty for other
    messages, when no matching sent message was found, and for non-database
    backends.

.. envvar:: SMS_1_TIMESTAMP

    Message date and time as seconds since the Unix epoch. For a received
    message this is the service center timestamp. For a delivery report this is
    the time when the service center received the original message. The value
    is empty when the message does not contain a valid date and time.

.. envvar:: SMS_1_DATETIME

    The same value as :envvar:`SMS_1_TIMESTAMP`, formatted as ISO 8601 with a
    timezone offset, for example ``2024-01-02T03:04:05+01:30``.

.. envvar:: SMS_1_SMSC_TIMESTAMP

    For a delivery report, the discharge time as seconds since the Unix epoch.
    The value is empty for other messages or when the report does not contain a
    valid discharge time.

.. envvar:: SMS_1_SMSC_DATETIME

    The same value as :envvar:`SMS_1_SMSC_TIMESTAMP`, formatted as ISO 8601
    with a timezone offset.

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

.. _smsd-run-windows:

Running a batch file on Windows
+++++++++++++++++++++++++++++++

Use ``cmd.exe`` to launch a batch file. For example, with the script and inbox
in ``C:\gammu``:

.. code-block:: ini

    [smsd]
    RunOnReceive = C:\Windows\System32\cmd.exe /d /c C:\gammu\forward_sms.bat

Adjust these absolute paths to match your installation and restart SMSD after
changing the configuration. The example uses paths without spaces.

Do not add ``"%FILE%"``: this is not a Gammu placeholder. SMSD appends the
received file names automatically. For example, the batch file receives
``IN20260228_053040_00_+61402111111_00.txt`` as its first argument. In the batch
file, ``%1`` accesses that argument and ``%~1`` removes surrounding quotes. With
the inbox above, the full path is ``C:\gammu\%~1``. Process subsequent arguments
as well, for example using ``shift`` in a loop.

Use absolute paths for the inbox, address book, log files, and helper programs.
The hook inherits SMSD's working directory, which need not be the script or inbox
directory. When moving SMSD to a Windows service, ensure the service account can
access these files and any credentials needed by the forwarding program.

To test invocation separately from email delivery, save the following as
``C:\gammu\receive-test.bat`` and temporarily use that path in ``RunOnReceive``:

.. code-block:: bat

    @echo off
    setlocal DisableDelayedExpansion
    >>C:\gammu\receive-hook.log echo Hook started
    >>C:\gammu\receive-hook.log set SMS_MESSAGES
    >>C:\gammu\receive-hook.log set SMS_1_NUMBER

Restart SMSD and send a fresh SMS. A new entry in ``receive-hook.log`` confirms
that the batch file started. If no entry appears, check the SMSD log for
``Starting run on receive`` and ``CreateProcess failed``, and check that the
account running SMSD can write the diagnostic log. Restore the forwarding script
path after testing. Its own log should record processing and email delivery
errors because Windows SMSD does not collect the script's output or exit status.

Forwarding messages with an address book
++++++++++++++++++++++++++++++++++++++++

A forwarding program can match :envvar:`SMS_1_NUMBER` against the ``Number``
column of a CSV address book and use the corresponding ``Name`` in the email
subject. Store and compare phone numbers in a consistent format, for example
international numbers beginning with ``+``, and use the sender's number when no
name matches.

For multipart text, use the available ``DECODED_n_TEXT`` variables described
above, or process all file arguments. Do not assume :envvar:`SMS_1_TEXT` contains
the entire message. Read message text as data inside the forwarding program
rather than expanding it into batch commands, where SMS characters could be
interpreted as command syntax.

Configure SMTP authentication in the forwarding program. That program is also
responsible for logging delivery failures and retaining pending messages for
retry; starting a hook does not guarantee email delivery.

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

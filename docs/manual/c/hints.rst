Hints for libGammu Novices
==========================

This is very short overview of libGammu usage. You will probably need to study
:doc:`api` to find out what functions you want to use.


Basic library usage
-------------------

You need to include main header file:

.. code-block:: c

    #include <gammu.h>

To compile you need to pass flags from pkg-config::

    pkg-config --cflags gammu

To link you need to pass from pkg-config::

    pkg-config --libs gammu


Gammu stores all its data in a GSM_StateMachine struct. This structure is not
public, so all you can define is a pointer to it:

.. code-block:: c

    GSM_StateMachine *state_machine;

You'll want to check for errors from time to time.  Do it using a
function something like this:

.. code-block:: c

    void check_error(GSM_Error err)
    {
      if (err == ERR_NONE) {
        return;
      }
      fprintf(stderr, "Gammu failure: %s\n", GSM_ErrorString(error));
      exit(1);
    }

As libGammu does interact with strings in your local encoding, it is good idea
to initialize locales subsystem first (otherwise you would get broken non
ASCII characters):

.. code-block:: c

    GSM_InitLocales(NULL);

You first need to allocate a state machine structure:

.. code-block:: c

    state_machine = GSM_AllocStateMachine();

Now think about the configuration file.  To use the default
:file:`~/.gammurc`, do this:

.. code-block:: c

    INI_Section *cfg;

    /* Find it */
    error = GSM_FindGammuRC(&cfg, NULL);
    check_error(error);

    /* Read it */
    error = GSM_ReadConfig(cfg, GSM_GetConfig(state_machine, 0), 0);
    check_error(error);

    /* Free allocated memory */
    INI_Free(cfg);

    /* We care onlu about first configuration */
    GSM_SetConfigNum(s, 1);

OK, now initialise the connection (1 means number of replies you want to wait
for in case of failure):

.. code-block:: c

    error = GSM_InitConnection(s, 1);
    check_error(error);

Now you are ready to communicate with the phone, for example you can read
manufacturer name:

.. code-block:: c

    error = GSM_GetManufacturer(s, buffer);
    check_error(error);

When you're finished, you need to disconnect and free allocated memory:

.. code-block:: c

    error = GSM_TerminateConnection(s);
    check_error(error);

    /* Free up used memory */
    GSM_FreeStateMachine(s);
    check_error(error);

There are also other :doc:`examples`.

Compling the code
-----------------

To compile program using Gammu library, you need to pass include path to the
compiler and library name and search path to the linker. This can be easiest
achieved by using :program:`pkg-config`. See following ``Makefile`` for
example:

.. code-block:: make

    # Sample Makefile which can be used to build examples shipped with Gammu

    CFLAGS=$(shell pkg-config --cflags --libs gammu-smsd) -Wall
    LDFLAGS=$(shell pkg-config --cflags --libs gammu)

    ALL=phone-info sms-send smsd

    .PHONY: all clean

    all: $(ALL)

    clean:
        rm -f $(ALL)

    %:%.c
        $(CC) $< $(CFLAGS) $(LDFLAGS) -o $@


Unicode
-------

Gammu stores all strings internally in UCS-2-BE encoding (terminated by two
zero bytes). This is used mostly for historical reasons and today the obvious
choice would be ``wchar_t``.  To work with these strings, various functions
are provided (``UnicodeLength``, ``DecodeUnicode``, ``EncodeUnicode``,
``CopyUnicodeString``, etc.).

For printing on console you should use:

.. code-block:: c

    printf("%s\n", DecodeUnicodeConsole(unicode_string));

For giving string to some GUI toolkit:

.. code-block:: c

    printf("%s\n", DecodeUnicodeString(unicode_string));

.. note::

   These functions differ only on platforms where console uses historically
   different character set than GUI, what effectively means only Microsoft
   Windows.

Debugging
---------

You can either enabled debug logging globally or per state machine.

To enable global debugging use:

.. code-block:: c

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

For per state machine configuration:

.. code-block:: c

	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(FALSE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

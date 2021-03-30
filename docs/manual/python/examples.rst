.. _python-gammu-examples:

More python-gammu Examples
==========================

Many examples are available in ``examples/`` directory in the python-gammu
sources.

Sending a message
-----------------

.. code-block:: python

    #!/usr/bin/env python
    # Sample script to show how to send SMS

    from __future__ import print_function
    import gammu
    import sys

    # Create object for talking with phone
    state_machine = gammu.StateMachine()

    # Optionally load config file as defined by first parameter
    if len(sys.argv) > 2:
        # Read the configuration from given file
        state_machine.ReadConfig(Filename=sys.argv[1])
        # Remove file name from args list
        del sys.argv[1]
    else:
        # Read the configuration (~/.gammurc)
        state_machine.ReadConfig()

    # Check parameters
    if len(sys.argv) != 2:
        print("Usage: sendsms.py [configfile] RECIPIENT_NUMBER")
        sys.exit(1)

    # Connect to the phone
    state_machine.Init()

    # Prepare message data
    # We tell that we want to use first SMSC number stored in phone
    message = {
        "Text": "python-gammu testing message",
        "SMSC": {"Location": 1},
        "Number": sys.argv[1],
    }

    # Actually send the message
    state_machine.SendSMS(message)

Sending a long message
----------------------

.. code-block:: python


    #!/usr/bin/env python
    # Sample script to show how to send long (multipart) SMS

    from __future__ import print_function
    import gammu
    import sys

    # Create object for talking with phone
    state_machine = gammu.StateMachine()

    # Optionally load config file as defined by first parameter
    if len(sys.argv) > 2:
        # Read the configuration from given file
        state_machine.ReadConfig(Filename=sys.argv[1])
        # Remove file name from args list
        del sys.argv[1]
    else:
        # Read the configuration (~/.gammurc)
        state_machine.ReadConfig()

    # Check parameters
    if len(sys.argv) != 2:
        print("Usage: sendlongsms.py [configfile] RECIPIENT_NUMBER")
        sys.exit(1)

    # Connect to the phone
    state_machine.Init()


    # Create SMS info structure
    smsinfo = {
        "Class": -1,
        "Unicode": False,
        "Entries": [
            {
                "ID": "ConcatenatedTextLong",
                "Buffer": "Very long python-gammu testing message "
                "sent from example python script. "
                "Very long python-gammu testing message "
                "sent from example python script. "
                "Very long python-gammu testing message "
                "sent from example python script. ",
            }
        ],
    }

    # Encode messages
    encoded = gammu.EncodeSMS(smsinfo)

    # Send messages
    for message in encoded:
        # Fill in numbers
        message["SMSC"] = {"Location": 1}
        message["Number"] = sys.argv[1]

        # Actually send the message
        state_machine.SendSMS(message)

Initiating a voice call
-----------------------

.. code-block:: python

    #!/usr/bin/env python

    from __future__ import print_function
    import gammu
    import sys

    # Create object for talking with phone
    state_machine = gammu.StateMachine()

    # Read the configuration (~/.gammurc or from command line)
    if len(sys.argv) > 2:
        state_machine.ReadConfig(Filename=sys.argv[1])
        del sys.argv[1]
    else:
        state_machine.ReadConfig()

    # Connect to the phone
    state_machine.Init()

    # Check whether we have a number to dial
    if len(sys.argv) != 2:
        print("Usage: dialvoice.py NUMBER")
        sys.exit(1)

    # Dial a number
    state_machine.DialVoice(sys.argv[1])

Reading calendar from phone
---------------------------

.. code-block:: python

    #!/usr/bin/env python
    # Example for reading calendar from phone

    from __future__ import print_function
    import gammu

    # Create object for talking with phone
    state_machine = gammu.StateMachine()

    # Read the configuration (~/.gammurc)
    state_machine.ReadConfig()

    # Connect to the phone
    state_machine.Init()

    # Get number of calendar entries
    status = state_machine.GetCalendarStatus()

    remain = status["Used"]

    start = True

    while remain > 0:
        # Read the entry
        if start:
            entry = state_machine.GetNextCalendar(Start=True)
            start = False
        else:
            entry = state_machine.GetNextCalendar(Location=entry["Location"])
        remain = remain - 1

        # Display it
        print()
        print("%-20s: %d" % ("Location", entry["Location"]))
        print("%-20s: %s" % ("Type", entry["Type"]))
        for v in entry["Entries"]:
            print("%-20s: %s" % (v["Type"], str(v["Value"])))

#!/usr/bin/env python
# Sample script to show how to send SMS

import gammu
import sys

# Create object for talking with phone
sm = gammu.StateMachine()

# Optionally load config file as defined by first parameter
if len(sys.argv) >= 2:
    # Read the configuration from given file
    sm.ReadConfig(Filename = sys.argv[1])
    # Remove file name from args list
    del sys.argv[1]
else:
    # Read the configuration (~/.gammurc)
    sm.ReadConfig()

# Check parameters
if len(sys.argv) != 2:
    print 'Usage: sendsms.py [configfile] RECIPIENT_NUMBER'
    sys.exit(1)

# Connect to the phone
sm.Init()

# Prepare message data
# We tell that we want to use first SMSC number stored in phone
message = {
    'Text': 'python-gammu testing message',
    'SMSC': {'Location': 1},
    'Number': sys.argv[1],
}

# Actually send the message
sm.SendSMS(message)

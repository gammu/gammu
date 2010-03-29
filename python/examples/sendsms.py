#!/usr/bin/env python
# Sample script to show how to send SMS

import gammu
import sys

# Check parameters
if len(sys.argv) != 2:
    print 'This requires one parameter containing number!'
    sys.exit(1)

# Create object for talking with phone
sm = gammu.StateMachine()

# Read the configuration (~/.gammurc)
sm.ReadConfig()

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

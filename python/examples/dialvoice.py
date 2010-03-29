#!/usr/bin/env python

import gammu
import sys

# Create object for talking with phone
sm = gammu.StateMachine()

# Read the configuration (~/.gammurc or from command line)
if len(sys.argv) >= 2:
    sm.ReadConfig(Filename = sys.argv[1])
    del sys.argv[1]
else:
    sm.ReadConfig()

# Connect to the phone
sm.Init()

# Check whether we have a number to dial
if len(sys.argv) != 2:
    print 'Usage: dialvoice.py NUMBER'
    sys.exit(1)

# Dial a number
sm.DialVoice(sys.argv[1])

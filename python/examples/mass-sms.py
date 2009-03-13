#!/usr/bin/env python
# sample script to show how to same SMS to multiple recipients

import gammu
import sys

# Check parameters count
if len(sys.argv) < 3 or sys.argv[1] in ['--help', '-h', '-?']:
    print 'Usage: mass-sms <TEXT> [number]...'
    sys.exit(1)

# Configure Gammu
sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

# Prepare SMS template
message = {'Text': sys.argv[1], 'SMSC': {'Location': 1}}

# Send SMS to all recipients on command line
for number in sys.argv[2:]:
    message['Number'] = number
    try:
        sm.SendSMS(message)
    except Exception, exc:
        print 'Sending to %s failed: %s' % (number, exc)

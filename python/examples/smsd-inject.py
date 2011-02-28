#!/usr/bin/env python
# sample script to show how to send SMS through SMSD

import gammu.smsd

smsd = gammu.smsd.SMSD('/etc/gammu-smsdrc')

import sys
if len(sys.argv) != 2:
    print 'Usage: smsd-inject.py RECIPIENT_NUMBER'
    sys.exit(1)

message = {'Text': 'python-gammu testing message', 'SMSC': {'Location': 1}, 'Number': sys.argv[1]}

smsd.InjectSMS([message])

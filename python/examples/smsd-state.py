#!/usr/bin/env python
# sample script to show how to get SMSD status

import gammu.smsd

smsd = gammu.smsd.SMSD('/etc/gammu-smsdrc')

print smsd.GetStatus()

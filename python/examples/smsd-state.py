#!/usr/bin/env python
# sample script to show how to get SMSD status

import gammu

smsd = gammu.SMSD('/etc/gammu-smsdrc')

print smsd.GetStatus()

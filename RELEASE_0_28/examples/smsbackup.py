#!/usr/bin/env python

import gammu

import sys
if len(sys.argv) != 3:
    print 'This requires two parameter with file names! First is input, second output.'
    sys.exit(1)

backup = gammu.ReadSMSBackup(sys.argv[1])
gammu.SaveSMSBackup(sys.argv[2], backup)

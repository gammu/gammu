#!/usr/bin/env python

import gammu

import sys
if len(sys.argv) != 2:
    print 'This requires parameter with hex encoded PDU data!'
    sys.exit(1)

# Global debug level
gammu.SetDebugFile(sys.stderr)
gammu.SetDebugLevel('textall')

sms = gammu.DecodePDU(sys.argv[1].decode('hex'))
print sms

#!/usr/bin/env python

import gammu
import sys

# Global debug level
gammu.SetDebugFile(sys.stderr)
gammu.SetDebugLevel('textall')

sm = gammu.StateMachine()
sm.ReadConfig()

# Use global debug stub regardless configuration
c = sm.GetConfig(0)
c['UseGlobalDebugFile'] = True
sm.SetConfig(0, c)

sm.Init()

Manufacturer = sm.GetManufacturer()
Model = sm.GetModel()
IMEI = sm.GetIMEI()
Firmware = sm.GetFirmware()
print _('Phone infomation:')
print '%-15s: %s' % (_('Manufacturer'), Manufacturer)
print '%-15s: %s (%s)' % (_('Model'), Model[0], Model[1])
print '%-15s: %s' % (_('IMEI'), IMEI)
print '%-15s: %s' % (_('Firmware'), Firmware[0])

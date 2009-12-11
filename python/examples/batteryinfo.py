#!/usr/bin/env python

import gammu
import sys

sm = gammu.StateMachine()
if len(sys.argv) >= 2:
    sm.ReadConfig(Filename = sys.argv[1])
    del sys.argv[1]
else:
    sm.ReadConfig()
sm.Init()

status = sm.GetBatteryCharge()

for x in status:
    if status[x] != -1:
        print "%20s: %s" % (x, status[x])

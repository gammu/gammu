#!/usr/bin/env python

import gammu

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

import sys

status = sm.GetBatteryCharge()

for x in status:
    if status[x] != -1:
        print "%20s: %s" % (x, status[x])

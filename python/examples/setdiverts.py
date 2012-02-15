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

if len(sys.argv) != 2:
    print 'Usage: setdiverts.py NUMBER'
    sys.exit(1)

sm.SetCallDivert('AllTypes', 'All', sys.argv[1])
diverts = sm.GetCallDivert()

for x in diverts:
    print x


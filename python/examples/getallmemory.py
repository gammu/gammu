#!/usr/bin/env python

import gammu

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

import sys
if len(sys.argv) != 2:
    print 'This requires one parameter with memory type!'
    sys.exit(1)

type = sys.argv[1]

status = sm.GetMemoryStatus(Type = type)

remain = status['Used']

start = True

while remain > 0:
    if start:
        entry = sm.GetNextMemory(Start = True, Type = type)
        start = False
    else:
        entry = sm.GetNextMemory(Location = entry['Location'], Type = type)
    remain = remain - 1

    print
    print '%-15s: %d' % ('Location',entry['Location'])
    for v in entry['Entries']:
        print '%-15s: %s' % (v['Type'], str(v['Value']))

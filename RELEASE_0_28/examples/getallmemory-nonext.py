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

location = 1

while remain > 0:
    try:
        entry = sm.GetMemory(Type = type, Location = location)
        print
        print '%-15s: %d' % ('Location',entry['Location'])
        for v in entry['Entries']:
            print '%-15s: %s' % (v['Type'], str(v['Value']))
        remain = remain - 1
    except gammu.ERR_EMPTY:
        pass
    location = location + 1
